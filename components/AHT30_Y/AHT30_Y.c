#include "AHT30_Y.h"

#include "ahtxx.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl_display.h"

#define TAG "AHT30_Y"

#define AHT30_I2C_PORT I2C_NUM_0
#define AHT30_I2C_SDA GPIO_NUM_8
#define AHT30_I2C_SCL GPIO_NUM_9
#define AHT30_FIRST_RETRY_COUNT 5
#define AHT30_FIRST_DELAY_MS 200
#define AHT30_NORMAL_RETRY_COUNT 2
#define AHT30_NORMAL_DELAY_MS 80

static i2c_master_bus_handle_t s_i2c_bus = NULL;
static ahtxx_handle_t s_aht = NULL;
static bool s_first_read = true;
static int s_consecutive_failures = 0;
#define MAX_CONSECUTIVE_FAILURES 3

void aht30_y_init(void) {
  if (s_aht) {
    return;
  }

  i2c_master_bus_config_t bus_cfg = {
      .i2c_port = AHT30_I2C_PORT,
      .sda_io_num = AHT30_I2C_SDA,
      .scl_io_num = AHT30_I2C_SCL,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7,
      .intr_priority = 0,
      .trans_queue_depth = 0,
      .flags.enable_internal_pullup = 1,
  };

  // 【修改 1】不崩溃，只返回错误
  esp_err_t err = i2c_new_master_bus(&bus_cfg, &s_i2c_bus);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "I2C 初始化失败");
    return;
  }

  ahtxx_config_t cfg = I2C_AHT30_CONFIG_DEFAULT;
  
  // 【修改 2】找不到 AHT30 不崩溃，只打印警告
  err = ahtxx_init(s_i2c_bus, &cfg, &s_aht);
  if (err != ESP_OK) {
    ESP_LOGW(TAG, "未找到 AHT30 传感器，跳过初始化");
    s_aht = NULL;
    return;
  }

  // 复位也做容错
  ahtxx_reset(s_aht);
  vTaskDelay(pdMS_TO_TICKS(200));

  ESP_LOGI(TAG, "AHT30 初始化成功");
}

void aht30_y_start(void) {
  // No-op: we read on-demand before sleep to minimize memory usage.
}

void aht30_y_read_update_ui(void) {
  if (!s_aht) {
    return;
  }
  float temp = 0.0f;
  float humi = 0.0f;
  int retries = s_first_read ? AHT30_FIRST_RETRY_COUNT
                             : AHT30_NORMAL_RETRY_COUNT;
  int delay_ms = s_first_read ? AHT30_FIRST_DELAY_MS
                              : AHT30_NORMAL_DELAY_MS;

  esp_err_t err = ESP_FAIL;
  for (int attempt = 0; attempt < retries; attempt++) {
    err = ahtxx_get_measurement(s_aht, &temp, &humi);
    if (err == ESP_OK) {
      break;
    }
    if (attempt + 1 < retries) {
      vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
  }

  if (err == ESP_OK) {
    lvgl_ui_set_temp_humi(temp, humi);
    s_first_read = false;
    s_consecutive_failures = 0;  // 连续读取成功，复位计数器
  } else {
    ESP_LOGW(TAG, "read failed: %s", esp_err_to_name(err));
    s_consecutive_failures++;

    // 连续失败超过阈值，复位传感器
    if (s_consecutive_failures >= MAX_CONSECUTIVE_FAILURES) {
      ESP_LOGW(TAG, "Too many consecutive failures (%d), resetting AHT30...",
               s_consecutive_failures);
      s_consecutive_failures = 0;

      // 方法1：先尝试软复位（不重建句柄）
      ahtxx_reset(s_aht);
      vTaskDelay(pdMS_TO_TICKS(300));

      // 立即试读一次
      float tmp_t, tmp_h;
      esp_err_t retry_err = ahtxx_get_measurement(s_aht, &tmp_t, &tmp_h);
      if (retry_err == ESP_OK) {
        ESP_LOGI(TAG, "AHT30 recovered after soft reset");
        lvgl_ui_set_temp_humi(tmp_t, tmp_h);
        s_first_read = false;
        return;
      }

      // 方法2：软复位失败，重建 I2C 设备句柄
      ESP_LOGW(TAG, "Soft reset failed (%s), re-creating device handle...",
               esp_err_to_name(retry_err));
      ahtxx_remove(s_aht);
      vTaskDelay(pdMS_TO_TICKS(50));
      ahtxx_delete(s_aht);
      s_aht = NULL;
      vTaskDelay(pdMS_TO_TICKS(100));

      ahtxx_config_t cfg = I2C_AHT30_CONFIG_DEFAULT;
      esp_err_t init_err = ahtxx_init(s_i2c_bus, &cfg, &s_aht);
      if (init_err == ESP_OK) {
        ahtxx_reset(s_aht);
        vTaskDelay(pdMS_TO_TICKS(300));
        // 复位后再试读一次
        float t, h;
        esp_err_t read_err = ahtxx_get_measurement(s_aht, &t, &h);
        if (read_err == ESP_OK) {
          ESP_LOGI(TAG, "AHT30 re-initialized successfully");
          lvgl_ui_set_temp_humi(t, h);
          s_first_read = false;
        } else {
          ESP_LOGW(TAG, "AHT30 re-init but read still fails: %s",
                   esp_err_to_name(read_err));
        }
      } else {
        ESP_LOGW(TAG, "AHT30 re-initialization failed: %s",
                 esp_err_to_name(init_err));
        s_aht = NULL;
      }
    }
  }
}

i2c_master_bus_handle_t aht30_y_get_bus(void) {
  return s_i2c_bus;
}