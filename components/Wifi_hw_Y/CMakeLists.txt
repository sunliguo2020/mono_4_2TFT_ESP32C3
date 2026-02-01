idf_component_register(SRCS "hw_time_Y.c" "Wifi_hw_Y.c"
                    INCLUDE_DIRS "include"
                    REQUIRES nvs_flash esp_event esp_netif esp_wifi lwip lvgl_display)
