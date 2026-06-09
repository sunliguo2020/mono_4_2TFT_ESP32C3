#include "custom.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "events_init.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "gui_guider.h"
#include "setup_scr_screen_main_top_bar.h"
#include "setup_scr_screen_main_time.h"
#include "setup_scr_screen_main_bottom.h"
#include "setup_scr_screen_main_temp_humi.h"

static const char *TAG = "lvgl_ui";

lv_ui guider_ui;

// UI???????UI?????????UI?????
typedef enum {
  UI_MSG_WIFI_ONLINE = 0,
  UI_MSG_WIFI_OFFLINE,
  UI_MSG_TIME_LABELS,
  UI_MSG_TIME_SYMBOL,
  UI_MSG_TEMP_HUMI,
  UI_MSG_LIGHT_LUX,
  UI_MSG_SLEEP_SCREEN,
  UI_MSG_BATTERY,
  UI_MSG_PRESENCE,
  UI_MSG_WEATHER
} ui_msg_type_t;

typedef struct {
  ui_msg_type_t type;
  union {
    struct {
      char date[16];
      char hour[4];
      char min[4];
    } time_labels;
    struct {
      uint32_t hours;
      uint32_t minutes;
    } hm;
    struct {
      float temp;
      float humi;
    } th;
    float lux;
    bool show_sleep;
    bool presence;
    char text[16];
    struct {
      uint32_t mv;
      uint32_t percent;
    } bat;
    struct {
      char text[16];
      char temp[16];
      char humi[16];
    } weather;
  } data;
} ui_msg_t;

static QueueHandle_t s_ui_queue = NULL;
static TaskHandle_t s_ui_task = NULL;
static uint8_t s_ui_lock_fail = 0;
static volatile bool s_ui_paused = false;

LV_IMAGE_DECLARE(_keep_stu_I4_34x32);
LV_IMAGE_DECLARE(_out_line_I4_39x43);

static void ensure_screen_main(void) {
  if (!guider_ui.screen_main || guider_ui.screen_main_del) {
    setup_scr_screen_main(&guider_ui);
    setup_scr_screen_main_top_bar(&guider_ui);
    setup_scr_screen_main_time(&guider_ui);
    setup_scr_screen_main_bottom(&guider_ui);
    setup_scr_screen_main_temp_humi(&guider_ui);
    guider_ui.screen_main_del = false;
  }
}

static void ensure_screen_sleep(void) {
  if (!guider_ui.screen_sleep || guider_ui.screen_sleep_del) {
    setup_scr_screen_sleep(&guider_ui);
    guider_ui.screen_sleep_del = false;
  }
}

// ???????????????????UI????
static void ui_queue_send(const ui_msg_t *msg) {
  if (!s_ui_queue || !msg) {
    return;
  }
  if (xQueueSend(s_ui_queue, msg, 0) != pdTRUE) {
    ui_msg_t drop;
    (void)xQueueReceive(s_ui_queue, &drop, 0);
    (void)xQueueSend(s_ui_queue, msg, 0);
  }
}

// UI????????????LVGL API????
static void ui_task(void *arg) {
  (void)arg;
  ui_msg_t msg;
  for (;;) {
    if (s_ui_paused) {
      vTaskDelay(pdMS_TO_TICKS(10));
      continue;
    }
    if (xQueueReceive(s_ui_queue, &msg, portMAX_DELAY) != pdTRUE) {
      continue;
    }
    if (!lvgl_port_lock(200)) {
      ESP_LOGW(TAG, "lvgl lock timeout, drop ui msg %d", (int)msg.type);
      continue;
    }
    s_ui_lock_fail = 0;
    switch (msg.type) {
      case UI_MSG_WIFI_ONLINE:
        if (guider_ui.screen_main_img_1) {
          lv_image_set_src(guider_ui.screen_main_img_1, &_wifi_fill_I4_33x31);
        }
        break;
      case UI_MSG_WIFI_OFFLINE:
        if (guider_ui.screen_main_img_1) {
          lv_image_set_src(guider_ui.screen_main_img_1, &_wifi_down_I4_33x31);
        }
        break;
      case UI_MSG_TIME_LABELS:
        if (guider_ui.screen_main_label_date &&
            guider_ui.screen_main_label_time_hour &&
            guider_ui.screen_main_label_time_min) {
          lv_label_set_text(guider_ui.screen_main_label_date,
                            msg.data.time_labels.date);
          lv_label_set_text(guider_ui.screen_main_label_time_hour,
                            msg.data.time_labels.hour);
          lv_label_set_text(guider_ui.screen_main_label_time_min,
                            msg.data.time_labels.min);
        }
        break;
      case UI_MSG_TIME_SYMBOL:
        if (guider_ui.screen_main_label_time_update) {
          lv_label_set_text(guider_ui.screen_main_label_time_update,
                            msg.data.text);
        }
        break;
      case UI_MSG_TEMP_HUMI:
        if (guider_ui.screen_main_label_temp_val &&
            guider_ui.screen_main_label_humi_val) {
          char temp_buf[8];
          char humi_buf[8];
          snprintf(temp_buf, sizeof(temp_buf), "%.1f", msg.data.th.temp);
          snprintf(humi_buf, sizeof(humi_buf), "%.1f", msg.data.th.humi);
          lv_label_set_text(guider_ui.screen_main_label_temp_val, temp_buf);
          lv_label_set_text(guider_ui.screen_main_label_humi_val, humi_buf);
        }
        break;
      case UI_MSG_LIGHT_LUX:
        if (guider_ui.screen_main_label_light_val) {
          char lux_buf[12];
          snprintf(lux_buf, sizeof(lux_buf), "%.0f", msg.data.lux);
          lv_label_set_text(guider_ui.screen_main_label_light_val, lux_buf);
        }
        break;
      case UI_MSG_SLEEP_SCREEN:
        if (msg.data.show_sleep) {
          ensure_screen_sleep();
          lv_screen_load(guider_ui.screen_sleep);
        } else {
          ensure_screen_main();
          lv_screen_load(guider_ui.screen_main);
        }
        break;
      case UI_MSG_BATTERY:
        if (guider_ui.screen_main_label_bat_val) {
          char buf[12];
          snprintf(buf, sizeof(buf), "%u%%", (unsigned)msg.data.bat.percent);
          lv_label_set_text(guider_ui.screen_main_label_bat_val, buf);
        }
        if (guider_ui.screen_main_label_17) {
          const char *sym = LV_SYMBOL_BATTERY_FULL;
          uint32_t pct = msg.data.bat.percent;
          if (pct <= 10) {
            sym = LV_SYMBOL_BATTERY_EMPTY;
          } else if (pct <= 35) {
            sym = LV_SYMBOL_BATTERY_1;
          } else if (pct <= 65) {
            sym = LV_SYMBOL_BATTERY_2;
          } else if (pct <= 90) {
            sym = LV_SYMBOL_BATTERY_3;
          } else {
            sym = LV_SYMBOL_BATTERY_FULL;
          }
          lv_label_set_text(guider_ui.screen_main_label_17, sym);
        }
        break;
      case UI_MSG_PRESENCE:
        if (guider_ui.screen_main_img_5) {
          if (msg.data.presence) {
            lv_image_set_src(guider_ui.screen_main_img_5, &_Human_Presence_Sensor_RGB565A8_36x37);
            lv_obj_clear_flag(guider_ui.screen_main_img_5, LV_OBJ_FLAG_HIDDEN);
          } else {
            lv_obj_add_flag(guider_ui.screen_main_img_5, LV_OBJ_FLAG_HIDDEN);
          }
        }
        break;
      case UI_MSG_WEATHER:
        // ???????
        if (guider_ui.screen_main_label_5) {
          lv_label_set_text(guider_ui.screen_main_label_5, msg.data.weather.text);
        }
        // ???????
        if (guider_ui.screen_main_label_20) {
          lv_label_set_text(guider_ui.screen_main_label_20, msg.data.weather.temp);
        }
        // screen_main_label_18 - ℃符号（使用 weather_20 字体）
        if (guider_ui.screen_main_label_18) {
          lv_label_set_text(guider_ui.screen_main_label_18, "℃");
        }
        // ???????
        if (guider_ui.screen_main_label_run_hour) {
          lv_label_set_text(guider_ui.screen_main_label_run_hour, msg.data.weather.humi);
        }
        if (guider_ui.screen_main_label_run_min) {
          lv_label_set_text(guider_ui.screen_main_label_run_min, "");
        }
        if (guider_ui.screen_main_label_2) {
          lv_label_set_text(guider_ui.screen_main_label_2, "");
        }
        // 隐藏旧天气标签（数据已分行显示在其他标签上）
        if (guider_ui.screen_main_label_weather) {
          lv_obj_add_flag(guider_ui.screen_main_label_weather, LV_OBJ_FLAG_HIDDEN);
        }
        break;
      default:
        break;
    }
    lv_display_t *disp = lv_display_get_default();
    if (disp) {
      lv_refr_now(disp);
    }
    lvgl_port_unlock();
  }
}

// ???UI??????
static void lvgl_ui_task_init(void) {
  if (s_ui_queue && s_ui_task) {
    return;
  }
  s_ui_queue = xQueueCreate(16, sizeof(ui_msg_t));
  if (!s_ui_queue) {
    ESP_LOGE(TAG, "failed to create ui queue");
    return;
  }
  xTaskCreate(ui_task, "ui_task", 8192, NULL, 6, &s_ui_task);
}

bool lvgl_ui_wait_idle(uint32_t timeout_ms) {
  if (!s_ui_queue) {
    return true;
  }

  // Ensure LVGL task is awake to drain pending UI updates.
  lvgl_port_task_wake(LVGL_PORT_EVENT_DISPLAY, NULL);

  const TickType_t start = xTaskGetTickCount();
  const TickType_t timeout_ticks = pdMS_TO_TICKS(timeout_ms);

  while (uxQueueMessagesWaiting(s_ui_queue) > 0) {
    if (timeout_ms == 0) {
      return false;
    }
    if ((xTaskGetTickCount() - start) >= timeout_ticks) {
      UBaseType_t pending = uxQueueMessagesWaiting(s_ui_queue);
      ESP_LOGW(TAG, "ui queue not drained within %u ms (pending=%u)",
               (unsigned)timeout_ms, (unsigned)pending);
      return false;
    }
    vTaskDelay(pdMS_TO_TICKS(5));
  }

  return true;
}

void lvgl_ui_pause(bool pause) {
  s_ui_paused = pause;
}

/**
 * @brief LVGL???????????
 *
 * ????????LVGL???????????UI???
 */
void lvgl_gui_init(void) {
  setup_ui(&guider_ui);
  guider_ui.screen_main_del = false;
  lvgl_ui_task_init();
}

// Wi-Fi ?????
void wifi_online(void) {
  ui_msg_t msg = {.type = UI_MSG_WIFI_ONLINE};
  ui_queue_send(&msg);
}

// Wi-Fi ?????
void wifi_downline(void) {
  ui_msg_t msg = {.type = UI_MSG_WIFI_OFFLINE};
  ui_queue_send(&msg);
}

// ?????????/??/????
void lvgl_ui_set_time_labels(const char *date_str, const char *hour_str,
                             const char *min_str) {
  ui_msg_t msg = {.type = UI_MSG_TIME_LABELS};
  snprintf(msg.data.time_labels.date, sizeof(msg.data.time_labels.date), "%s",
           date_str ? date_str : "");
  snprintf(msg.data.time_labels.hour, sizeof(msg.data.time_labels.hour), "%s",
           hour_str ? hour_str : "");
  snprintf(msg.data.time_labels.min, sizeof(msg.data.time_labels.min), "%s",
           min_str ? min_str : "");
  ui_queue_send(&msg);
}

// ?????????/????
void lvgl_ui_set_time_update_symbol(const char *symbol_text) {
  ui_msg_t msg = {.type = UI_MSG_TIME_SYMBOL};
  snprintf(msg.data.text, sizeof(msg.data.text), "%s",
           symbol_text ? symbol_text : "");
  ui_queue_send(&msg);
}

// ??/??????????
void lvgl_ui_set_temp_humi(float temp, float humi) {
  ui_msg_t msg = {.type = UI_MSG_TEMP_HUMI};
  msg.data.th.temp = temp;
  msg.data.th.humi = humi;
  ui_queue_send(&msg);
}

// ????????(lux)
void lvgl_ui_set_light_lux(float lux) {
  ui_msg_t msg = {.type = UI_MSG_LIGHT_LUX};
  msg.data.lux = lux;
  ui_queue_send(&msg);
}

// Update battery label and icon.
void lvgl_ui_set_battery(uint32_t millivolt, uint32_t percent) {
  ui_msg_t msg = {.type = UI_MSG_BATTERY};
  msg.data.bat.mv = millivolt;
  msg.data.bat.percent = percent;
  ui_queue_send(&msg);
}

// Switch between sleep screen and main screen.
void lvgl_ui_show_sleep_screen(bool show_sleep) {
  ui_msg_t msg = {.type = UI_MSG_SLEEP_SCREEN};
  msg.data.show_sleep = show_sleep;
  ui_queue_send(&msg);
}

// Update presence icon.
void lvgl_ui_set_presence(bool present) {
  ui_msg_t msg = {.type = UI_MSG_PRESENCE};
  msg.data.presence = present;
  ui_queue_send(&msg);
}

// Update weather label.
void lvgl_ui_set_weather(const char *text, const char *temp, const char *icon) {
  ui_msg_t msg = {.type = UI_MSG_WEATHER};
  snprintf(msg.data.weather.text, sizeof(msg.data.weather.text), "%s",
           text ? text : "");
  snprintf(msg.data.weather.temp, sizeof(msg.data.weather.temp), "%s",
           temp ? temp : "");
  snprintf(msg.data.weather.humi, sizeof(msg.data.weather.humi), "%s",
           icon ? icon : "");
  ui_queue_send(&msg);
}
