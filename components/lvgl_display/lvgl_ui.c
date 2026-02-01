#include "custom.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "events_init.h"
#include "freertos/FreeRTOS.h"
#include "gui_guider.h"

static const char *TAG = "lvgl_ui";



static void tick_time_min_cb(lv_timer_t *timer) {
  LV_UNUSED(timer);
  static uint8_t val = 0;
  if (!guider_ui.screen_main_label_time_min ||
      !guider_ui.screen_main_label_time_hour) {
    return;
  }
  char buf[4];
  snprintf(buf, sizeof(buf), "%02u", (unsigned int)val);
  lv_label_set_text(guider_ui.screen_main_label_time_min, buf);
  buf[1] = buf[0];
  buf[2] = '\0';
  lv_label_set_text(guider_ui.screen_main_label_time_hour, buf);
  val = (uint8_t)((val + 1) % 60);
}

static void tick_label_3_cb(lv_timer_t *timer) {
  LV_UNUSED(timer);
  static uint16_t val = 0;
  if (!guider_ui.screen_main_label_3) {
    return;
  }
  char buf[4];
  snprintf(buf, sizeof(buf), "%02u", (unsigned int)(val % 100));
  lv_label_set_text(guider_ui.screen_main_label_3, buf);
  val = (uint16_t)(val + 1);
}

lv_ui guider_ui; // 全局变量，用于保存界面信息

/**
 * @brief LVGL图形用户界面初始化函数
 *
 * 该函数用于初始化LVGL图形用户界面，包括设置UI界面、初始化事件处理和自定义功能。
 */

void lvgl_gui_init(void) {

  // 调用setup_ui函数初始化基本UI界面
  setup_ui(&guider_ui);
  // 调用events_init函数初始化事件处理系统
//   events_init(&guider_ui);
//   // 调用custom_init函数初始化自定义功能模块
//   custom_init(&guider_ui);

//   if (guider_ui.screen_main_label_time_min) {
//     lv_timer_create(tick_time_min_cb, 1000, NULL);
//   }
//   if (guider_ui.screen_main_label_3) {
//     lv_timer_create(tick_label_3_cb, 1000, NULL);
//   }
  
}
void wifi_online(void) {
  lvgl_port_lock(0);
  lv_image_set_src(guider_ui.screen_main_img_1, &_wifi_fill_I4_33x31);
  lvgl_port_unlock();
}
void wifi_downline(void) {
  lvgl_port_lock(0);
  lv_image_set_src(guider_ui.screen_main_img_1, &_wifi_down_I4_33x31);
  lvgl_port_unlock();
}
void lvgl_ui_set_time_labels(const char *date_str, const char *hour_str,
                             const char *min_str) {
  if (!guider_ui.screen_main_label_date ||
      !guider_ui.screen_main_label_time_hour ||
      !guider_ui.screen_main_label_time_min) {
    return;
  }
  lv_label_set_text(guider_ui.screen_main_label_date, date_str ? date_str : "");
  lv_label_set_text(guider_ui.screen_main_label_time_hour,
                    hour_str ? hour_str : "");
  lv_label_set_text(guider_ui.screen_main_label_time_min,
                    min_str ? min_str : "");
}

LV_IMAGE_DECLARE(_keep_stu_I4_34x32);
LV_IMAGE_DECLARE(_out_line_I4_39x43);

void lvgl_ui_set_pc_state_image(bool is_on) {
  if (!guider_ui.screen_main_img_PC_statee) {
    return;
  }
  const void *src = is_on ? (const void *)&_keep_stu_I4_34x32
                          : (const void *)&_out_line_I4_39x43;
  lv_image_set_src(guider_ui.screen_main_img_PC_statee, src);
}
