/**
 * @file BATTERY_Y.c
 * @brief 电池电压 ADC 采集与电量计算
 * 
 * 硬件连接：
 *   - ADC 引脚: GPIO3 → ADC1_CH3
 *   - 分压电路: 100k（电池侧）+ 200k（GND侧）
 *     Vadc = Vbat * 200 / (100 + 200) = Vbat * 2/3
 *     Vbat = Vadc * 300 / 200 = Vadc * 3 / 2
 *   - MOSFET 控制: GPIO7（AO3400，控制 ADC 通断）
 *     睡眠时断开 MOSFET 以省电，唤醒后重新导通
 * 
 * 电量计算：
 *   - 3300mV = 0%（电池最低电压）
 *   - 4200mV = 100%（电池满电电压）
 *   - 百分比 = (Vbat - 3300) * 100 / (4200 - 3300)
 */

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
#define BAT_ADC_ATTEN ADC_ATTEN_DB_12
#define BAT_ADC_BITWIDTH ADC_BITWIDTH_DEFAULT

// Divider: 100k (to battery) + 200k (to GND)
// Vadc = Vbat * 200 / (100 + 200) = Vbat * 2/3
// Vbat = Vadc * 300 / 200 = Vadc * 3 / 2
#define BAT_DIV_NUM 300.0f
#define BAT_DIV_DEN 200.0f

#define BAT_VOLT_MIN_MV 3300
#define BAT_VOLT_MAX_MV 4200

static adc_oneshot_unit_handle_t s_adc = NULL;
static adc_cali_handle_t s_cali = NULL;
static bool s_inited = false;

/**
 * @brief 初始化 ADC 单元和校准
 * 
 * 配置 ADC1_CH3（GPIO3）为 12 位分辨率，0~3300mV 输入范围
 * 创建校准曲线用于将 ADC 原始值转换为电压
 */
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

/**
 * @brief 控制 MOSFET 导通/关断，连接/断开 ADC
 * 
 * AO3400 是 P-MOSFET，Gate 由 GPIO7 控制：
 *   - enable=true  → GPIO7 = Low  → MOSFET 导通，ADC 连接到电池
 *   - enable=false → GPIO7 = High → MOSFET 关断，ADC 断开（省电）
 * 
 * 睡眠前调用 battery_y_ctrl_adc(false) 断开 ADC 以省电
 * 唤醒后调用 battery_y_ctrl_adc(true) 重新连接 ADC
 */
void battery_y_ctrl_adc(bool enable) {
  if (enable) {
    gpio_set_level(BAT_ADC_CTRL_PIN, 0);
  } else {
    // Open-drain high = High-Z
    gpio_set_level(BAT_ADC_CTRL_PIN, 1);
  }
}

/**
 * @brief 初始化电池 ADC 采集
 * 
 * 1. 配置 GPIO7 为开漏输出，控制 MOSFET
 * 2. 默认导通 MOSFET，连接 ADC
 * 3. 初始化 ADC 单元和校准
 */
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

/**
 * @brief 读取电池电压并更新 UI
 * 
 * 流程：
 *   1. 读取 ADC 原始值（0~4095，12位）
 *   2. 通过校准曲线转换为电压（mV）
 *   3. 根据分压比计算实际电池电压：
 *      Vbat = Vadc * 300 / 200 = Vadc * 3 / 2
 *   4. 将电压限制在 3300~4200mV 范围内
 *   5. 计算百分比：
 *      3300mV = 0%, 4200mV = 100%
 *      percent = (Vbat - 3300) * 100 / (4200 - 3300)
 *   6. 更新 UI 显示
 * 
 * @return true 读取成功，false 读取失败
 */
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
