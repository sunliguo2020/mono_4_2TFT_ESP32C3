#pragma once

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
void lvgl_ui_set_pc_state_image(bool is_on);
#ifdef __cplusplus
}
#endif
