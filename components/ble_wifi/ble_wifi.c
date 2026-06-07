#include "ble_wifi.h"
#include "Wifi_hw_Y.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_bt.h"

#include <string.h>
#include <stdlib.h>

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"

#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "host/ble_hs_mbuf.h"

#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#define TAG "BLE_WIFI"

#define BLE_DEVICE_NAME "ESP32-C3-WiFi"

#define BLE_SERVICE_UUID        0xFFE0
#define BLE_CHAR_WRITE_UUID     0xFFE1

static uint16_t s_char_handle = 0;

// 配网完成回调
static ble_provisioning_done_cb_t s_done_cb = NULL;
// BLE 运行状态
static bool s_ble_running = false;

// 累积接收缓冲区（用于处理 BLE 数据分片）
// BLE MTU 可能较小，数据可能分多次写入特征值
#define BLE_RX_BUF_SIZE 256
static char s_rx_buf[BLE_RX_BUF_SIZE] = {0};
static int s_rx_buf_len = 0;
static TickType_t s_last_rx_time = 0;  // 上次收到 BLE 数据的时间戳

void ble_wifi_register_done_cb(ble_provisioning_done_cb_t cb)
{
    s_done_cb = cb;
}

bool ble_wifi_is_running(void)
{
    return s_ble_running;
}

/**
 * @brief 解析并处理累积缓冲区中的 WiFi 凭据
 * 
 * 从缓冲区中查找 "ssid:... ,pwd:..." 格式的数据并解析
 * 数据必须以 '\n' 结尾才进行解析，确保接收完整
 * @return true 成功解析并保存了 WiFi 凭据
 */
static bool try_parse_wifi_creds(void)
{
    // 打印缓冲区原始字节（十六进制），用于调试
    ESP_LOGI(TAG, "Parsing buffer(%d), hex:", s_rx_buf_len);
    for (int i = 0; i < s_rx_buf_len && i < 64; i++) {
        printf("%02x ", (unsigned char)s_rx_buf[i]);
    }
    printf("\n");

    TickType_t now = xTaskGetTickCount();

    // 检查是否有结束符 '\n' (0x0A) 或 '\r\n' (0x0D 0x0A)
    char *end_marker = NULL;
    for (int i = 0; i < s_rx_buf_len; i++) {
        if (s_rx_buf[i] == '\n' || s_rx_buf[i] == '\r') {
            end_marker = &s_rx_buf[i];
            break;
        }
    }

    if (!end_marker) {
        // 没有结束符，但检查是否已经包含完整的 ssid:...,pwd:... 格式
        // 如果数据看起来完整（有 ssid: 和 ,pwd: 且密码不为空），也尝试解析
        // 但需确保距离上次接收到数据超过 300ms，避免 BLE 分片导致截断
        char *ssid_test = strstr(s_rx_buf, "ssid:");
        char *pwd_test = strstr(s_rx_buf, ",pwd:");
        if (ssid_test && pwd_test && pwd_test > ssid_test) {
            char *pwd_val = pwd_test + 5;
            if (strlen(pwd_val) > 0 &&
                (now - s_last_rx_time) >= pdMS_TO_TICKS(300)) {
                ESP_LOGI(TAG, "No terminator but data looks complete, trying to parse anyway");
                // 继续解析，不截断
            } else {
                ESP_LOGI(TAG, "No terminator found, buffer(%d): %s", s_rx_buf_len, s_rx_buf);
                return false;
            }
        } else {
            ESP_LOGI(TAG, "No terminator found, buffer(%d): %s", s_rx_buf_len, s_rx_buf);
            return false;
        }
    } else {
        // 找到结束符，截断
        *end_marker = 0;
        ESP_LOGI(TAG, "Found terminator, parsing buffer: %s", s_rx_buf);
    }

    char *ssid_start = strstr(s_rx_buf, "ssid:");
    char *pwd_marker = strstr(s_rx_buf, ",pwd:");

    if (!ssid_start || !pwd_marker) {
        ESP_LOGW(TAG, "Invalid format (no ssid:/,pwd: found), resetting buffer");
        // 清空缓冲区，重新开始接收
        s_rx_buf_len = 0;
        s_rx_buf[0] = 0;
        return false;
    }

    // 确保 pwd_marker 在 ssid_start 之后
    if (pwd_marker < ssid_start) {
        ESP_LOGW(TAG, "Invalid format (pwd before ssid), resetting buffer");
        s_rx_buf_len = 0;
        s_rx_buf[0] = 0;
        return false;
    }

    char *pwd_start = pwd_marker + 5; // 跳过 ",pwd:"

    // 计算 ssid 长度（ssid: 之后到 ,pwd: 之前）
    size_t ssid_len = pwd_marker - (ssid_start + 5);
    if (ssid_len > 32) ssid_len = 32;

    // 计算 pwd 长度（pwd: 之后到结束符之前）
    size_t pwd_len = strlen(pwd_start);
    if (pwd_len > 64) pwd_len = 64;

    char ssid[33] = {0};
    char pwd[65]  = {0};

    strncpy(ssid, ssid_start + 5, ssid_len);
    strncpy(pwd, pwd_start, pwd_len);

    // 去除密码末尾的字面 "\n"（两个字符：反斜杠 + n）
    // 手机发送的数据末尾可能带有字面的 "\n" 而不是换行符 0x0A
    size_t pwd_actual_len = strlen(pwd);
    if (pwd_actual_len >= 2 && pwd[pwd_actual_len - 2] == '\\' && pwd[pwd_actual_len - 1] == 'n') {
        pwd[pwd_actual_len - 2] = '\0';
        ESP_LOGI(TAG, "Stripped trailing \\n from password");
    }

    ESP_LOGI(TAG, "WiFi Save: '%s' / '%s'", ssid, pwd);

    if (strlen(ssid) == 0) {
        ESP_LOGW(TAG, "SSID is empty, ignoring");
        // 清空缓冲区，重新开始接收
        s_rx_buf_len = 0;
        s_rx_buf[0] = 0;
        return false;
    }

    // 保存 WiFi 凭据到 NVS（多组凭据，不覆盖已有）
    wifi_save_cred_to_nvs(ssid, pwd);

    ESP_LOGI(TAG, "WiFi credentials saved via BLE (multi-cred support)");

    // 清空缓冲区，准备下一次配网
    s_rx_buf_len = 0;
    s_rx_buf[0] = 0;

    return true;
}

static int ble_char_write(uint16_t conn_handle,
                          uint16_t attr_handle,
                          struct ble_gatt_access_ctxt *ctxt,
                          void *arg)
{
    char buf[129] = {0};

    uint16_t len = OS_MBUF_PKTLEN(ctxt->om);

    if (len > 128) {
        len = 128;
    }

    ble_hs_mbuf_to_flat(ctxt->om, buf, len, NULL);

    buf[len] = 0;

    ESP_LOGI(TAG, "BLE RECV (%d bytes): %s", len, buf);

    // 将新接收的数据追加到累积缓冲区
    if (s_rx_buf_len + len < BLE_RX_BUF_SIZE) {
        memcpy(s_rx_buf + s_rx_buf_len, buf, len);
        s_rx_buf_len += len;
        s_rx_buf[s_rx_buf_len] = 0;
    } else {
        // 缓冲区溢出，清空重新开始
        ESP_LOGW(TAG, "RX buffer overflow, resetting");
        s_rx_buf_len = 0;
        s_rx_buf[0] = 0;
        memcpy(s_rx_buf, buf, len);
        s_rx_buf_len = len;
        s_rx_buf[s_rx_buf_len] = 0;
    }

    ESP_LOGI(TAG, "Accumulated buffer: %s", s_rx_buf);

    // 更新最后接收数据的时间戳，用于分片超时判断
    s_last_rx_time = xTaskGetTickCount();

    // 尝试解析 WiFi 凭据
    if (try_parse_wifi_creds()) {
        // 解析成功，调用配网完成回调
        if (s_done_cb) {
            s_done_cb();
        } else {
            // 没有回调时，保持向后兼容：重启设备
            ESP_LOGI(TAG, "No callback registered, restarting...");
            vTaskDelay(pdMS_TO_TICKS(100));
            esp_bt_controller_deinit();
            vTaskDelay(pdMS_TO_TICKS(50));
            esp_restart();
        }
    }

    return 0;
}

static const struct ble_gatt_svc_def gatt_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(BLE_SERVICE_UUID),

        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = BLE_UUID16_DECLARE(BLE_CHAR_WRITE_UUID),

                .access_cb = ble_char_write,

                .flags = BLE_GATT_CHR_F_WRITE,

                .val_handle = &s_char_handle,
            },

            {0}
        }
    },

    {0}
};

static void ble_start_adv(void)
{
    struct ble_gap_adv_params adv_params = {
        .conn_mode = BLE_GAP_CONN_MODE_UND,
        .disc_mode = BLE_GAP_DISC_MODE_GEN,
    };

    struct ble_hs_adv_fields fields;

    memset(&fields, 0, sizeof(fields));

    fields.flags = BLE_HS_ADV_F_DISC_GEN |
                   BLE_HS_ADV_F_BREDR_UNSUP;

    fields.name = (uint8_t *)BLE_DEVICE_NAME;
    fields.name_len = strlen(BLE_DEVICE_NAME);
    fields.name_is_complete = 1;

    ble_gap_adv_set_fields(&fields);

    int rc = ble_gap_adv_start(
        BLE_OWN_ADDR_PUBLIC,
        NULL,
        BLE_HS_FOREVER,
        &adv_params,
        NULL,
        NULL
    );

    if (rc != 0) {
        ESP_LOGE(TAG, "adv start fail: %d", rc);
    } else {
        ESP_LOGI(TAG, "BLE advertising...");
    }
}

static void nimble_sync_cb(void)
{
    uint8_t own_addr_type;

    ble_hs_id_infer_auto(0, &own_addr_type);

    ble_start_adv();
}

void ble_host_task(void *param)
{
    nimble_port_run();

    nimble_port_freertos_deinit();
}

void ble_wifi_deinit(void)
{
    ESP_LOGI(TAG, "BLE deinit");

    s_ble_running = false;

    // Stop the NimBLE host task
    nimble_port_stop();

    // Deinit the NimBLE port (this also disables and deinits the BT controller)
    nimble_port_deinit();

    ESP_LOGI(TAG, "BLE deinit done");
}

esp_err_t ble_wifi_init(void)
{
    ESP_LOGI(TAG, "NimBLE BLE WiFi init");

    esp_err_t ret;

    ret = nimble_port_init();

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nimble port init failed");
        return ret;
    }

    ble_svc_gap_init();
    ble_svc_gatt_init();

    ble_svc_gap_device_name_set(BLE_DEVICE_NAME);

    ble_gatts_count_cfg(gatt_svcs);
    ble_gatts_add_svcs(gatt_svcs);

    ble_hs_cfg.sync_cb = nimble_sync_cb;

    nimble_port_freertos_init(ble_host_task);

    s_ble_running = true;

    ESP_LOGI(TAG, "BLE init success");

    return ESP_OK;
}
