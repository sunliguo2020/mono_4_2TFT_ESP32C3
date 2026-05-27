#pragma once

#include <stdbool.h>
#include "freertos/FreeRTOS.h"

void wifi_hw_init(void);//ʼwifi
bool wifi_is_connected(void);
bool wifi_wait_connected(TickType_t ticks_to_wait);
void wifi_save_to_nvs(const char *ssid, const char *pass);
void wifi_load_from_nvs(void);

extern char MY_SSID[33];
extern char MY_PASSWORD[65];
