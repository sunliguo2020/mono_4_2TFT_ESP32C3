idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES lvgl esp_lvgl_port lvgl_display Wifi_hw_Y sleep pc_status)
