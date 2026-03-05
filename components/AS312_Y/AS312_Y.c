#include "AS312_Y.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

#define TAG "AS312_Y"

static gpio_num_t s_gpio = GPIO_NUM_NC;
static volatile bool s_motion_event = false;
static int64_t s_last_motion_us = 0;
static portMUX_TYPE s_lock = portMUX_INITIALIZER_UNLOCKED;

static void IRAM_ATTR as312_isr_handler(void *arg) {
  (void)arg;
  portENTER_CRITICAL_ISR(&s_lock);
  s_motion_event = true;
  portEXIT_CRITICAL_ISR(&s_lock);
}

// Initialize AS312 PIR input and IRQ on rising edge.
void as312_y_init(gpio_num_t gpio) {
  if (s_gpio == gpio) {
    return;
  }

  s_gpio = gpio;
  s_last_motion_us = esp_timer_get_time();

  gpio_config_t cfg = {
      .pin_bit_mask = 1ULL << s_gpio,
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_POSEDGE,
  };
  gpio_config(&cfg);

  esp_err_t err = gpio_isr_handler_add(s_gpio, as312_isr_handler, NULL);
  if (err != ESP_OK) {
    ESP_LOGW(TAG, "gpio_isr_handler_add failed: %s", esp_err_to_name(err));
  }
}

// Read the current motion level (high = motion).
bool as312_y_get_motion(void) {
  if (s_gpio == GPIO_NUM_NC) {
    return false;
  }
  return gpio_get_level(s_gpio) != 0;
}

// Return and clear the latched motion event; refresh last-motion time.
bool as312_y_take_motion_event(void) {
  bool event = false;
  portENTER_CRITICAL(&s_lock);
  event = s_motion_event;
  s_motion_event = false;
  portEXIT_CRITICAL(&s_lock);

  if (event || as312_y_get_motion()) {
    s_last_motion_us = esp_timer_get_time();
    return true;
  }
  return false;
}

// Return timestamp of the last detected motion.
int64_t as312_y_get_last_motion_us(void) {
  return s_last_motion_us;
}

// Check if motion happened within the given timeout.
bool as312_y_motion_recent(int64_t timeout_us) {
  if (timeout_us <= 0) {
    return true;
  }
  int64_t last = s_last_motion_us;
  if (last <= 0) {
    return false;
  }
  int64_t now = esp_timer_get_time();
  return (now - last) <= timeout_us;
}

// Return the GPIO configured for the AS312 sensor.
gpio_num_t as312_y_get_gpio(void) {
  return s_gpio;
}
