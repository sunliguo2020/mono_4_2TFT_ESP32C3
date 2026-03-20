#include "sleep.h"

#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_lvgl_port.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "gui_guider.h"
#include "lvgl_display.h"
#include "Wifi_hw_Y.h"
#include "pc_status.h"
#include "hw_time_Y.h"
#include "MONO_TFT.h"
#include "lvgl.h"
#include "AHT30_Y.h"
#include "OPT3001_Y.h"
#include "BATTERY_Y.h"
#include "AS312_Y.h"
#include <time.h>

#define TAG "sleep"
#define SLEEP_WAKEUP_SEC 60
#define HTTP_UPDATE_INTERVAL_SEC 60
#define DISPLAY_KEEP_ON (CS_PIN < 0)
#define DISPLAY_USE_PANEL_SLEEP (!DISPLAY_KEEP_ON)
#define DISPLAY_SLEEP_DELAY_MS 80
#define AHT30_TASK_STACK 3072
#define AHT30_TASK_WAIT_MS 2000
#define OPT3001_SLEEP_LOW_LUX 5.0f
#define OPT3001_WAKE_HIGH_LUX 20.0f
#define OPT3001_INT_ACTIVE_LOW 1
#define OPT3001_USE_DEEP_SLEEP 1
#define OPT3001_DISARM_HIGH_LUX 100000.0f
#define WAKE_BTN_GPIO 5
#define AS312_NO_MOTION_TIMEOUT_US (30LL * 60LL * 1000000LL)
#define POWER_ON_RETRY_MS 5000
#define POWER_ON_LOOP_DELAY_MS 300
#define POWER_ON_WIFI_WAIT_MS 15000

static void force_screen_switch(bool show_sleep);
static TaskHandle_t s_sleep_task = NULL;
static sleep_update_cb_t s_http_update_cb = NULL;
static sleep_update_cb_t s_battery_update_cb = NULL;
static sleep_update_cb_t s_state_update_cb = NULL;
static int64_t s_last_http_update_us = 0;
static volatile bool s_power_on_pending = false;
static int64_t s_last_power_on_attempt_us = 0;
static int s_io5_last_level = 1;
static volatile bool s_io5_irq_flag = false;

static void IRAM_ATTR io5_isr_handler(void *arg) {
  (void)arg;
  s_io5_irq_flag = true;
}

// Return true when lux is below the night threshold.
static bool is_low_lux(bool has_lux, float lux) {
  return has_lux && (lux < OPT3001_SLEEP_LOW_LUX);
}

// Arm OPT3001 interrupt so a high-lux transition wakes the device.
static bool opt3001_arm_wake(float wake_lux, bool active_low) {
  gpio_num_t int_gpio = opt3001_y_get_int_gpio();
  int inactive_level = active_low ? 1 : 0;

  // Disarm first: set a very high limit so INT should be inactive.
  opt3001_y_config_interrupt(0.0f, OPT3001_DISARM_HIGH_LUX, active_low);
  vTaskDelay(pdMS_TO_TICKS(OPT3001_CONV_DELAY_MS));
  int level = gpio_get_level(int_gpio);
  ESP_LOGI(TAG, "opt3001 disarm int level=%d", level);

  // Arm wake threshold.
  opt3001_y_config_interrupt(0.0f, wake_lux, active_low);
  vTaskDelay(pdMS_TO_TICKS(OPT3001_CONV_DELAY_MS));
  level = gpio_get_level(int_gpio);
  ESP_LOGI(TAG, "opt3001 arm int level=%d (active_low=%d)", level,
           active_low ? 1 : 0);

  return level == inactive_level;
}

// Hold display GPIO levels during sleep to avoid spurious resets.
static void display_gpio_hold(bool enable) {
  // Hold display-related GPIO levels during sleep to avoid spurious writes.
  const int pins[] = {RST_PIN, CS_PIN, DC_PIN, SCLK_PIN, SDI_PIN};

  if (enable) {
    if (RST_PIN >= 0) {
      // 1. Set output level to 1 FIRST (writes to GPIO_OUT_W1TS_REG)
      gpio_set_level(RST_PIN, 1);

      // 2. Configure RST as Open Drain output with pull-up
      gpio_config_t rst_conf = {
          .pin_bit_mask = (1ULL << RST_PIN),
          .mode = GPIO_MODE_OUTPUT_OD,
          .pull_up_en = GPIO_PULLUP_ENABLE,
          .pull_down_en = GPIO_PULLDOWN_DISABLE,
          .intr_type = GPIO_INTR_DISABLE,
      };
      gpio_config(&rst_conf);
      
      // 3. Ensure the level is set before holding
      vTaskDelay(pdMS_TO_TICKS(10));
      
      // Explicitly hold RST pin
      esp_err_t hold_ret = gpio_hold_en(RST_PIN);
      ESP_LOGI(TAG, "Holding RST pin %d high (ret=%d)", RST_PIN, hold_ret);
    }
    for (size_t i = 0; i < sizeof(pins) / sizeof(pins[0]); i++) {
      if (pins[i] < 0 || pins[i] == RST_PIN) { // Skip RST_PIN as handled above
        continue;
      }
      gpio_hold_en(pins[i]);
    }
    
    // Enable global deep sleep hold for digital GPIOs (Required for ESP32-C3)
    gpio_deep_sleep_hold_en();
    ESP_LOGI(TAG, "Enabled global deep sleep hold");
  } else {
    // Re-configure RST pin to output HIGH BEFORE disabling hold to prevent glitch
    if (RST_PIN >= 0) {
      // 1. Set output level to 1 FIRST
      gpio_set_level(RST_PIN, 1);

      // 2. Configure as Open Drain Output
      gpio_config_t rst_conf = {
          .pin_bit_mask = (1ULL << RST_PIN),
          .mode = GPIO_MODE_OUTPUT_OD,
          .pull_up_en = GPIO_PULLUP_ENABLE,
          .pull_down_en = GPIO_PULLDOWN_DISABLE,
          .intr_type = GPIO_INTR_DISABLE,
      };
      gpio_config(&rst_conf);
      
      // 3. Ensure level is stable
      esp_rom_delay_us(100); 
      ESP_LOGI(TAG, "Restored RST pin %d high before unhold", RST_PIN);
    }

    // Disable global deep sleep hold
    gpio_deep_sleep_hold_dis();
    ESP_LOGI(TAG, "Disabled global deep sleep hold");

    for (size_t i = 0; i < sizeof(pins) / sizeof(pins[0]); i++) {
      if (pins[i] < 0) {
        continue;
      }
      gpio_hold_dis(pins[i]);
    }
  }
}

// Ensure Wi-Fi is connected before any HTTPS operations.
static bool ensure_wifi_connected(TickType_t wait_ticks) {
  // Ensure STA is connected before network updates.
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

// Try to execute the pending power-on HTTP request.
static bool handle_power_on_pending(void) {
  if (!s_power_on_pending) {
    return false;
  }

  int64_t now_us = esp_timer_get_time();
  if (s_last_power_on_attempt_us != 0 &&
      (now_us - s_last_power_on_attempt_us) <
          (int64_t)POWER_ON_RETRY_MS * 1000LL) {
    return true;
  }
  s_last_power_on_attempt_us = now_us;

  ESP_LOGI(TAG, "io5 power on pending, attempt http");
  if (!ensure_wifi_connected(pdMS_TO_TICKS(POWER_ON_WIFI_WAIT_MS))) {
    ESP_LOGW(TAG, "power on pending, wifi connect failed");
    return true;
  }
  if (pc_status_send_power_on()) {
    ESP_LOGI(TAG, "power on request success");
    s_power_on_pending = false;
    return false;
  }

  ESP_LOGW(TAG, "power on request failed, keep awake");
  return true;
}

// Update time-related UI labels from RTC time.
static void update_time_labels(void) {
  // Read RTC time and push to UI labels.
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

// One-shot AHT30 update task (keeps UI thread responsive).
static void aht30_update_task(void *arg) {
  TaskHandle_t waiter = (TaskHandle_t)arg;
  aht30_y_read_update_ui();
  if (waiter) {
    xTaskNotifyGive(waiter);
  }
  vTaskDelete(NULL);
}

// Run AHT30 update in a short-lived task with timeout.
static void run_aht30_update_task(uint32_t timeout_ms) {
  TaskHandle_t self = xTaskGetCurrentTaskHandle();
  if (xTaskCreate(aht30_update_task, "aht30_upd", AHT30_TASK_STACK, self, 4,
                  NULL) != pdPASS) {
    ESP_LOGW(TAG, "aht30 task create failed, run inline");
    aht30_y_read_update_ui();
    return;
  }
  if (timeout_ms > 0) {
    (void)ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(timeout_ms));
  }
}

// Collect sensor readings and refresh UI before network work.
static void sleep_step_collect(struct tm *timeinfo, float *lux, bool *has_lux,
                               bool *low_lux, bool *motion_event,
                               bool *motion_recent) {
  time_t now = 0;
  time(&now);
  localtime_r(&now, timeinfo);

  update_time_labels();
  pc_status_update_runtime_ui();
  run_aht30_update_task(AHT30_TASK_WAIT_MS);
  battery_y_read_update_ui();
  opt3001_y_read_update_ui();

  *has_lux = opt3001_y_get_last_lux(lux);
  *low_lux = is_low_lux(*has_lux, *lux);
  *motion_event = as312_y_take_motion_event();
  *motion_recent = as312_y_motion_recent(AS312_NO_MOTION_TIMEOUT_US);

  lvgl_ui_set_presence(*motion_recent);
  ESP_LOGI(TAG, "lux=%.1f low_lux=%d", *lux, *low_lux ? 1 : 0);

  if (*low_lux) {
    force_screen_switch(true);
  } else {
    force_screen_switch(false);
  }
}

// Capture IO5 presses during awake time (edge and IRQ).
static void sleep_step_handle_io5_events(void) {
  if (s_io5_irq_flag) {
    s_io5_irq_flag = false;
    ESP_LOGI(TAG, "io5 irq detected, schedule power on");
    s_power_on_pending = true;
    s_last_power_on_attempt_us = 0;
  }

  int btn_level = gpio_get_level(WAKE_BTN_GPIO);
  if (btn_level == 0 && s_io5_last_level != 0) {
    ESP_LOGI(TAG, "io5 press detected, schedule power on");
    s_power_on_pending = true;
    s_last_power_on_attempt_us = 0;
  }
  s_io5_last_level = btn_level;
}

// Decide and run network updates (PC status + fans).
static void sleep_step_run_network(bool low_lux, bool motion_recent,
                                   bool motion_event, int64_t now_us) {
  bool http_due = false;
  if (s_last_http_update_us == 0 ||
      (now_us - s_last_http_update_us) >=
          ((int64_t)HTTP_UPDATE_INTERVAL_SEC * 1000000LL)) {
    http_due = true;
  }

  bool need_fans_update = (s_state_update_cb && douyin_fans_need_retry());
  bool need_http_update = (http_due && s_http_update_cb);
  bool do_battery_update = false;
  bool do_any_update =
      need_http_update || need_fans_update || do_battery_update;

  if (low_lux) {
    ESP_LOGI(TAG, "low lux, skip http updates");
    return;
  }
  if (!motion_recent) {
    ESP_LOGI(TAG, "no motion for 30 min, skip wifi/http updates");
    return;
  }
  if (!ensure_wifi_connected(pdMS_TO_TICKS(5000))) {
    ESP_LOGW(TAG, "skip updates, wifi not connected");
    return;
  }

  if (motion_event) {
    hw_time_sync_start();
    (void)hw_time_wait_for_sync(15000);
  } else {
    hw_time_resync_if_due();
  }

  if (!do_any_update) {
    ESP_LOGI(TAG, "skip http updates, next in %d sec",
             HTTP_UPDATE_INTERVAL_SEC);
    return;
  }

  lvgl_ui_pause(true);

  if (need_http_update) {
    s_http_update_cb();
    s_last_http_update_us = now_us;
    vTaskDelay(pdMS_TO_TICKS(50));
  }
  if (need_fans_update) {
    s_state_update_cb();
    vTaskDelay(pdMS_TO_TICKS(50));
  }
  if (do_battery_update && s_battery_update_cb) {
    s_battery_update_cb();
  }

  lvgl_ui_pause(false);
  vTaskDelay(pdMS_TO_TICKS(20));
}

// Update UI and fan retry flags before sleeping.
static void sleep_step_prepare_sleep(bool low_lux) {
  update_time_labels();
  lvgl_ui_set_time_update_symbol(LV_SYMBOL_LOOP);
  vTaskDelay(pdMS_TO_TICKS(30));
  bool ui_idle = lvgl_ui_wait_idle(1500);
  bool fans_updated = douyin_fans_updated_take();
  if (!ui_idle && fans_updated) {
    ESP_LOGW(TAG, "fans ui not drained, retry next wake");
    douyin_fans_force_retry();
  }

  if (low_lux) {
    force_screen_switch(true);
    lvgl_ui_wait_idle(500);
  }
}

// Configure wake sources and enter light/deep sleep.
static void sleep_step_enter_sleep(bool low_lux, float lux, int sleep_sec) {
  gpio_wakeup_enable(WAKE_BTN_GPIO, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();

#if OPT3001_USE_DEEP_SLEEP
  if (low_lux) {
    int int_level = gpio_get_level(opt3001_y_get_int_gpio());
    ESP_LOGI(TAG, "opt3001 int level=%d (active_low=%d)", int_level,
             OPT3001_INT_ACTIVE_LOW ? 1 : 0);
    bool allow_deep = (gpio_get_level(WAKE_BTN_GPIO) != 0);
    if (!allow_deep) {
      ESP_LOGW(TAG, "wake btn low, skip deep sleep");
    } else {
      bool int_ready = opt3001_arm_wake(OPT3001_WAKE_HIGH_LUX,
                                        OPT3001_INT_ACTIVE_LOW);
      if (!int_ready) {
        ESP_LOGW(TAG, "opt3001 int still active, skip deep sleep");
        allow_deep = false;
      }
    }

    if (allow_deep) {
      lvgl_display_low_power(true);
      vTaskDelay(pdMS_TO_TICKS(DISPLAY_SLEEP_DELAY_MS));
      display_gpio_hold(true);
      esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
      uint64_t mask = (1ULL << opt3001_y_get_int_gpio()) |
                      (1ULL << WAKE_BTN_GPIO);
      esp_deepsleep_gpio_wake_up_mode_t mode = ESP_GPIO_WAKEUP_GPIO_LOW;
      esp_err_t wk = esp_deep_sleep_enable_gpio_wakeup(mask, mode);
      ESP_LOGI(TAG,
               "enter deep sleep (lux=%.1f, wake_cfg=0x%x, opt_wake=low)",
               lux, (unsigned)wk);
      esp_deep_sleep_start();
    }
  }
#endif

  if (DISPLAY_USE_PANEL_SLEEP) {
    lvgl_display_sleep(true);
    vTaskDelay(pdMS_TO_TICKS(DISPLAY_SLEEP_DELAY_MS));
  } else if (DISPLAY_KEEP_ON) {
    ESP_LOGI(TAG, "display keep-on enabled (CS tied low)");
  }
  
  // Disconnect battery ADC (High-Z) before sleep
  battery_y_ctrl_adc(false);
  
  display_gpio_hold(true);

  int btn_level = gpio_get_level(WAKE_BTN_GPIO);
  if (btn_level == 0) {
    ESP_LOGW(TAG, "wake btn low, skip sleep this cycle");
    vTaskDelay(pdMS_TO_TICKS(200));
    return;
  }

  esp_err_t wk_ret = esp_sleep_enable_timer_wakeup(
      (uint64_t)sleep_sec * 1000000ULL);
  ESP_LOGI(TAG, "enter light sleep for %d sec, wake cfg ret=0x%x",
           sleep_sec, (unsigned int)wk_ret);
  esp_err_t slp_ret = esp_light_sleep_start();
  esp_sleep_wakeup_cause_t wake_cause = esp_sleep_get_wakeup_cause();
  uint64_t gpio_mask = esp_sleep_get_gpio_wakeup_status();
  ESP_LOGI(TAG, "woke from light sleep, ret=0x%x, cause=%d",
           (unsigned int)slp_ret, (int)wake_cause);
  if (wake_cause == ESP_SLEEP_WAKEUP_GPIO &&
      (gpio_mask & (1ULL << WAKE_BTN_GPIO))) {
    s_power_on_pending = true;
    s_last_power_on_attempt_us = 0;
  }

  display_gpio_hold(false);
  
  // Reconnect battery ADC (Low) after wake
  battery_y_ctrl_adc(true);

  if (DISPLAY_USE_PANEL_SLEEP) {
    lvgl_display_sleep(false);
    lvgl_display_wakeup();
  }
  lvgl_port_task_wake(LVGL_PORT_EVENT_DISPLAY, NULL);
  esp_wifi_set_ps(WIFI_PS_NONE);
  vTaskDelay(pdMS_TO_TICKS(120));

  if (!low_lux) {
    force_screen_switch(false);
  }
  update_time_labels();
  lvgl_ui_set_time_update_symbol(LV_SYMBOL_HOME);
  lvgl_ui_wait_idle(300);
}

static void sleep_task(void *arg) {
  // Main low-power loop: update UI, run network tasks, then light sleep.
  (void)arg;
  gpio_config_t wake_btn_cfg = {
      .pin_bit_mask = 1ULL << WAKE_BTN_GPIO,
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_ENABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_NEGEDGE,
  };
  gpio_config(&wake_btn_cfg);
  gpio_isr_handler_add(WAKE_BTN_GPIO, io5_isr_handler, NULL);
  s_io5_last_level = gpio_get_level(WAKE_BTN_GPIO);

  while (true) {
    ESP_LOGI(TAG, "sleep loop tick");

    struct tm timeinfo = {0};
    float lux = 0.0f;
    bool has_lux = false;
    bool low_lux = false;
    bool motion_event = false;
    bool motion_recent = false;

    sleep_step_collect(&timeinfo, &lux, &has_lux, &low_lux,
                       &motion_event, &motion_recent);
    sleep_step_handle_io5_events();
    if (handle_power_on_pending()) {
      vTaskDelay(pdMS_TO_TICKS(POWER_ON_LOOP_DELAY_MS));
      continue;
    }

    int sleep_sec = 60 - timeinfo.tm_sec;
    if (sleep_sec <= 0) {
      sleep_sec = 60;
    }

    int64_t now_us = esp_timer_get_time();
    sleep_step_run_network(low_lux, motion_recent, motion_event, now_us);
    sleep_step_prepare_sleep(low_lux);
    sleep_step_enter_sleep(low_lux, lux, sleep_sec);
  }
}

// Initialize and start the sleep task once.
void sleep_init(void) {
  // Create the sleep task once.
  if (s_sleep_task) {
    return;
  }
  xTaskCreate(sleep_task, "sleep_task", 8192, NULL, 5, &s_sleep_task);
}

// Public entry for starting light-sleep loop.
void sleep_start_light(void) {
  sleep_init();
}

// Register HTTP update callback (PC status).
void sleep_register_http_update_cb(sleep_update_cb_t cb) {
  s_http_update_cb = cb;
}

// Register battery update callback.
void sleep_register_battery_update_cb(sleep_update_cb_t cb) {
  s_battery_update_cb = cb;
}

// Register generic state update callback (fans).
void sleep_register_state_update_cb(sleep_update_cb_t cb) {
  s_state_update_cb = cb;
}

// Schedule power-on HTTP after IO5 wake.
void sleep_notify_io5_wake(void) {
  s_power_on_pending = true;
  s_last_power_on_attempt_us = 0;
}

// Force switch between main/sleep screens with LVGL locking.
static void force_screen_switch(bool show_sleep) {
  lvgl_ui_pause(true);
  lvgl_port_task_wake(LVGL_PORT_EVENT_DISPLAY, NULL);
  if (!lvgl_port_lock(800)) {
    ESP_LOGW(TAG, "lvgl lock timeout, skip screen switch");
    lvgl_ui_pause(false);
    return;
  }
  if (show_sleep) {
    if (!guider_ui.screen_sleep || guider_ui.screen_sleep_del) {
      setup_scr_screen_sleep(&guider_ui);
      guider_ui.screen_sleep_del = false;
    }
    lv_screen_load(guider_ui.screen_sleep);
  } else {
    if (!guider_ui.screen_main || guider_ui.screen_main_del) {
      setup_scr_screen_main(&guider_ui);
      guider_ui.screen_main_del = false;
    }
    lv_screen_load(guider_ui.screen_main);
  }
  lv_display_t *disp = lv_display_get_default();
  if (disp) {
    lv_refr_now(disp);
  }
  lvgl_port_unlock();
  lvgl_ui_pause(false);
}
