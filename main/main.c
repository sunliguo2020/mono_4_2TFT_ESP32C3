#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_lvgl_port.h"
#include "lvgl.h"
#include "esp_log.h"

#include "lvgl_display.h"
#include "Wifi_hw_Y.h"
#include "hw_time_Y.h"
#include "pc_status.h"
#include "sleep.h"
#include "AHT30_Y.h"
#include "OPT3001_Y.h"
#include "BATTERY_Y.h"
#include "AS312_Y.h"
#include "esp_sleep.h"
#define WAKE_BTN_GPIO 5
#define OPT3001_INT_TEST 0
#define OPT3001_INT_TEST_PERIOD_MS 200
#define OPT3001_INT_TEST_LUX_POLL_MS 1000
#define OPT3001_TEST_LOW_LUX 5.0f
#define OPT3001_TEST_HIGH_LUX 20.0f
#define OPT3001_DEEPSLEEP_TEST 0
#define OPT3001_DEEPSLEEP_WAKE_LOW 1
#define OPT3001_DEEPSLEEP_WAKE_SEC 5
#define AS312_GPIO GPIO_NUM_2

static void handle_io5_wakeup(void) {
    sleep_notify_io5_wake();
}

void app_main(void)
{
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    uint64_t gpio_wakeup = esp_sleep_get_gpio_wakeup_status();
    ESP_LOGI("main", "wakeup cause=%d gpio_mask=0x%08x%08x",
             (int)cause, (uint32_t)(gpio_wakeup >> 32),
             (uint32_t)gpio_wakeup);

    lv_display_t *disp = lvgl_display_init();
    if (!disp) {
        abort();
    }

    lvgl_port_lock(0);
    lvgl_gui_init();
    lvgl_port_unlock();
    lvgl_ui_show_sleep_screen(false);
    lvgl_ui_wait_idle(500);

    // Install GPIO ISR service once for IO5 wake and AS312 motion.
    esp_err_t isr_ret = gpio_install_isr_service(0);
    if (isr_ret != ESP_OK && isr_ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGW("main", "gpio ISR service init failed: 0x%x",
                 (unsigned)isr_ret);
    }

    aht30_y_init();
    aht30_y_start();
    aht30_y_read_update_ui();
    opt3001_y_init();
    opt3001_y_read_update_ui();
    battery_y_init();
    battery_y_read_update_ui();
    as312_y_init(AS312_GPIO);
    opt3001_y_config_interrupt(OPT3001_TEST_LOW_LUX, OPT3001_TEST_HIGH_LUX,
                               true);

#if OPT3001_DEEPSLEEP_TEST
    opt3001_y_run_deepsleep_test(OPT3001_DEEPSLEEP_WAKE_LOW,
                                 OPT3001_DEEPSLEEP_WAKE_SEC);
#endif

#if OPT3001_INT_TEST
    opt3001_y_start_int_test(OPT3001_INT_TEST_PERIOD_MS,
                             OPT3001_INT_TEST_LUX_POLL_MS);
    return;
#endif

    wifi_hw_init();
    if (wifi_wait_connected(portMAX_DELAY)) {
        hw_time_set_default_if_unset();
        hw_time_sync_start();
        (void)hw_time_wait_for_sync(30000);

        if (gpio_wakeup & (1ULL << WAKE_BTN_GPIO)) {
            handle_io5_wakeup();
        }

        sleep_register_http_update_cb(pc_status_poll_once);
        sleep_register_state_update_cb(douyin_fans_poll_once);
        sleep_start_light();
    }
}
