#include "esp_lcd_panel_st7306_bw.h"

#include <stdlib.h>
#include <string.h>
#include <sys/cdefs.h>

#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define ST7306_COLOR_00 (0x00)
#define ST7306_COLOR_01 (0x01)
#define ST7306_COLOR_10 (0x02)
#define ST7306_COLOR_11 (0x03)

#define ST73XX_COLOR_BLACK ST7306_COLOR_11
#define ST73XX_COLOR_WHITE ST7306_COLOR_00

#define ST7306_COL_START 0x05
#define ST7306_ROW_START 0x00

#define ST7306_CMD_CASET 0x2A
#define ST7306_CMD_RASET 0x2B
#define ST7306_CMD_RAMWR 0x2C

#define ST7306_CMD_DISP_ON  0x29
#define ST7306_CMD_DISP_OFF 0x28
#define ST7306_CMD_INV_ON   0x21
#define ST7306_CMD_INV_OFF  0x20

typedef struct {
    esp_lcd_panel_t base;
    esp_lcd_panel_io_handle_t io;
    uint16_t width;
    uint16_t height;
    uint16_t data_width;
    uint16_t data_height;
    int reset_gpio_num;
    bool reset_level;
    int x_gap;
    int y_gap;
    bool swap_axes;
    bool mirror_x;
    bool mirror_y;
    uint8_t *buffer;
    size_t buffer_len;
} st7306_panel_t;

static const char *TAG = "lcd_panel.st7306_bw";

static esp_err_t panel_st7306_del(esp_lcd_panel_t *panel);
static esp_err_t panel_st7306_reset(esp_lcd_panel_t *panel);
static esp_err_t panel_st7306_init(esp_lcd_panel_t *panel);
static esp_err_t panel_st7306_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start,
                                          int x_end, int y_end, const void *color_data);
static esp_err_t panel_st7306_invert_color(esp_lcd_panel_t *panel, bool invert_color_data);
static esp_err_t panel_st7306_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y);
static esp_err_t panel_st7306_swap_xy(esp_lcd_panel_t *panel, bool swap_axes);
static esp_err_t panel_st7306_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap);
static esp_err_t panel_st7306_disp_on_off(esp_lcd_panel_t *panel, bool on_off);

static uint8_t st7306_rgb565_to_gray2(uint16_t color)
{
    uint8_t r5 = (uint8_t)((color >> 11) & 0x1F);
    uint8_t g6 = (uint8_t)((color >> 5) & 0x3F);
    uint8_t b5 = (uint8_t)(color & 0x1F);

    uint16_t r8 = (uint16_t)((r5 * 255 + 15) / 31);
    uint16_t g8 = (uint16_t)((g6 * 255 + 31) / 63);
    uint16_t b8 = (uint16_t)((b5 * 255 + 15) / 31);

    uint16_t lum = (uint16_t)((r8 * 299 + g8 * 587 + b8 * 114 + 500) / 1000);

    if (lum > 191) {
        return ST7306_COLOR_00;
    }
    if (lum > 127) {
        return ST7306_COLOR_01;
    }
    if (lum > 63) {
        return ST7306_COLOR_10;
    }
    return ST7306_COLOR_11;
}

static inline void st7306_write_point(st7306_panel_t *st7306, uint32_t x, uint32_t y, uint16_t data)
{
    uint32_t real_x = x / 2;
    uint32_t real_y = y / 2;
    uint32_t write_byte_index = real_y * st7306->data_width + real_x;
    uint32_t one_two = (y % 2 == 0) ? 0 : 1;
    uint32_t line_bit_1 = (x % 2) * 4;
    uint32_t line_bit_0 = (x % 2) * 4 + 2;
    uint8_t write_bit_1 = (uint8_t)(7 - (line_bit_1 + one_two));
    uint8_t write_bit_0 = (uint8_t)(7 - (line_bit_0 + one_two));

    bool data_bit0 = (data & 0x0001) != 0;
    bool data_bit1 = (data & 0x0002) != 0;

    if (data_bit1) {
        st7306->buffer[write_byte_index] |= (uint8_t)(1U << write_bit_1);
    } else {
        st7306->buffer[write_byte_index] &= (uint8_t)~(1U << write_bit_1);
    }

    if (data_bit0) {
        st7306->buffer[write_byte_index] |= (uint8_t)(1U << write_bit_0);
    } else {
        st7306->buffer[write_byte_index] &= (uint8_t)~(1U << write_bit_0);
    }
}

static inline void st7306_transform_point(const st7306_panel_t *st7306, int *x, int *y)
{
    int tx = *x;
    int ty = *y;
    int w = st7306->swap_axes ? (int)st7306->height : (int)st7306->width;
    int h = st7306->swap_axes ? (int)st7306->width : (int)st7306->height;

    if (st7306->swap_axes) {
        int t = tx;
        tx = ty;
        ty = t;
    }
    if (st7306->mirror_x) {
        tx = w - 1 - tx;
    }
    if (st7306->mirror_y) {
        ty = h - 1 - ty;
    }

    *x = tx;
    *y = ty;
}

static inline void st7306_transform_area(const st7306_panel_t *st7306, int *x1, int *y1, int *x2, int *y2)
{
    int tx1 = *x1;
    int ty1 = *y1;
    int tx2 = *x2;
    int ty2 = *y2;
    int w = st7306->swap_axes ? (int)st7306->height : (int)st7306->width;
    int h = st7306->swap_axes ? (int)st7306->width : (int)st7306->height;

    if (st7306->swap_axes) {
        int t1 = tx1;
        int t2 = tx2;
        tx1 = ty1;
        tx2 = ty2;
        ty1 = t1;
        ty2 = t2;
    }
    if (st7306->mirror_x) {
        int nx1 = w - 1 - tx2;
        int nx2 = w - 1 - tx1;
        tx1 = nx1;
        tx2 = nx2;
    }
    if (st7306->mirror_y) {
        int ny1 = h - 1 - ty2;
        int ny2 = h - 1 - ty1;
        ty1 = ny1;
        ty2 = ny2;
    }

    *x1 = tx1;
    *y1 = ty1;
    *x2 = tx2;
    *y2 = ty2;
}

static esp_err_t st7306_send_cmd_params(st7306_panel_t *st7306, uint8_t cmd, const uint8_t *params, size_t len)
{
    return esp_lcd_panel_io_tx_param(st7306->io, cmd, params, len);
}

static esp_err_t st7306_set_full_address(st7306_panel_t *st7306)
{
    uint8_t col_end = (uint8_t)(ST7306_COL_START + (st7306->data_width / 3) - 1);
    uint8_t row_end = (uint8_t)(st7306->data_height - 1);

    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, ST7306_CMD_CASET, (uint8_t[]) {
        ST7306_COL_START,
        col_end
    }, 2), TAG, "set column range failed");

    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, ST7306_CMD_RASET, (uint8_t[]) {
        ST7306_ROW_START,
        row_end
    }, 2), TAG, "set row range failed");

    return ESP_OK;
}

static esp_err_t st7306_set_address_window(st7306_panel_t *st7306, uint8_t col_start, uint8_t col_end,
                                           uint8_t row_start, uint8_t row_end)
{
    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, ST7306_CMD_CASET, (uint8_t[]) {
        col_start,
        col_end
    }, 2), TAG, "set column range failed");

    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, ST7306_CMD_RASET, (uint8_t[]) {
        row_start,
        row_end
    }, 2), TAG, "set row range failed");

    return ESP_OK;
}

static esp_err_t st7306_flush_region(st7306_panel_t *st7306, int x1, int y1, int x2, int y2)
{
    x1 = (x1 / 6) * 6;
    y1 = (y1 / 2) * 2;
    x2 = ((x2 / 6) + 1) * 6 - 1;
    y2 = ((y2 / 2) + 1) * 2 - 1;

    if (x1 < 0) {
        x1 = 0;
    }
    if (y1 < 0) {
        y1 = 0;
    }
    if (x2 >= st7306->width) {
        x2 = st7306->width - 1;
    }
    if (y2 >= st7306->height) {
        y2 = st7306->height - 1;
    }
    if (x1 > x2 || y1 > y2) {
        return ESP_OK;
    }

    uint32_t real_x_start = (uint32_t)x1 / 2;
    uint32_t real_x_end = (uint32_t)x2 / 2;
    uint32_t real_y_start = (uint32_t)y1 / 2;
    uint32_t real_y_end = (uint32_t)y2 / 2;

    uint32_t byte_start = real_x_start;
    uint32_t byte_end = real_x_end;

    uint32_t col_byte_start = (byte_start / 3) * 3;
    uint32_t col_byte_end = (byte_end / 3) * 3 + 2;
    if (col_byte_end >= st7306->data_width) {
        col_byte_end = st7306->data_width - 1;
    }

    uint8_t col_start = (uint8_t)(ST7306_COL_START + (col_byte_start / 3));
    uint8_t col_end = (uint8_t)(ST7306_COL_START + (col_byte_end / 3));

    size_t len = col_byte_end - col_byte_start + 1;
    for (uint32_t row = real_y_start; row <= real_y_end; ++row) {
        uint8_t row_start = (uint8_t)row;
        uint8_t row_end = (uint8_t)row;
        ESP_RETURN_ON_ERROR(st7306_set_address_window(st7306, col_start, col_end, row_start, row_end), TAG, "set window failed");
        const uint8_t *ptr = st7306->buffer + (row * st7306->data_width) + col_byte_start;
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_color(st7306->io, ST7306_CMD_RAMWR, ptr, len), TAG, "tx color failed");
    }

    return ESP_OK;
}

esp_err_t esp_lcd_new_panel_st7306_bw(const esp_lcd_panel_io_handle_t io,
                                      const esp_lcd_panel_dev_config_t *panel_dev_config,
                                      esp_lcd_panel_handle_t *ret_panel)
{
    esp_err_t ret = ESP_OK;
    st7306_panel_t *st7306 = NULL;
    ESP_GOTO_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG, err, TAG, "invalid argument");
    ESP_GOTO_ON_FALSE(panel_dev_config->bits_per_pixel == 1, ESP_ERR_INVALID_ARG, err, TAG, "bpp must be 1");

    const esp_lcd_panel_st7306_bw_config_t *spec =
        (const esp_lcd_panel_st7306_bw_config_t *)panel_dev_config->vendor_config;
    ESP_GOTO_ON_FALSE(spec && spec->width > 0 && spec->height > 0, ESP_ERR_INVALID_ARG, err, TAG, "invalid size");

    st7306 = calloc(1, sizeof(st7306_panel_t));
    ESP_GOTO_ON_FALSE(st7306, ESP_ERR_NO_MEM, err, TAG, "no mem for panel");

    if (panel_dev_config->reset_gpio_num >= 0) {
        gpio_config_t io_conf = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = 1ULL << panel_dev_config->reset_gpio_num,
        };
        ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, TAG, "configure GPIO for RST line failed");
    }

    st7306->io = io;
    st7306->width = spec->width;
    st7306->height = spec->height;
    st7306->data_width = spec->width / 2;
    st7306->data_height = spec->height / 2;
    st7306->reset_gpio_num = panel_dev_config->reset_gpio_num;
    st7306->reset_level = panel_dev_config->flags.reset_active_high;
    st7306->buffer_len = (size_t)(spec->width * spec->height / 4);

    st7306->buffer = heap_caps_malloc(st7306->buffer_len, MALLOC_CAP_DMA);
    ESP_GOTO_ON_FALSE(st7306->buffer, ESP_ERR_NO_MEM, err, TAG, "no mem for frame buffer");
    memset(st7306->buffer, 0x00, st7306->buffer_len);

    st7306->base.del = panel_st7306_del;
    st7306->base.reset = panel_st7306_reset;
    st7306->base.init = panel_st7306_init;
    st7306->base.draw_bitmap = panel_st7306_draw_bitmap;
    st7306->base.invert_color = panel_st7306_invert_color;
    st7306->base.set_gap = panel_st7306_set_gap;
    st7306->base.mirror = panel_st7306_mirror;
    st7306->base.swap_xy = panel_st7306_swap_xy;
    st7306->base.disp_on_off = panel_st7306_disp_on_off;

    *ret_panel = &(st7306->base);
    return ESP_OK;

err:
    if (st7306) {
        if (st7306->buffer) {
            heap_caps_free(st7306->buffer);
        }
        if (panel_dev_config->reset_gpio_num >= 0) {
            gpio_reset_pin(panel_dev_config->reset_gpio_num);
        }
        free(st7306);
    }
    return ret;
}

static esp_err_t panel_st7306_del(esp_lcd_panel_t *panel)
{
    st7306_panel_t *st7306 = __containerof(panel, st7306_panel_t, base);
    if (st7306->reset_gpio_num >= 0) {
        gpio_reset_pin(st7306->reset_gpio_num);
    }
    if (st7306->buffer) {
        heap_caps_free(st7306->buffer);
    }
    free(st7306);
    return ESP_OK;
}

static esp_err_t panel_st7306_reset(esp_lcd_panel_t *panel)
{
    st7306_panel_t *st7306 = __containerof(panel, st7306_panel_t, base);
    if (st7306->reset_gpio_num >= 0) {
        gpio_set_level(st7306->reset_gpio_num, st7306->reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(st7306->reset_gpio_num, !st7306->reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    return ESP_OK;
}

static esp_err_t panel_st7306_init(esp_lcd_panel_t *panel)
{
    st7306_panel_t *st7306 = __containerof(panel, st7306_panel_t, base);

    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, 0xD6, (uint8_t[]) { 0x17, 0x02 }, 2), TAG, "cmd 0xD6");
    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, 0xD1, (uint8_t[]) { 0x01 }, 1), TAG, "cmd 0xD1");
    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, 0xC0, (uint8_t[]) { 0x12, 0x0A }, 2), TAG, "cmd 0xC0");
    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, 0xC1, (uint8_t[]) { 115, 0x3E, 0x3C, 0x3C }, 4), TAG, "cmd 0xC1");
    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, 0xC2, (uint8_t[]) { 0x00, 0x21, 0x23, 0x23 }, 4), TAG, "cmd 0xC2");
    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, 0xC4, (uint8_t[]) { 50, 0x5C, 0x5A, 0x5A }, 4), TAG, "cmd 0xC4");
    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, 0xC5, (uint8_t[]) { 50, 0x35, 0x37, 0x37 }, 4), TAG, "cmd 0xC5");
    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, 0xD8, (uint8_t[]) { 0xA6, 0xE9 }, 2), TAG, "cmd 0xD8");
    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, 0xB2, (uint8_t[]) { 0x12 }, 1), TAG, "cmd 0xB2");
    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, 0xB3, (uint8_t[]) { 0xE5, 0xF6, 0x17, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x71 }, 10), TAG, "cmd 0xB3");
    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, 0xB4, (uint8_t[]) { 0x05, 0x46, 0x77, 0x77, 0x77, 0x77, 0x76, 0x45 }, 8), TAG, "cmd 0xB4");
    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, 0x62, (uint8_t[]) { 0x32, 0x03, 0x1F }, 3), TAG, "cmd 0x62");
    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, 0xB7, (uint8_t[]) { 0x13 }, 1), TAG, "cmd 0xB7");
    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, 0xB0, (uint8_t[]) { 0x64 }, 1), TAG, "cmd 0xB0");

    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, 0x11, NULL, 0), TAG, "cmd 0x11");
    vTaskDelay(pdMS_TO_TICKS(120));

    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, 0xC9, (uint8_t[]) { 0x00 }, 1), TAG, "cmd 0xC9");
    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, 0x36, (uint8_t[]) { 0x48 }, 1), TAG, "cmd 0x36");
    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, 0x3A, (uint8_t[]) { 0x11 }, 1), TAG, "cmd 0x3A");
    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, 0xB9, (uint8_t[]) { 0x20 }, 1), TAG, "cmd 0xB9");
    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, 0xB8, (uint8_t[]) { 0x29 }, 1), TAG, "cmd 0xB8");

    ESP_RETURN_ON_ERROR(st7306_set_full_address(st7306), TAG, "set address");
    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, 0x35, (uint8_t[]) { 0x00 }, 1), TAG, "cmd 0x35");
    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, 0xD0, (uint8_t[]) { 0xFF }, 1), TAG, "cmd 0xD0");
    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, 0x38, NULL, 0), TAG, "cmd 0x38");

    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, ST7306_CMD_DISP_ON, NULL, 0), TAG, "disp on");
    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, ST7306_CMD_INV_OFF, NULL, 0), TAG, "inv off");
    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, 0xBB, (uint8_t[]) { 0x4F }, 1), TAG, "cmd 0xBB");

    return ESP_OK;
}

static esp_err_t panel_st7306_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start,
                                          int x_end, int y_end, const void *color_data)
{
    st7306_panel_t *st7306 = __containerof(panel, st7306_panel_t, base);
    if (!color_data) {
        return ESP_ERR_INVALID_ARG;
    }

    const int orig_x_start = x_start;
    const int orig_y_start = y_start;
    const int orig_x_end = x_end;
    const int orig_y_end = y_end;

    x_start += st7306->x_gap;
    x_end += st7306->x_gap;
    y_start += st7306->y_gap;
    y_end += st7306->y_gap;

    if (x_start < 0) {
        x_start = 0;
    }
    if (y_start < 0) {
        y_start = 0;
    }
    if (x_end > st7306->width) {
        x_end = st7306->width;
    }
    if (y_end > st7306->height) {
        y_end = st7306->height;
    }
    if (x_start >= x_end || y_start >= y_end) {
        return ESP_OK;
    }

    int area_w = orig_x_end - orig_x_start;
    if (area_w <= 0) {
        return ESP_OK;
    }
    const uint16_t *src = (const uint16_t *)color_data;

    for (int y = y_start; y < y_end; ++y) {
        int src_y = y - orig_y_start;
        const uint16_t *row = src + (size_t)src_y * (size_t)area_w;
        for (int x = x_start; x < x_end; ++x) {
            int src_x = x - orig_x_start;
            uint16_t px = row[(uint32_t)src_x];
            uint8_t gray = st7306_rgb565_to_gray2(px);
            int draw_x = x;
            int draw_y = y;
            if (st7306->swap_axes || st7306->mirror_x || st7306->mirror_y) {
                st7306_transform_point(st7306, &draw_x, &draw_y);
            }
            st7306_write_point(st7306, (uint32_t)draw_x, (uint32_t)draw_y, gray);
        }
    }

    if (st7306->swap_axes || st7306->mirror_x || st7306->mirror_y) {
        int tx1 = x_start;
        int ty1 = y_start;
        int tx2 = x_end - 1;
        int ty2 = y_end - 1;
        st7306_transform_area(st7306, &tx1, &ty1, &tx2, &ty2);
        return st7306_flush_region(st7306, tx1, ty1, tx2, ty2);
    }

    return st7306_flush_region(st7306, x_start, y_start, x_end - 1, y_end - 1);
}

static esp_err_t panel_st7306_invert_color(esp_lcd_panel_t *panel, bool invert_color_data)
{
    st7306_panel_t *st7306 = __containerof(panel, st7306_panel_t, base);
    uint8_t cmd = invert_color_data ? ST7306_CMD_INV_ON : ST7306_CMD_INV_OFF;
    return st7306_send_cmd_params(st7306, cmd, NULL, 0);
}

static esp_err_t panel_st7306_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
{
    st7306_panel_t *st7306 = __containerof(panel, st7306_panel_t, base);
    st7306->mirror_x = mirror_x;
    st7306->mirror_y = mirror_y;
    return ESP_OK;
}

static esp_err_t panel_st7306_swap_xy(esp_lcd_panel_t *panel, bool swap_axes)
{
    st7306_panel_t *st7306 = __containerof(panel, st7306_panel_t, base);
    st7306->swap_axes = swap_axes;
    return ESP_OK;
}

static esp_err_t panel_st7306_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap)
{
    st7306_panel_t *st7306 = __containerof(panel, st7306_panel_t, base);
    st7306->x_gap = x_gap;
    st7306->y_gap = y_gap;
    return ESP_OK;
}

static esp_err_t panel_st7306_disp_on_off(esp_lcd_panel_t *panel, bool on_off)
{
    st7306_panel_t *st7306 = __containerof(panel, st7306_panel_t, base);
    uint8_t cmd = on_off ? ST7306_CMD_DISP_ON : ST7306_CMD_DISP_OFF;
    ESP_RETURN_ON_ERROR(st7306_send_cmd_params(st7306, cmd, NULL, 0), TAG, "disp on/off");
    vTaskDelay(pdMS_TO_TICKS(100));
    return ESP_OK;
}
