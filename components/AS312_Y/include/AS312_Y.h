#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialize AS312 PIR input (active high on motion).
void as312_y_init(gpio_num_t gpio);
// Current motion level (true = motion/high).
bool as312_y_get_motion(void);
// Returns true if a new motion event occurred since last call.
bool as312_y_take_motion_event(void);
// Last motion timestamp (us, esp_timer timebase).
int64_t as312_y_get_last_motion_us(void);
// Whether motion happened within the given timeout (us).
bool as312_y_motion_recent(int64_t timeout_us);
// Get configured GPIO (or GPIO_NUM_NC).
gpio_num_t as312_y_get_gpio(void);

#ifdef __cplusplus
}
#endif
