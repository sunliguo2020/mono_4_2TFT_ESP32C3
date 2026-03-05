#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "driver/gpio.h"

#define OPT3001_CONV_DELAY_MS 120

#ifdef __cplusplus
extern "C" {
#endif

void opt3001_y_init(void);
// Read OPT3001 lux and update UI label.
void opt3001_y_read_update_ui(void);
// Configure OPT3001 interrupt thresholds and polarity.
void opt3001_y_config_interrupt(float low_lux, float high_lux,
                                bool active_low);
// Fetch last cached lux reading.
bool opt3001_y_get_last_lux(float *lux);
// Return the INT pin assigned to OPT3001.
gpio_num_t opt3001_y_get_int_gpio(void);
// Start a periodic INT debug log task.
void opt3001_y_start_int_test(uint32_t period_ms, uint32_t lux_poll_ms);
// Run a deep-sleep wake test using the OPT3001 INT pin.
void opt3001_y_run_deepsleep_test(bool wake_low, int awake_seconds);

#ifdef __cplusplus
}
#endif
