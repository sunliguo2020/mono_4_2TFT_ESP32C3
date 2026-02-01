#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_lvgl_port.h"
#include "lvgl.h"

#include "lvgl_display.h"
#include "Wifi_hw_Y.h"
#include "hw_time_Y.h"
#include "pc_status.h"
#include "sleep.h"

void app_main(void)
{
    lv_display_t *disp = lvgl_display_init();
    if (!disp) {
        abort();
    }

    lvgl_port_lock(0);
    lvgl_gui_init();
    lvgl_port_unlock();

    wifi_hw_init();
    if (wifi_wait_connected(portMAX_DELAY)) {
        hw_time_set_default_if_unset();
        hw_time_sync_start();
        (void)hw_time_wait_for_sync(30000);

        sleep_register_http_update_cb(pc_status_poll_once);
        sleep_start_light();
    }
}
