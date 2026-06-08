#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_lvgl_port.h"
#include "lvgl.h"
#include "esp_log.h"
#include "esp_task_wdt.h"

#include "lvgl_display.h"
#include "Wifi_hw_Y.h"
#include "hw_time_Y.h"
#include "sleep.h"
#include "AHT30_Y.h"
#include "OPT3001_Y.h"
#include "BATTERY_Y.h"
#include "AS312_Y.h"
#include "esp_sleep.h"
#include "ble_wifi.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "weather.h"

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
#define TAG "MAIN"

// WiFi 连接超时时间（毫秒）
#define WIFI_CONNECT_TIMEOUT_MS 30000

// 系统状态枚举
typedef enum {
    STATE_INIT,                    // 初始状态
    STATE_WIFI_CONNECTING,         // WiFi 连接中（初始尝试）
    STATE_WIFI_CONNECTED,          // WiFi 已连接
    STATE_BLE_PROVISIONING,        // BLE 配网中
    STATE_PROVISIONING_DONE,       // 配网完成，正在连接 WiFi
    STATE_PROVISIONING_WIFI_FAIL,  // 配网后 WiFi 连接失败
} system_state_t;

static system_state_t s_state = STATE_INIT;
static TickType_t s_state_start_tick = 0;

// 配网完成标志（用于从 BLE 回调通知主任务）
// 使用 volatile 标志位而非队列，避免在 NimBLE 任务中操作队列导致死锁
static volatile bool s_provisioning_done_flag = false;

static void handle_io5_wakeup(void) {
    sleep_notify_io5_wake();
}

/**
 * @brief 配网完成回调：BLE 收到 WiFi 凭据后调用
 * 
 * 这个回调在 NimBLE 任务中执行，不能直接调用 WiFi 相关函数。
 * 设置标志位，由主循环轮询检查。
 */
static void on_provisioning_done(void)
{
    ESP_LOGI(TAG, "Provisioning done, setting flag for main task...");
    s_provisioning_done_flag = true;
}

/**
 * @brief 进入 BLE 配网状态
 * 关闭 WiFi 驱动，启动 BLE
 */
static void enter_ble_provisioning(void)
{
    ESP_LOGI(TAG, "Entering BLE provisioning mode...");

    // 关闭 WiFi 底层驱动，释放射频资源给 BLE 使用
    esp_wifi_stop();
    esp_wifi_deinit();

    // 注册配网完成回调
    ble_wifi_register_done_cb(on_provisioning_done);

    esp_err_t ret = ble_wifi_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "BLE init failed: %s", esp_err_to_name(ret));
    }

    s_state = STATE_BLE_PROVISIONING;
    s_state_start_tick = xTaskGetTickCount();
}

/**
 * @brief 进入 WiFi 连接状态
 */
static void enter_wifi_connecting(void)
{
    ESP_LOGI(TAG, "Entering WiFi connecting mode...");
    wifi_hw_init();
    s_state = STATE_WIFI_CONNECTING;
    s_state_start_tick = xTaskGetTickCount();
}

/**
 * @brief WiFi 连接成功后的初始化
 * 注意：此函数在状态机中调用，不能阻塞！
 * 时间同步在后台任务中进行，不阻塞主循环
 */
static void on_wifi_connected(void)
{
    ESP_LOGI(TAG, "WiFi connected! Performing post-connection init...");
    s_state = STATE_WIFI_CONNECTED;

    // 设置默认时间（非阻塞）
    hw_time_set_default_if_unset();
    // 启动时间同步（在后台任务中进行，不阻塞）
    hw_time_sync_start();

    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    uint64_t gpio_wakeup = esp_sleep_get_gpio_wakeup_status();
    if (gpio_wakeup & (1ULL << WAKE_BTN_GPIO)) {
        handle_io5_wakeup();
    }

    // 天气请求由 app_main 在显示初始化前完成，这里不再重复调用
}

/**
 * @brief 状态机主循环处理函数
 * 每次主循环迭代时调用，非阻塞地处理状态转换
 */
static void state_machine_run(void)
{
    switch (s_state) {
        case STATE_INIT: {
            // 初始状态：尝试 WiFi 连接
            wifi_load_from_nvs();
            bool has_wifi_creds = (strlen(MY_SSID) > 0 && strlen(MY_PASSWORD) > 0);

            if (has_wifi_creds) {
                ESP_LOGI(TAG, "WiFi credentials found, attempting to connect...");
                enter_wifi_connecting();
            } else {
                ESP_LOGI(TAG, "No WiFi credentials, starting BLE provisioning...");
                enter_ble_provisioning();
            }
            break;
        }

        case STATE_WIFI_CONNECTING: {
            // WiFi 连接中，检查是否已连接或超时
            if (wifi_is_connected()) {
                on_wifi_connected();
                return;
            }

            TickType_t elapsed = xTaskGetTickCount() - s_state_start_tick;
            if (elapsed >= pdMS_TO_TICKS(WIFI_CONNECT_TIMEOUT_MS)) {
                ESP_LOGW(TAG, "WiFi connection timeout after %d ms", WIFI_CONNECT_TIMEOUT_MS);
                enter_ble_provisioning();
            }
            break;
        }

        case STATE_WIFI_CONNECTED: {
            // WiFi 已连接，无需额外操作
            break;
        }

        case STATE_BLE_PROVISIONING: {
            // BLE 配网中，检查配网完成标志
            if (s_provisioning_done_flag) {
                s_provisioning_done_flag = false;
                ESP_LOGI(TAG, "Provisioning done, connecting WiFi...");
                // 关闭 BLE
                ble_wifi_deinit();
                // 重新加载 WiFi 凭据
                wifi_load_from_nvs();
                // 进入 WiFi 连接状态
                enter_wifi_connecting();
                s_state = STATE_PROVISIONING_DONE;
                s_state_start_tick = xTaskGetTickCount();
            }
            break;
        }

        case STATE_PROVISIONING_DONE: {
            // 配网完成，正在连接 WiFi
            if (wifi_is_connected()) {
                on_wifi_connected();
                return;
            }

            TickType_t elapsed = xTaskGetTickCount() - s_state_start_tick;
            if (elapsed >= pdMS_TO_TICKS(WIFI_CONNECT_TIMEOUT_MS)) {
                ESP_LOGW(TAG, "WiFi connection failed after provisioning");
                s_state = STATE_PROVISIONING_WIFI_FAIL;
            }
            break;
        }

        case STATE_PROVISIONING_WIFI_FAIL: {
            // 配网后 WiFi 连接失败，重新开启 BLE
            ESP_LOGW(TAG, "WiFi failed after provisioning, restarting BLE...");
            enter_ble_provisioning();
            break;
        }

        default:
            break;
    }
}

void app_main(void)
{
    esp_err_t ret;

    // 禁用任务看门狗，避免 LVGL 渲染时触发看门狗超时
    // LVGL 任务在渲染大屏幕时可能占用 CPU 超过默认的 5 秒超时
    esp_task_wdt_deinit();

    // =========================
    // NVS INIT
    // =========================
    ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {

        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    uint64_t gpio_wakeup = esp_sleep_get_gpio_wakeup_status();
    ESP_LOGI("main", "wakeup cause=%d gpio_mask=0x%08x%08x",
             (int)esp_sleep_get_wakeup_cause(), (uint32_t)(gpio_wakeup >> 32),
             (uint32_t)gpio_wakeup);

    // Install GPIO ISR service once for IO5 wake and AS312 motion.
    esp_err_t isr_ret = gpio_install_isr_service(0);
    if (isr_ret != ESP_OK && isr_ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGW("main", "gpio ISR service init failed: 0x%x",
                 (unsigned)isr_ret);
    }

    // 先初始化传感器和 Wi-Fi，暂不启动屏幕
    aht30_y_init();
    aht30_y_start();
    opt3001_y_init();
    battery_y_init();
    as312_y_init(AS312_GPIO);
    opt3001_y_config_interrupt(OPT3001_TEST_LOW_LUX, OPT3001_TEST_HIGH_LUX,
                               true);

    // Wi-Fi 连接状态机（会阻塞等待连接）
    while (s_state != STATE_WIFI_CONNECTED) {
        state_machine_run();
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // 等待时间同步完成（TLS 证书验证需要正确的时间）
    ESP_LOGI(TAG, "waiting for time sync before weather...");
    hw_time_wait_for_sync(60000);

    // 获取天气（在独立高栈任务中执行，避免 main 栈溢出）
    weather_poll_once_blocking();

    // ========== 初始化 LVGL 显示 ==========
    lv_display_t *disp = lvgl_display_init();
    if (!disp) {
        abort();
    }

    lvgl_port_lock(portMAX_DELAY);
    lvgl_gui_init();
    lvgl_ui_show_sleep_screen(false);
    lvgl_port_unlock();
    lvgl_ui_wait_idle(500);

    // 把已缓存的传感器数据推送到 UI
    aht30_y_read_update_ui();
    opt3001_y_read_update_ui();
    battery_y_read_update_ui();

    // 屏幕显示完毕后再启动 sleep 任务
    sleep_start_light();

#if OPT3001_DEEPSLEEP_TEST
    opt3001_y_run_deepsleep_test(OPT3001_DEEPSLEEP_WAKE_LOW,
                                 OPT3001_DEEPSLEEP_WAKE_SEC);
#endif

#if OPT3001_INT_TEST
    opt3001_y_start_int_test(OPT3001_INT_TEST_PERIOD_MS,
                             OPT3001_INT_TEST_LUX_POLL_MS);
    return;
#endif

    // 温湿度定时读取间隔
    TickType_t last_temp_humi_tick = xTaskGetTickCount();
#define TEMP_HUMI_INTERVAL_MS 3000  // 每 3 秒读取一次

    // 主循环：非阻塞地处理状态机
    while (1) {
        // 运行状态机（非阻塞）
        state_machine_run();

        // 定时读取温湿度传感器并更新 UI
        TickType_t now = xTaskGetTickCount();
        if ((now - last_temp_humi_tick) >= pdMS_TO_TICKS(TEMP_HUMI_INTERVAL_MS)) {
            aht30_y_read_update_ui();
            opt3001_y_read_update_ui();
            last_temp_humi_tick = now;
        }

        // 短暂延时，让其他任务（如 LVGL）有机会运行
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
