#pragma once

#include "esp_err.h"
#include "esp_lcd_panel_dev.h"
#include "esp_lcd_panel_io.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t width;
    uint16_t height;
} esp_lcd_panel_st7306_bw_config_t;

/**
 * @brief Create a new ST7306 4.2" BW panel instance
 *
 * @param io LCD panel IO handle
 * @param panel_dev_config Panel device configuration
 * @param ret_panel Returned panel handle
 * @return esp_err_t
 */
esp_err_t esp_lcd_new_panel_st7306_bw(const esp_lcd_panel_io_handle_t io,
                                      const esp_lcd_panel_dev_config_t *panel_dev_config,
                                      esp_lcd_panel_handle_t *ret_panel);

#ifdef __cplusplus
}
#endif
