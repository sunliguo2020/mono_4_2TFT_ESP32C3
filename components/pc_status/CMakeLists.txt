idf_component_register(SRCS "pc_status.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_http_client esp-tls lvgl_display esp_lvgl_port Wifi_hw_Y
                    EMBED_TXTFILES "howsmyssl_com_root_cert.pem"
                    )
