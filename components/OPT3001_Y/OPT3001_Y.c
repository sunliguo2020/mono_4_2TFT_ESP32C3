#include "OPT3001_Y.h"

#include "AHT30_Y.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl_display.h"

#define TAG "OPT3001_Y"

#define OPT3001_I2C_ADDR 0x44
#define OPT3001_INT_GPIO GPIO_NUM_0
#define OPT3001_I2C_SPEED_HZ 100000
#define OPT3001_I2C_TIMEOUT_MS 150
#define OPT3001_CONV_DELAY_MS 120
#define OPT3001_FIRST_DELAY_MS 200

#define OPT3001_REG_RESULT 0x00
#define OPT3001_REG_CONFIG 0x01

// Auto-range + single-shot (100ms) conversion
#define OPT3001_CFG_SINGLE_SHOT 0xC200
#define OPT3001_CFG_CONTINUOUS_NO_LATCH 0xC600

#define OPT3001_REG_LOW_LIMIT 0x02
#define OPT3001_REG_HIGH_LIMIT 0x03
#define OPT3001_DISARM_HIGH_LUX 100000.0f

static i2c_master_dev_handle_t s_opt_dev = NULL;
static bool s_first_read = true;
static float s_last_lux = 0.0f;
static bool s_has_lux = false;
static TaskHandle_t s_int_test_task = NULL;

typedef struct {
  uint32_t period_ms;
  uint32_t lux_poll_ms;
} opt3001_int_test_cfg_t;

static opt3001_int_test_cfg_t s_int_test_cfg = {
    .period_ms = 200,
    .lux_poll_ms = 1000,
};

static esp_err_t opt3001_write_reg(uint8_t reg, uint16_t value) {
  if (!s_opt_dev) {
    return ESP_ERR_INVALID_STATE;
  }
  uint8_t tx[3] = {reg, (uint8_t)(value >> 8), (uint8_t)(value & 0xFF)};
  return i2c_master_transmit(s_opt_dev, tx, sizeof(tx),
                             OPT3001_I2C_TIMEOUT_MS);
}

static esp_err_t opt3001_read_reg(uint8_t reg, uint16_t *value) {
  if (!s_opt_dev || !value) {
    return ESP_ERR_INVALID_STATE;
  }
  uint8_t rx[2] = {0};
  esp_err_t err = i2c_master_transmit_receive(s_opt_dev, &reg, 1, rx, 2,
                                              OPT3001_I2C_TIMEOUT_MS);
  if (err != ESP_OK) {
    return err;
  }
  *value = (uint16_t)((rx[0] << 8) | rx[1]);
  return ESP_OK;
}

static uint16_t opt3001_lux_to_raw(float lux) {
  if (lux <= 0.0f) {
    return 0;
  }
  float val = lux / 0.01f;
  uint16_t exp = 0;
  while (val > 4095.0f && exp < 15) {
    val *= 0.5f;
    exp++;
  }
  uint16_t mant = (uint16_t)(val + 0.5f);
  if (mant > 0x0FFF) {
    mant = 0x0FFF;
  }
  return (uint16_t)((exp << 12) | mant);
}

// Initialize OPT3001 device on the shared I2C bus.
void opt3001_y_init(void) {
  if (s_opt_dev) {
    return;
  }

  aht30_y_init();
  i2c_master_bus_handle_t bus = aht30_y_get_bus();
  if (!bus) {
    ESP_LOGW(TAG, "I2C bus not ready");
    return;
  }

  i2c_device_config_t dev_cfg = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = OPT3001_I2C_ADDR,
      .scl_speed_hz = OPT3001_I2C_SPEED_HZ,
  };

  esp_err_t err = i2c_master_bus_add_device(bus, &dev_cfg, &s_opt_dev);
  if (err != ESP_OK) {
    ESP_LOGW(TAG, "i2c add device failed: %s", esp_err_to_name(err));
    return;
  }

  gpio_config_t io_cfg = {
      .pin_bit_mask = 1ULL << OPT3001_INT_GPIO,
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_ENABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };
  gpio_config(&io_cfg);

  // Default to continuous conversions so INT keeps a stable level.
  (void)opt3001_write_reg(OPT3001_REG_LOW_LIMIT, 0);
  (void)opt3001_write_reg(OPT3001_REG_HIGH_LIMIT,
                          opt3001_lux_to_raw(OPT3001_DISARM_HIGH_LUX));
  (void)opt3001_write_reg(OPT3001_REG_CONFIG, OPT3001_CFG_CONTINUOUS_NO_LATCH);
  vTaskDelay(pdMS_TO_TICKS(OPT3001_CONV_DELAY_MS));
}

// Read lux once and update the UI label.
void opt3001_y_read_update_ui(void) {
  if (!s_opt_dev) {
    return;
  }

  int delay_ms = s_first_read ? OPT3001_FIRST_DELAY_MS : 0;
  if (delay_ms > 0) {
    vTaskDelay(pdMS_TO_TICKS(delay_ms));
  }

  uint16_t raw = 0;
  esp_err_t err = opt3001_read_reg(OPT3001_REG_RESULT, &raw);
  if (err != ESP_OK) {
    ESP_LOGW(TAG, "read failed: %s", esp_err_to_name(err));
    return;
  }

  uint16_t exponent = (raw >> 12) & 0x0F;
  uint16_t mantissa = raw & 0x0FFF;
  float lux = (float)mantissa * (1 << exponent) * 0.01f;

  lvgl_ui_set_light_lux(lux);
  s_last_lux = lux;
  s_has_lux = true;
  s_first_read = false;
}

// Retrieve the last lux reading (if available).
bool opt3001_y_get_last_lux(float *lux) {
  if (!s_has_lux || !lux) {
    return false;
  }
  *lux = s_last_lux;
  return true;
}

// Configure OPT3001 interrupt thresholds and polarity.
void opt3001_y_config_interrupt(float low_lux, float high_lux,
                                bool active_low) {
  if (!s_opt_dev) {
    return;
  }

  uint16_t low_raw = opt3001_lux_to_raw(low_lux);
  uint16_t high_raw = opt3001_lux_to_raw(high_lux);

  (void)opt3001_write_reg(OPT3001_REG_LOW_LIMIT, low_raw);
  (void)opt3001_write_reg(OPT3001_REG_HIGH_LIMIT, high_raw);

  uint16_t cfg = OPT3001_CFG_CONTINUOUS_NO_LATCH;
  if (!active_low) {
    cfg |= 0x0008;  // POL=1 active high
  }
  (void)opt3001_write_reg(OPT3001_REG_CONFIG, cfg);

  // Give the device time to update INT state in continuous mode.
  vTaskDelay(pdMS_TO_TICKS(OPT3001_CONV_DELAY_MS));
  uint16_t raw = 0;
  if (opt3001_read_reg(OPT3001_REG_RESULT, &raw) == ESP_OK) {
    uint16_t exponent = (raw >> 12) & 0x0F;
    uint16_t mantissa = raw & 0x0FFF;
    float lux = (float)mantissa * (1 << exponent) * 0.01f;
    s_last_lux = lux;
    s_has_lux = true;
  }
}

// Return the INT GPIO assigned to OPT3001.
gpio_num_t opt3001_y_get_int_gpio(void) {
  return OPT3001_INT_GPIO;
}

static void opt3001_int_test_task(void *arg) {
  (void)arg;
  gpio_num_t int_gpio = opt3001_y_get_int_gpio();
  TickType_t last_lux_tick = 0;

  while (true) {
    int level = gpio_get_level(int_gpio);
    TickType_t now = xTaskGetTickCount();
    if (now - last_lux_tick >= pdMS_TO_TICKS(s_int_test_cfg.lux_poll_ms)) {
      opt3001_y_read_update_ui();
      last_lux_tick = now;
    }

    float lux = 0.0f;
    bool has_lux = opt3001_y_get_last_lux(&lux);
    if (has_lux) {
      ESP_LOGI(TAG, "int=%d lux=%.1f", level, lux);
    } else {
      ESP_LOGI(TAG, "int=%d lux=NA", level);
    }
    vTaskDelay(pdMS_TO_TICKS(s_int_test_cfg.period_ms));
  }
}

// Start a periodic INT debug log task (used during bring-up).
void opt3001_y_start_int_test(uint32_t period_ms, uint32_t lux_poll_ms) {
  s_int_test_cfg.period_ms = period_ms;
  s_int_test_cfg.lux_poll_ms = lux_poll_ms;
  if (s_int_test_task) {
    return;
  }
  xTaskCreate(opt3001_int_test_task, "opt3001_int_test", 4096, NULL, 4,
              &s_int_test_task);
}

// Run a deep sleep wake test using only OPT3001 INT.
void opt3001_y_run_deepsleep_test(bool wake_low, int awake_seconds) {
  gpio_num_t int_gpio = opt3001_y_get_int_gpio();
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  uint64_t mask = (1ULL << int_gpio);
  esp_deepsleep_gpio_wake_up_mode_t mode =
      wake_low ? ESP_GPIO_WAKEUP_GPIO_LOW : ESP_GPIO_WAKEUP_GPIO_HIGH;
  esp_err_t wk = esp_deep_sleep_enable_gpio_wakeup(mask, mode);
  ESP_LOGI(TAG, "deep sleep wake cfg: gpio=%d mode=%s ret=0x%x",
           (int)int_gpio, wake_low ? "LOW" : "HIGH", (unsigned)wk);

  for (int i = 0; i < awake_seconds; i++) {
    ESP_LOGI(TAG, "awake tick %d", i + 1);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
  ESP_LOGI(TAG, "enter deep sleep");
  esp_deep_sleep_start();
}
