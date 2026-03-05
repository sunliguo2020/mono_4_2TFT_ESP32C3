#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize ADC for battery measurement.
void battery_y_init(void);

// Control AO3400 MOSFET for ADC connection.
void battery_y_ctrl_adc(bool enable);

// Read battery, update UI labels and icon.
bool battery_y_read_update_ui(void);

#ifdef __cplusplus
}
#endif
