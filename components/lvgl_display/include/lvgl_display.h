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
void lvgl_ui_set_pc_state_text(const char *text);
void lvgl_ui_set_fans_count(uint32_t count);
void lvgl_ui_set_time_update_symbol(const char *symbol_text);
void lvgl_ui_set_pc_session_time(uint32_t hours, uint32_t minutes);
void lvgl_ui_set_pc_total_time(uint32_t hours, uint32_t minutes);
void lvgl_ui_set_fan_update_symbol(const char *symbol_text);
bool lvgl_ui_wait_idle(uint32_t timeout_ms);
void lvgl_ui_set_temp_humi(float temp, float humi);
void lvgl_ui_set_light_lux(float lux);
void lvgl_ui_set_battery(uint32_t millivolt, uint32_t percent);
void lvgl_ui_pause(bool pause);
void lvgl_ui_show_sleep_screen(bool show_sleep);
void lvgl_ui_set_presence(bool present);
#ifdef __cplusplus
}
#endif
