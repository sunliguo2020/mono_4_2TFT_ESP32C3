idf_component_register(
    SRCS "ble_wifi.c"
    INCLUDE_DIRS "include"
    REQUIRES bt nvs_flash esp_wifi Wifi_hw_Y
)