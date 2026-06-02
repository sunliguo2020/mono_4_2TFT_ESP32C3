idf_component_register(SRCS "weather.c"
                    INCLUDE_DIRS "include"
REQUIRES esp_http_client esp-tls  Wifi_hw_Y lvgl_display esp_lvgl_port
                    EMBED_TXTFILES "weather_root_cert.pem"
                    )
