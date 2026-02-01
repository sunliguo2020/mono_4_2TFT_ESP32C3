#include "sleep.h"

#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_lvgl_port.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "gui_guider.h"
#include "lvgl_display.h"
#include "Wifi_hw_Y.h"
#include <time.h>

#define TAG "sleep"
#define SLEEP_WAKEUP_SEC 60
#define UPDATE_WAIT_TIMEOUT_MS 20000

#define UPDATE_BIT_HTTP (1U << 0)
#define UPDATE_BIT_BAT  (1U << 1)
#define UPDATE_BIT_STATE (1U << 2)

static TaskHandle_t s_sleep_task = NULL;
static sleep_update_cb_t s_http_update_cb = NULL;
static sleep_update_cb_t s_battery_update_cb = NULL;
static sleep_update_cb_t s_state_update_cb = NULL;
static EventGroupHandle_t s_update_event_group = NULL;

typedef struct {
  sleep_update_cb_t cb;
  EventBits_t bit;
} update_task_ctx_t;

static bool ensure_wifi_connected(TickType_t wait_ticks) {
  if (wifi_is_connected()) {
    esp_wifi_set_ps(WIFI_PS_NONE);
    return true;
  }

  ESP_LOGW(TAG, "wifi not connected, try reconnect");
  for (int i = 0; i < 3; i++) {
    esp_wifi_connect();
    if (wifi_wait_connected(wait_ticks)) {
      esp_wifi_set_ps(WIFI_PS_NONE);
      return true;
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }

  return false;
}

static void update_time_labels(void) {
  time_t now = 0;
  struct tm timeinfo = {0};
  time(&now);
  localtime_r(&now, &timeinfo);

  char date_buf[16];
  char hour_buf[4];
  char min_buf[4];

  if (strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", &timeinfo) == 0) {
    date_buf[0] = '\0';
  }
  if (strftime(hour_buf, sizeof(hour_buf), "%H", &timeinfo) == 0) {
    hour_buf[0] = '\0';
  }
  if (strftime(min_buf, sizeof(min_buf), "%M", &timeinfo) == 0) {
    min_buf[0] = '\0';
  }

  ESP_LOGI(TAG, "update_time_labels: %s %s:%s", date_buf, hour_buf, min_buf);
  lvgl_ui_set_time_labels(date_buf, hour_buf, min_buf);
}

static void update_task(void *arg) {
  update_task_ctx_t *ctx = (update_task_ctx_t *)arg;
  if (ctx && ctx->cb) {
    ctx->cb();
  }
  if (s_update_event_group && ctx) {
    xEventGroupSetBits(s_update_event_group, ctx->bit);
  }
  if (ctx) {
    free(ctx);
  }
  vTaskDelete(NULL);
}

static void run_update_tasks_and_wait(void) {
  EventBits_t wait_mask = 0;

  if (!s_update_event_group) {
    s_update_event_group = xEventGroupCreate();
  }
  if (!s_update_event_group) {
    ESP_LOGW(TAG, "update event group alloc failed");
    return;
  }
  xEventGroupClearBits(s_update_event_group,
                       UPDATE_BIT_HTTP | UPDATE_BIT_BAT | UPDATE_BIT_STATE);

  if (s_http_update_cb) {
    update_task_ctx_t *ctx = (update_task_ctx_t *)calloc(1, sizeof(*ctx));
    if (ctx) {
      ctx->cb = s_http_update_cb;
      ctx->bit = UPDATE_BIT_HTTP;
      wait_mask |= UPDATE_BIT_HTTP;
      xTaskCreate(update_task, "upd_http", 4096, ctx, 5, NULL);
    }
  }
  if (s_battery_update_cb) {
    update_task_ctx_t *ctx = (update_task_ctx_t *)calloc(1, sizeof(*ctx));
    if (ctx) {
      ctx->cb = s_battery_update_cb;
      ctx->bit = UPDATE_BIT_BAT;
      wait_mask |= UPDATE_BIT_BAT;
      xTaskCreate(update_task, "upd_bat", 3072, ctx, 5, NULL);
    }
  }
  if (s_state_update_cb) {
    update_task_ctx_t *ctx = (update_task_ctx_t *)calloc(1, sizeof(*ctx));
    if (ctx) {
      ctx->cb = s_state_update_cb;
      ctx->bit = UPDATE_BIT_STATE;
      wait_mask |= UPDATE_BIT_STATE;
      xTaskCreate(update_task, "upd_state", 3072, ctx, 5, NULL);
    }
  }

  if (wait_mask) {
    (void)xEventGroupWaitBits(s_update_event_group, wait_mask, pdTRUE, pdTRUE,
                              pdMS_TO_TICKS(UPDATE_WAIT_TIMEOUT_MS));
  }
}

static void sleep_task(void *arg) {
  (void)arg;
  while (true) {
    ESP_LOGI(TAG, "sleep loop tick");

    time_t now = 0;
    struct tm timeinfo = {0};
    time(&now);
    localtime_r(&now, &timeinfo);
    // Update immediately so UI doesn't lag up to a minute after sync
    lvgl_port_lock(0);
    update_time_labels();
    lv_obj_invalidate(lv_scr_act());
    lv_refr_now(NULL);
    lvgl_port_unlock();

    int sleep_sec = 60 - timeinfo.tm_sec;
    if (sleep_sec <= 0) {
      sleep_sec = 60;
    }

    if (!ensure_wifi_connected(pdMS_TO_TICKS(5000))) {
      ESP_LOGW(TAG, "skip updates, wifi not connected");
    } else {
      run_update_tasks_and_wait();
    }

    lvgl_port_lock(0);
    lv_refr_now(NULL);
    lvgl_port_unlock();

    esp_err_t wk_ret = esp_sleep_enable_timer_wakeup(
        (uint64_t)sleep_sec * 1000000ULL);
    ESP_LOGI(TAG, "enter light sleep for %d sec, wake cfg ret=0x%x",
             sleep_sec, (unsigned int)wk_ret);
    esp_err_t slp_ret = esp_light_sleep_start();
    ESP_LOGI(TAG, "woke from light sleep, ret=0x%x, cause=%d",
             (unsigned int)slp_ret, (int)esp_sleep_get_wakeup_cause());
    lvgl_display_wakeup();
    esp_wifi_set_ps(WIFI_PS_NONE);
    vTaskDelay(pdMS_TO_TICKS(200));

    lvgl_port_lock(0);
    update_time_labels();
    lv_obj_invalidate(lv_scr_act());
    lv_refr_now(NULL);
    lvgl_port_unlock();
  }
}

void sleep_init(void) {
  if (s_sleep_task) {
    return;
  }
  xTaskCreate(sleep_task, "sleep_task", 8192, NULL, 5, &s_sleep_task);
}

void sleep_start_light(void) {
  sleep_init();
}

void sleep_register_http_update_cb(sleep_update_cb_t cb) {
  s_http_update_cb = cb;
}

void sleep_register_battery_update_cb(sleep_update_cb_t cb) {
  s_battery_update_cb = cb;
}

void sleep_register_state_update_cb(sleep_update_cb_t cb) {
  s_state_update_cb = cb;
}
