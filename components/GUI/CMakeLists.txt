
file(GLOB_RECURSE srcs *.c

                    )

set(include_dirs 
                    . 
                    custom 
                    generated 
                    generated/guider_customer_fonts
                    generated/guider_fonts           
                    generated/images          
                   
                    )               
        
idf_component_register(
                    SRCS ${srcs}       
                    INCLUDE_DIRS   ${include_dirs}          
                    REQUIRES  esp_lcd  esp_wifi driver lvgl 
                    )
add_compile_definitions(LV_LVGL_H_INCLUDE_SIMPLE)
