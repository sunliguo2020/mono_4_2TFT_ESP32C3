#include "BATTERY_Y.h"

#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "lvgl_display.h"
#include "driver/gpio.h"

#define TAG "BATTERY_Y"

// AO3400 Control Pin (IO7)
#define BAT_ADC_CTRL_PIN GPIO_NUM_7

// GPIO3 -> ADC1_CH3 on ESP32-C3
#define BAT_ADC_UNIT ADC_UNIT_1
#define BAT_ADC_CH ADC_CHANNEL_3
#define BAT_ADC_ATTEN ADC_ATTEN_DB_11
#define BAT_ADC_BITWIDTH ADC_BITWIDTH_DEFAULT

// Divider: 10k (to battery) + 22k (to GND)
// Vadc = Vbat * 22 / (10 + 22)
#define BAT_DIV_NUM 32.0f
#define BAT_DIV_DEN 22.0f

#define BAT_VOLT_MIN_MV 3300
#define BAT_VOLT_MAX_MV 4200

static adc_oneshot_unit_handle_t s_adc = NULL;
static adc_cali_handle_t s_cali = NULL;
static bool s_inited = false;

static bool battery_adc_init(void) {
  if (s_inited) {
    return true;
  }

  adc_oneshot_unit_init_cfg_t unit_cfg = {
      .unit_id = BAT_ADC_UNIT,
      .ulp_mode = ADC_ULP_MODE_DISABLE,
  };
  if (adc_oneshot_new_unit(&unit_cfg, &s_adc) != ESP_OK) {
    ESP_LOGE(TAG, "adc unit init failed");
    return false;
  }

  adc_oneshot_chan_cfg_t chan_cfg = {
      .atten = BAT_ADC_ATTEN,
      .bitwidth = BAT_ADC_BITWIDTH,
  };
  if (adc_oneshot_config_channel(s_adc, BAT_ADC_CH, &chan_cfg) != ESP_OK) {
    ESP_LOGE(TAG, "adc channel config failed");
    return false;
  }

  adc_cali_curve_fitting_config_t cali_cfg = {
      .unit_id = BAT_ADC_UNIT,
      .chan = BAT_ADC_CH,
      .atten = BAT_ADC_ATTEN,
      .bitwidth = BAT_ADC_BITWIDTH,
  };
  if (adc_cali_create_scheme_curve_fitting(&cali_cfg, &s_cali) != ESP_OK) {
    s_cali = NULL;
    ESP_LOGW(TAG, "adc calibration not supported");
  }

  s_inited = true;
  return true;
}

// Control AO3400 MOSFET for ADC connection.
// enable=true -> Low level (Connect)
// enable=false -> High-Z (Disconnect)
void battery_y_ctrl_adc(bool enable) {
  if (enable) {
    gpio_set_level(BAT_ADC_CTRL_PIN, 0);
  } else {
    // Open-drain high = High-Z
    gpio_set_level(BAT_ADC_CTRL_PIN, 1);
  }
}

// Initialize ADC channel and calibration for battery sensing.
void battery_y_init(void) {
  // Configure AO3400 control pin as Open Drain Output
  gpio_config_t io_conf = {
      .pin_bit_mask = (1ULL << BAT_ADC_CTRL_PIN),
      .mode = GPIO_MODE_OUTPUT_OD,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };
  gpio_config(&io_conf);

  // Enable ADC connection by default on init
  battery_y_ctrl_adc(true);

  (void)battery_adc_init();
}

static uint32_t clamp_u32(uint32_t v, uint32_t lo, uint32_t hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

// Read battery voltage, convert to percent, and update UI.
bool battery_y_read_update_ui(void) {
  if (!battery_adc_init()) {
    return false;
  }

  int raw = 0;
  if (adc_oneshot_read(s_adc, BAT_ADC_CH, &raw) != ESP_OK) {
    ESP_LOGW(TAG, "adc read failed");
    return false;
  }

  int mv = 0;
  if (s_cali) {
    if (adc_cali_raw_to_voltage(s_cali, raw, &mv) != ESP_OK) {
      mv = 0;
    }
  }

  // If calibration unavailable, estimate using raw range (12-bit).
  if (mv == 0) {
    mv = (raw * 3300) / 4095;
  }

  float vbat_mv = (float)mv * (BAT_DIV_NUM / BAT_DIV_DEN);
  uint32_t vbat = (uint32_t)(vbat_mv + 0.5f);

  uint32_t v_clamped = clamp_u32(vbat, BAT_VOLT_MIN_MV, BAT_VOLT_MAX_MV);
  uint32_t percent = (v_clamped - BAT_VOLT_MIN_MV) * 100 /
                     (BAT_VOLT_MAX_MV - BAT_VOLT_MIN_MV);

  lvgl_ui_set_battery(vbat, percent);
  ESP_LOGI(TAG, "battery raw=%d mv=%d vbat=%u pct=%u", raw, mv, vbat,
           (unsigned)percent);
  return true;
}
