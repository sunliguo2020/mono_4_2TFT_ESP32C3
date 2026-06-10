#pragma once

#include <stdint.h>
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

lv_display_t *lvgl_display_init(void);
void lvgl_gui_init(void);
void wifi_online(void);
void wifi_downline(void);//wifi断开
void lvgl_ui_set_time_labels(const char *date_str, const char *hour_str,
                             const char *min_str);
void lvgl_display_wakeup(void);
void lvgl_display_sleep(bool sleep);
void lvgl_display_low_power(bool enable);
void lvgl_ui_set_time_update_symbol(const char *symbol_text);
bool lvgl_ui_wait_idle(uint32_t timeout_ms);
void lvgl_ui_set_temp_humi(float temp, float humi);
void lvgl_ui_set_light_lux(float lux);
void lvgl_ui_set_battery(uint32_t millivolt, uint32_t percent);
void lvgl_ui_pause(bool pause);
void lvgl_ui_show_sleep_screen(bool show_sleep);
void lvgl_ui_set_presence(bool present);
void lvgl_ui_set_weather(const char *today_text, const char *today_temp, const char *tomorrow_text, const char *tomorrow_temp);
#ifdef __cplusplus
}
#endif
