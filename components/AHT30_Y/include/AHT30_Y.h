#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/i2c_master.h"

void aht30_y_init(void);
void aht30_y_start(void);
void aht30_y_read_update_ui(void);
i2c_master_bus_handle_t aht30_y_get_bus(void);

#ifdef __cplusplus
}
#endif
