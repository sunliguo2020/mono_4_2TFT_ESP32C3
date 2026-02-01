idf_component_register(SRCS "sleep.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_lvgl_port GUI lvgl_display Wifi_hw_Y)
