#include <stdio.h>
#include "Wifi_hw_Y.h"
#include "hw_time_Y.h"
#include "esp_sntp.h" //获取网络时间
#include "esp_netif_sntp.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <string.h>
#include "esp_lvgl_port.h"
#include "lvgl_display.h"
#include <sys/time.h>
#include <time.h>


#define TAG "hw_time_Y" 

#define SNTP_SERVER_0 "cn.pool.ntp.org"
#define SNTP_SERVER_1 "pool.ntp.org"
#define SNTP_SERVER_2 "time.cloudflare.com"
#define SNTP_SYNC_INTERVAL_MS (60 * 60 * 1000) // 1 hour
static bool s_sntp_started = false;
static SemaphoreHandle_t s_time_sync_sem = NULL;
static volatile bool s_time_synced = false;

static void time_sync_notification_cb(struct timeval *tv) {
  if (tv) {
    settimeofday(tv, NULL); // ensure RTC keeps the synced time
  }
  s_time_synced = true;
  if (s_time_sync_sem) {
    xSemaphoreGive(s_time_sync_sem);
  }
  ESP_LOGI(TAG, "SNTP time synced");
}

/**
 * @brief 时间同步等待任务函数
 * 该任务用于等待系统时间同步完成，并在同步后打印当前时�? * @param arg 任务参数（未使用�? */
static void time_sync_wait_task(void *arg) {
  (void)arg;
  int retry = 0;
  const int retry_count = 20;

  while (esp_sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET &&
         retry < retry_count) {
    ESP_LOGI(TAG, "Waiting for time sync... (%d/%d)", retry + 1, retry_count);
    if (s_time_sync_sem &&
        xSemaphoreTake(s_time_sync_sem, pdMS_TO_TICKS(2000)) == pdTRUE) {
      break;
    }
    retry++;
  }

  time_t now = 0;
  struct tm timeinfo = {0};
  time(&now);
  localtime_r(&now, &timeinfo);
  if (timeinfo.tm_year < (2016 - 1900)) {
    ESP_LOGW(TAG, "Time not synced yet");
  } else {
    ESP_LOGI(TAG, "Current time: %04d-%02d-%02d %02d:%02d:%02d",
             timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    s_time_synced = true;
    if (s_time_sync_sem) {
      xSemaphoreGive(s_time_sync_sem);
    }

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
    lvgl_port_lock(0);
    lvgl_ui_set_time_labels(date_buf, hour_buf, min_buf);
    lvgl_port_unlock();
  }

  vTaskDelete(NULL);
}

void hw_time_sync_start(void) {
  if (!s_sntp_started) {
    s_time_sync_sem = xSemaphoreCreateBinary();
    setenv("TZ", HW_TIME_TZ, 1);
    tzset();
    s_time_synced = false;
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
        esp_sntp_setservername(0, SNTP_SERVER_0);
    esp_sntp_setservername(1, SNTP_SERVER_1);
    esp_sntp_setservername(2, SNTP_SERVER_2);
    esp_sntp_set_sync_interval(SNTP_SYNC_INTERVAL_MS);
    esp_sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    esp_sntp_init();
    s_sntp_started = true;
    ESP_LOGI(TAG, "SNTP started, interval %d ms", SNTP_SYNC_INTERVAL_MS);
  }

  xTaskCreate(time_sync_wait_task, "time_sync_wait", 4096, NULL, 5, NULL);
}

bool hw_time_is_synced(void) {
  return s_time_synced ||
         esp_sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED;
}

bool hw_time_wait_for_sync(uint32_t timeout_ms) {
  if (hw_time_is_synced()) {
    return true;
  }
  if (s_time_sync_sem) {
    TickType_t ticks = (timeout_ms == 0) ? 0 : pdMS_TO_TICKS(timeout_ms);
    if (xSemaphoreTake(s_time_sync_sem, ticks) == pdTRUE) {
      return true;
    }
  }
  return hw_time_is_synced();
}
void hw_time_set_default_if_unset(void) {
  time_t now = 0;
  struct tm timeinfo = {0};
  time(&now);
  localtime_r(&now, &timeinfo);
  if (timeinfo.tm_year >= (2016 - 1900)) {
    return;
  }

  setenv("TZ", HW_TIME_TZ, 1);
  tzset();

  struct tm build_tm = {0};
  char month_str[4] = {0};
  sscanf(__DATE__, "%3s %d %d", month_str, &build_tm.tm_mday,
         &build_tm.tm_year);
  build_tm.tm_year -= 1900;
  build_tm.tm_isdst = -1;

  if (strcmp(month_str, "Jan") == 0) build_tm.tm_mon = 0;
  else if (strcmp(month_str, "Feb") == 0) build_tm.tm_mon = 1;
  else if (strcmp(month_str, "Mar") == 0) build_tm.tm_mon = 2;
  else if (strcmp(month_str, "Apr") == 0) build_tm.tm_mon = 3;
  else if (strcmp(month_str, "May") == 0) build_tm.tm_mon = 4;
  else if (strcmp(month_str, "Jun") == 0) build_tm.tm_mon = 5;
  else if (strcmp(month_str, "Jul") == 0) build_tm.tm_mon = 6;
  else if (strcmp(month_str, "Aug") == 0) build_tm.tm_mon = 7;
  else if (strcmp(month_str, "Sep") == 0) build_tm.tm_mon = 8;
  else if (strcmp(month_str, "Oct") == 0) build_tm.tm_mon = 9;
  else if (strcmp(month_str, "Nov") == 0) build_tm.tm_mon = 10;
  else if (strcmp(month_str, "Dec") == 0) build_tm.tm_mon = 11;

  sscanf(__TIME__, "%d:%d:%d", &build_tm.tm_hour, &build_tm.tm_min,
         &build_tm.tm_sec);

  time_t build_time = mktime(&build_tm);
  if (build_time > 0) {
    struct timeval tv = {.tv_sec = build_time, .tv_usec = 0};
    settimeofday(&tv, NULL);
    ESP_LOGW(TAG, "Time not set; using build time: %s %s", __DATE__, __TIME__);
  }
}





