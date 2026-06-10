file(GLOB_RECURSE srcs *.c)

set(include_dirs 
    include
)

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS ${include_dirs}
    REQUIRES Wifi_hw_Y weather AHT30_Y OPT3001_Y BATTERY_Y
)