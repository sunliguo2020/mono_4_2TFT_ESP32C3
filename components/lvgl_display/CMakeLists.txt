idf_component_register(SRCS "lvgl_display.c" "lvgl_ui.c"
                    INCLUDE_DIRS "include"
                    REQUIRES lvgl esp_lvgl_port esp_lcd driver board_mono_tft display_st7306_bw GUI)
