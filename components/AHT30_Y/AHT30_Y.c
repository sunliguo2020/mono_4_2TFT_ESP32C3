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

  ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &s_i2c_bus));

  ahtxx_config_t cfg = I2C_AHT30_CONFIG_DEFAULT;
  ESP_ERROR_CHECK(ahtxx_init(s_i2c_bus, &cfg, &s_aht));
  ESP_ERROR_CHECK(ahtxx_reset(s_aht));
  vTaskDelay(pdMS_TO_TICKS(200));
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
  } else {
    ESP_LOGW(TAG, "read failed: %s", esp_err_to_name(err));
  }
}

i2c_master_bus_handle_t aht30_y_get_bus(void) {
  return s_i2c_bus;
}
