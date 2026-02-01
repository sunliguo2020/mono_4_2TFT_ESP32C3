idf_component_register(SRCS "ST7306_4p2_BW_DisplayDriver.c"
                            "esp_lcd_panel_st7306_bw.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver esp_lcd board_mono_tft)
