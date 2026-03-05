#include "lvgl_display.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "MONO_TFT.h"
#include "esp_lcd_panel_st7306_bw.h"

#define TAG "lvgl_display"
#define LVGL_DISPLAY_WAKE_RESET 0  // wake reset
#define LVGL_DISPLAY_WAKE_REINIT 0 // wake re-init

#define DISP_HOR_RES TFT_WIDTH
#define DISP_VER_RES TFT_HEIGHT
#define DISP_BUF_LINES 20
#define ST7306_ALIGN_X 6
#define ST7306_ALIGN_Y 2

static esp_lcd_panel_io_handle_t s_io_handle = NULL;
static esp_lcd_panel_handle_t s_panel_handle = NULL;
static bool s_display_sleeping = false;
static bool s_display_low_power = false;

static void st7306_release_gpio_hold(void)
{
    const int pins[] = {RST_PIN, CS_PIN, DC_PIN, SCLK_PIN, SDI_PIN};
    for (size_t i = 0; i < sizeof(pins) / sizeof(pins[0]); i++) {
        if (pins[i] < 0) {
            continue;
        }
        gpio_hold_dis(pins[i]);
    }
}
// Send a single ST7306 command with optional parameters over panel IO.
static esp_err_t st7306_send_cmd(uint8_t cmd, const uint8_t *params, size_t len)
{
    if (!s_io_handle) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_io_tx_param(s_io_handle, cmd, params, len);
}

// Program ST7306 power-related registers (VSH/VSL levels + source select).
static esp_err_t st7306_set_power_regs(void)
{
    // VSHP/VSLP/VSHN/VSLN + source voltage select, per vendor sequence
    esp_err_t err = st7306_send_cmd(0xC1, (uint8_t[]) { 115, 0x3E, 0x3C, 0x3C }, 4);
    if (err != ESP_OK) return err;
    err = st7306_send_cmd(0xC2, (uint8_t[]) { 0x00, 0x21, 0x23, 0x23 }, 4);
    if (err != ESP_OK) return err;
    err = st7306_send_cmd(0xC4, (uint8_t[]) { 50, 0x5C, 0x5A, 0x5A }, 4);
    if (err != ESP_OK) return err;
    err = st7306_send_cmd(0xC5, (uint8_t[]) { 50, 0x35, 0x37, 0x37 }, 4);
    if (err != ESP_OK) return err;
    err = st7306_send_cmd(0xC9, (uint8_t[]) { 0x00 }, 1);
    if (err != ESP_OK) return err;
    return ESP_OK;
}

// Restore key display config after sleep-out (MADCTL/format/panel/addr).
static esp_err_t st7306_restore_wake_regs(void)
{
    // Restore key settings after LPM->HPM, per vendor init sequence
    esp_err_t err = st7306_send_cmd(0x36, (uint8_t[]) { 0x48 }, 1); // MADCTL
    if (err != ESP_OK) return err;
    err = st7306_send_cmd(0x3A, (uint8_t[]) { 0x11 }, 1); // data format
    if (err != ESP_OK) return err;
    err = st7306_send_cmd(0xB9, (uint8_t[]) { 0x20 }, 1); // gamma mode
    if (err != ESP_OK) return err;
    err = st7306_send_cmd(0xB8, (uint8_t[]) { 0x29 }, 1); // panel setting
    if (err != ESP_OK) return err;
    err = st7306_send_cmd(0x2A, (uint8_t[]) { 0x05, 0x36 }, 2); // column
    if (err != ESP_OK) return err;
    err = st7306_send_cmd(0x2B, (uint8_t[]) { 0x00, 0xC7 }, 2); // row
    if (err != ESP_OK) return err;
    err = st7306_send_cmd(0x35, (uint8_t[]) { 0x00 }, 1); // TE
    if (err != ESP_OK) return err;
    err = st7306_send_cmd(0xD0, (uint8_t[]) { 0xFF }, 1); // auto power
    if (err != ESP_OK) return err;
    return ESP_OK;
}

// Enter/exit ST7306 low-power mode (keeps RAM content).
static esp_err_t st7306_set_low_power_mode(bool enable)
{
    return st7306_send_cmd(enable ? 0x39 : 0x38, NULL, 0);
}

// Align invalidated areas to controller row/column granularity.
static void st7306_align_invalidate_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_INVALIDATE_AREA) {
        return;
    }
    lv_area_t *a = lv_event_get_invalidated_area(e);
    if (!a) {
        return;
    }
    lv_display_t *disp = lv_event_get_target(e);
    if (!disp) {
        return;
    }

    int32_t hres = lv_display_get_horizontal_resolution(disp);
    int32_t vres = lv_display_get_vertical_resolution(disp);

    int32_t x1 = a->x1 < 0 ? 0 : a->x1;
    int32_t y1 = a->y1 < 0 ? 0 : a->y1;
    int32_t x2 = a->x2 >= hres ? (hres - 1) : a->x2;
    int32_t y2 = a->y2 >= vres ? (vres - 1) : a->y2;

    if (x1 > x2 || y1 > y2) {
        return;
    }

    int32_t x1_al = 0;
    int32_t x2_al = hres - 1;
    int32_t y1_al = (y1 / ST7306_ALIGN_Y) * ST7306_ALIGN_Y;
    int32_t y2_al = ((y2 / ST7306_ALIGN_Y) + 1) * ST7306_ALIGN_Y - 1;

    if (x1_al < 0) {
        x1_al = 0;
    }
    if (y1_al < 0) {
        y1_al = 0;
    }
    if (x2_al >= hres) {
        x2_al = hres - 1;
    }
    if (y2_al >= vres) {
        y2_al = vres - 1;
    }

    a->x1 = x1_al;
    a->y1 = y1_al;
    a->x2 = x2_al;
    a->y2 = y2_al;
}

// Initialize SPI bus, panel IO, panel driver, and LVGL display backend.
lv_display_t *lvgl_display_init(void)
{
    static bool s_inited = false;
    static lv_display_t *s_disp = NULL;

    if (s_inited) {
        return s_disp;
    }

    // Release any deep-sleep GPIO holds before reinitializing the panel.
    st7306_release_gpio_hold();

    spi_bus_config_t buscfg = {
        .sclk_io_num = SCLK_PIN,
        .mosi_io_num = SDI_PIN,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = (DISP_HOR_RES * DISP_VER_RES) / 4,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = CS_PIN,
        .dc_gpio_num = DC_PIN,
        .spi_mode = 0,
        .pclk_hz = 40 * 1000 * 1000,
        .trans_queue_depth = 1,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI2_HOST, &io_config, &s_io_handle));

    esp_lcd_panel_st7306_bw_config_t panel_cfg = {
        .width = DISP_HOR_RES,
        .height = DISP_VER_RES,
    };
    esp_lcd_panel_dev_config_t panel_dev_cfg = {
        .reset_gpio_num = RST_PIN,
        .bits_per_pixel = 1,
        .vendor_config = &panel_cfg,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7306_bw(s_io_handle, &panel_dev_cfg, &s_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(s_panel_handle, false));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(s_panel_handle));
    vTaskDelay(pdMS_TO_TICKS(50));
    ESP_ERROR_CHECK(esp_lcd_panel_init(s_panel_handle));
    vTaskDelay(pdMS_TO_TICKS(50));
    (void)st7306_set_low_power_mode(false);
    s_display_low_power = false;
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(s_panel_handle, false, false));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(s_panel_handle, false));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(s_panel_handle, true));

    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = s_io_handle,
        .panel_handle = s_panel_handle,
        .buffer_size = DISP_HOR_RES * DISP_BUF_LINES,
        .double_buffer = false,
        .hres = DISP_HOR_RES,
        .vres = DISP_VER_RES,
        .monochrome = false,
        .color_format = LV_COLOR_FORMAT_RGB565,
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .full_refresh = false,
        }
    };
    s_disp = lvgl_port_add_disp(&disp_cfg);
    if (!s_disp) {
        ESP_LOGE(TAG, "lvgl_port_add_disp failed");
        return NULL;
    }
    lv_display_add_event_cb(s_disp, st7306_align_invalidate_cb, LV_EVENT_INVALIDATE_AREA, NULL);

    s_inited = true;
    return s_disp;
}

// Wake the panel after ESP light sleep (optional reset/init + re-apply flips).
void lvgl_display_wakeup(void)
{
    ESP_LOGI(TAG, "lvgl_display_wakeup");
    if (!s_panel_handle) {
        ESP_LOGW(TAG, "panel handle null, re-init display");
        lvgl_display_init();
        return;
    }

    // Wake path: reset/init then refresh (no display off)
#if LVGL_DISPLAY_WAKE_RESET
    ESP_ERROR_CHECK(esp_lcd_panel_reset(s_panel_handle));
    vTaskDelay(pdMS_TO_TICKS(20));
#endif
#if LVGL_DISPLAY_WAKE_REINIT
    ESP_ERROR_CHECK(esp_lcd_panel_init(s_panel_handle));
    vTaskDelay(pdMS_TO_TICKS(20));
#endif
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(s_panel_handle, false, false));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(s_panel_handle, false));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(s_panel_handle, true));
    (void)st7306_set_low_power_mode(false);
    s_display_low_power = false;

    // Do not force a full-screen refresh here; UI updates will invalidate as needed.
    ESP_LOGI(TAG, "lvgl refresh done");
}

// Enter/exit panel sleep without full reset/init.
void lvgl_display_sleep(bool sleep)
{
    if (!s_panel_handle || !s_io_handle) {
        ESP_LOGW(TAG, "panel handle null, skip sleep");
        return;
    }
    if (sleep) {
        if (s_display_sleeping) {
            return;
        }
        // Per ST7306 SLPIN/SLPOUT sequence: enter sleep (no reset/init)
        esp_err_t err = st7306_send_cmd(0x10, NULL, 0); // SLPIN
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "sleep-in failed: 0x%x", (unsigned)err);
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
        s_display_sleeping = true;
    } else {
        if (!s_display_sleeping) {
            return;
        }
        // Exit sleep, then restore critical regs and display on
        esp_err_t err = st7306_send_cmd(0x11, NULL, 0); // SLPOUT
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "sleep-out failed: 0x%x", (unsigned)err);
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(120));
        err = st7306_restore_wake_regs();
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "restore regs failed: 0x%x", (unsigned)err);
        }
        (void)st7306_send_cmd(0x29, NULL, 0); // Display on
        (void)st7306_send_cmd(0x20, NULL, 0); // Inversion off
        s_display_sleeping = false;
    }
}

// Enter/exit panel low-power mode without clearing RAM.
void lvgl_display_low_power(bool enable)
{
    if (!s_panel_handle || !s_io_handle) {
        ESP_LOGW(TAG, "panel handle null, skip low power");
        return;
    }
    if (enable) {
        if (s_display_low_power) {
            return;
        }
        esp_err_t err = st7306_set_low_power_mode(true);
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "low power on failed: 0x%x", (unsigned)err);
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
        s_display_low_power = true;
    } else {
        if (!s_display_low_power) {
            return;
        }
        esp_err_t err = st7306_set_low_power_mode(false);
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "low power off failed: 0x%x", (unsigned)err);
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
        s_display_low_power = false;
    }
}



