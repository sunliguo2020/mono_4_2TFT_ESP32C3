idf_component_register(SRCS "sleep.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_lvgl_port GUI lvgl_display Wifi_hw_Y esp_timer pc_status AHT30_Y OPT3001_Y BATTERY_Y AS312_Y weather)
