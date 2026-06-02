idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
REQUIRES lvgl esp_lvgl_port lvgl_display Wifi_hw_Y AHT30_Y OPT3001_Y BATTERY_Y sleep bt nvs_flash ble_wifi weather )
