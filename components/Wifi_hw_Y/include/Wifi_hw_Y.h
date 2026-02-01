#pragma once

#include <stdbool.h>
#include "freertos/FreeRTOS.h"

void wifi_hw_init(void);//≥ı ºªØwifi
bool wifi_is_connected(void);
bool wifi_wait_connected(TickType_t ticks_to_wait);
