#include "lvgl_display.h"

#include "driver/spi_master.h"
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
#define LVGL_DISPLAY_WAKE_RESET 1  // wake reset
#define LVGL_DISPLAY_WAKE_REINIT 1 // wake re-init

#define DISP_HOR_RES TFT_WIDTH
#define DISP_VER_RES TFT_HEIGHT
#define DISP_BUF_LINES 40
#define ST7306_ALIGN_X 6
#define ST7306_ALIGN_Y 2

static esp_lcd_panel_io_handle_t s_io_handle = NULL;
static esp_lcd_panel_handle_t s_panel_handle = NULL;

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

lv_display_t *lvgl_display_init(void)
{
    static bool s_inited = false;
    static lv_display_t *s_disp = NULL;

    if (s_inited) {
        return s_disp;
    }

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

    lvgl_port_lock(0);
    lv_obj_invalidate(lv_scr_act());
    lv_refr_now(NULL);
    lvgl_port_unlock();
    ESP_LOGI(TAG, "lvgl refresh done");
}

void lvgl_display_sleep(bool sleep)
{
    if (!s_panel_handle) {
        ESP_LOGW(TAG, "panel handle null, skip sleep");
        return;
    }
    esp_err_t err = esp_lcd_panel_disp_sleep(s_panel_handle, sleep);
    if (err == ESP_ERR_NOT_SUPPORTED) {
        ESP_LOGW(TAG, "panel sleep not supported");
        return;
    }
    ESP_ERROR_CHECK(err);
}



