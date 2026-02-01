#ifndef ST7306_4P2_BW_DISPLAY_DRIVER_H
#define ST7306_4P2_BW_DISPLAY_DRIVER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "MONO_TFT.h"
#include "driver/spi_master.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ST7306_COLOR_00 (0x00)
#define ST7306_COLOR_01 (0x01)
#define ST7306_COLOR_10 (0x02)
#define ST7306_COLOR_11 (0x03)

#define ST73XX_COLOR_BLACK ST7306_COLOR_11
#define ST73XX_COLOR_WHITE ST7306_COLOR_00

#ifndef TFT_WIDTH
#define TFT_WIDTH 300
#endif

#ifndef TFT_HEIGHT
#define TFT_HEIGHT 400
#endif

#define ST7306_4P2_BW_LCD_WIDTH TFT_WIDTH
#define ST7306_4P2_BW_LCD_HEIGHT TFT_HEIGHT
#define ST7306_4P2_BW_LCD_DATA_WIDTH (ST7306_4P2_BW_LCD_WIDTH / 2)
#define ST7306_4P2_BW_LCD_DATA_HEIGHT (ST7306_4P2_BW_LCD_HEIGHT / 2)
#define ST7306_4P2_BW_BUFFER_LEN (ST7306_4P2_BW_LCD_WIDTH * ST7306_4P2_BW_LCD_HEIGHT / 4)

typedef struct {
    int pin_dc;
    int pin_rst;
    int pin_cs;
    int pin_sclk;
    int pin_mosi;
    spi_host_device_t spi_host;
    int spi_clock_hz;
    bool use_hw_cs;
} st7306_4p2_bw_config_t;

typedef struct st7306_4p2_bw_t st7306_4p2_bw_t;

esp_err_t st7306_4p2_bw_init(st7306_4p2_bw_t **out, const st7306_4p2_bw_config_t *cfg);
void st7306_4p2_bw_deinit(st7306_4p2_bw_t *dev);

esp_err_t st7306_4p2_bw_fill(st7306_4p2_bw_t *dev, uint8_t data);
esp_err_t st7306_4p2_bw_clear(st7306_4p2_bw_t *dev);
esp_err_t st7306_4p2_bw_write_point(st7306_4p2_bw_t *dev, uint32_t x, uint32_t y, uint16_t data);
esp_err_t st7306_4p2_bw_display(st7306_4p2_bw_t *dev);
esp_err_t st7306_4p2_bw_display_area(st7306_4p2_bw_t *dev, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2);

esp_err_t st7306_4p2_bw_low_power_mode(st7306_4p2_bw_t *dev);
esp_err_t st7306_4p2_bw_high_power_mode(st7306_4p2_bw_t *dev);
esp_err_t st7306_4p2_bw_display_on(st7306_4p2_bw_t *dev, bool enabled);
esp_err_t st7306_4p2_bw_display_sleep(st7306_4p2_bw_t *dev, bool enabled);
esp_err_t st7306_4p2_bw_display_inversion(st7306_4p2_bw_t *dev, bool enabled);

uint8_t *st7306_4p2_bw_buffer(st7306_4p2_bw_t *dev);
size_t st7306_4p2_bw_buffer_len(st7306_4p2_bw_t *dev);

#ifdef __cplusplus
}
#endif

#endif
