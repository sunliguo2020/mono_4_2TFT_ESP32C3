#include "Wifi_hw_Y.h"
#include "hw_time_Y.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "lwip/sockets.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <string.h>
#include "lvgl_display.h"

#define TAG "wifi_hw_Y"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

char MY_SSID[WIFI_SSID_LEN] = {0};
char MY_PASSWORD[WIFI_PASS_LEN] = {0};

char ip_address_str[16] = {0};
char wifi_ssid[33] = {0};

bool wifi_connected = false;
EventGroupHandle_t s_wifi_event_group = NULL;
esp_netif_t *sta_netif = NULL;

// 多组凭据缓存
static wifi_cred_t s_creds[MAX_WIFI_CREDS];
static int s_cred_count = 0;
static int s_cred_index = 0;   // 当前正在尝试的凭据索引

// 加载单个凭据的辅助函数
static wifi_cred_t *load_cred_from_nvs(nvs_handle_t handle, int idx) {
    wifi_cred_t *cred = &s_creds[s_cred_count];
    char key[32];
    size_t len;

    // 读取 SSID
    snprintf(key, sizeof(key), "ssid_%d", idx);
    len = sizeof(cred->ssid);
    if (nvs_get_str(handle, key, cred->ssid, &len) != ESP_OK) {
        return NULL;
    }
    if (strlen(cred->ssid) == 0) {
        return NULL;
    }

    // 读取密码
    snprintf(key, sizeof(key), "pass_%d", idx);
    len = sizeof(cred->password);
    if (nvs_get_str(handle, key, cred->password, &len) != ESP_OK) {
        cred->password[0] = '\0';
    }

    s_cred_count++;
    return cred;
}

void wifi_load_from_nvs(void) {
    s_cred_count = 0;
    memset(s_creds, 0, sizeof(s_creds));

    nvs_handle_t handle;
    esp_err_t err = nvs_open("wifi_cfg", NVS_READONLY, &handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "NVS open for reading failed: %s", esp_err_to_name(err));
        return;
    }

    // 尝试加载第 0~MAX_WIFI_CREDS-1 组
    for (int i = 0; i < MAX_WIFI_CREDS; i++) {
        if (s_cred_count >= MAX_WIFI_CREDS) break;
        if (!load_cred_from_nvs(handle, i)) {
            break;  // 遇到第一个空项即停止
        }
    }

    nvs_close(handle);

    // 将第一组凭据复制到 MY_SSID / MY_PASSWORD（向后兼容）
    if (s_cred_count > 0) {
        strncpy(MY_SSID, s_creds[0].ssid, WIFI_SSID_LEN - 1);
        strncpy(MY_PASSWORD, s_creds[0].password, WIFI_PASS_LEN - 1);
        ESP_LOGI(TAG, "Loaded %d WiFi credential(s) from NVS", s_cred_count);
        for (int i = 0; i < s_cred_count; i++) {
            ESP_LOGI(TAG, "  [%d] SSID='%s' (pwd len=%d)", i,
                     s_creds[i].ssid, strlen(s_creds[i].password));
        }
    } else {
        ESP_LOGW(TAG, "No WiFi credentials found in NVS");
    }
}

int wifi_save_cred_to_nvs(const char *ssid, const char *pass) {
    if (!ssid || strlen(ssid) == 0) {
        ESP_LOGW(TAG, "SSID is empty, not saving");
        return -1;
    }

    nvs_handle_t handle;
    if (nvs_open("wifi_cfg", NVS_READWRITE, &handle) != ESP_OK) {
        ESP_LOGE(TAG, "NVS open for write failed");
        return -1;
    }

    // 检查是否已存在相同的 SSID，若存在则更新密码
    int existing_idx = -1;
    for (int i = 0; i < s_cred_count; i++) {
        if (strcmp(s_creds[i].ssid, ssid) == 0) {
            existing_idx = i;
            break;
        }
    }

    int target_idx;
    if (existing_idx >= 0) {
        // 已有该 SSID，更新密码
        target_idx = existing_idx;
    } else {
        // 新凭据，追加到末尾
        target_idx = s_cred_count;
        if (target_idx >= MAX_WIFI_CREDS) {
            ESP_LOGW(TAG, "Max credentials (%d) reached, overwriting oldest",
                     MAX_WIFI_CREDS);
            // 循环覆盖：用新凭据替换最旧的（索引0），然后重新排列
            target_idx = 0;
        }
    }

    // 写入 NVS
    char key[32];
    snprintf(key, sizeof(key), "ssid_%d", target_idx);
    nvs_set_str(handle, key, ssid);
    snprintf(key, sizeof(key), "pass_%d", target_idx);
    nvs_set_str(handle, key, pass);
    nvs_commit(handle);
    nvs_close(handle);

    // 更新内存缓存
    strncpy(s_creds[target_idx].ssid, ssid, WIFI_SSID_LEN - 1);
    strncpy(s_creds[target_idx].password, pass, WIFI_PASS_LEN - 1);

    // 如果是新 SSID 且没有覆盖，增加计数
    if (existing_idx < 0 && target_idx == s_cred_count && s_cred_count < MAX_WIFI_CREDS) {
        s_cred_count++;
    }

    // 如果只有这一组凭据，也更新 MY_SSID / MY_PASSWORD
    if (s_cred_count == 1 || target_idx == 0) {
        strncpy(MY_SSID, ssid, WIFI_SSID_LEN - 1);
        strncpy(MY_PASSWORD, pass, WIFI_PASS_LEN - 1);
    }

    ESP_LOGI(TAG, "Saved credential [%d]: SSID='%s'", target_idx, ssid);

    // 重新加载以确保一致性
    wifi_load_from_nvs();
    return s_cred_count;
}

int wifi_get_cred_count(void) {
    return s_cred_count;
}

bool wifi_get_cred(int idx, wifi_cred_t *cred) {
    if (idx < 0 || idx >= s_cred_count || !cred) {
        return false;
    }
    *cred = s_creds[idx];
    return true;
}

void wifi_reset_cred_index(void) {
    s_cred_index = 0;
    if (s_cred_count > 0) {
        strncpy(MY_SSID, s_creds[0].ssid, WIFI_SSID_LEN - 1);
        strncpy(MY_PASSWORD, s_creds[0].password, WIFI_PASS_LEN - 1);
    }
}

void wifi_try_next_cred(void) {
    if (s_cred_count <= 0) {
        ESP_LOGW(TAG, "No credentials to try");
        return;
    }

    // 切换到下一个凭据
    s_cred_index = (s_cred_index + 1) % s_cred_count;

    // 更新当前 SSID/PASSWORD 全局变量
    strncpy(MY_SSID, s_creds[s_cred_index].ssid, WIFI_SSID_LEN - 1);
    strncpy(MY_PASSWORD, s_creds[s_cred_index].password, WIFI_PASS_LEN - 1);

    ESP_LOGI(TAG, "Trying next credential [%d/%d]: SSID='%s'",
             s_cred_index + 1, s_cred_count, MY_SSID);

    // 断开当前连接并重新用新凭据连接
    esp_wifi_disconnect();
    vTaskDelay(pdMS_TO_TICKS(100));

    wifi_config_t wifi_config = {0};
    strncpy((char *)wifi_config.sta.ssid, MY_SSID, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, MY_PASSWORD, sizeof(wifi_config.sta.password) - 1);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK;

    esp_err_t ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (ret == ESP_OK) {
        esp_wifi_connect();
    } else {
        ESP_LOGE(TAG, "Failed to set WiFi config for next cred: %s",
                 esp_err_to_name(ret));
    }
}

void wifi_clear_all_creds(void) {
    nvs_handle_t handle;
    if (nvs_open("wifi_cfg", NVS_READWRITE, &handle) == ESP_OK) {
        // 删除所有凭据
        for (int i = 0; i < MAX_WIFI_CREDS; i++) {
            char key[32];
            snprintf(key, sizeof(key), "ssid_%d", i);
            nvs_erase_key(handle, key);
            snprintf(key, sizeof(key), "pass_%d", i);
            nvs_erase_key(handle, key);
        }
        nvs_commit(handle);
        nvs_close(handle);
    }

    s_cred_count = 0;
    s_cred_index = 0;
    memset(s_creds, 0, sizeof(s_creds));
    memset(MY_SSID, 0, sizeof(MY_SSID));
    memset(MY_PASSWORD, 0, sizeof(MY_PASSWORD));
    ESP_LOGI(TAG, "All WiFi credentials cleared");
}

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data) {
    static int s_retry_num = 0;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        if (s_cred_count > 0) {
            esp_wifi_connect();
        } else {
            ESP_LOGW(TAG, "No credentials to connect with");
        }
    }
    else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t *disc =
            (wifi_event_sta_disconnected_t *)event_data;
        ESP_LOGW(TAG, "sta disconnected, reason=%d",
                 disc ? (int)disc->reason : -1);
        wifi_connected = false;
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);

        if (s_retry_num < (5 * s_cred_count)) {
            // 每重试 5 次切换一个凭据
            if (s_retry_num > 0 && s_retry_num % 5 == 0) {
                wifi_try_next_cred();
            } else {
                esp_wifi_connect();
            }
            s_retry_num++;
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGW(TAG, "All credentials exhausted after %d retries",
                     s_retry_num);
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        wifi_connected = true;
        esp_wifi_set_ps(WIFI_PS_NONE);
        ESP_LOGW(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));

        wifi_ap_record_t ap_info;
        esp_err_t ret = esp_wifi_sta_get_ap_info(&ap_info);
        if (ret == ESP_OK) {
            snprintf(wifi_ssid, sizeof(wifi_ssid), "%.*s",
                     sizeof(wifi_ssid) - 1, ap_info.ssid);
        }
        wifi_online();
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void) {
    s_wifi_event_group = xEventGroupCreate();

    esp_err_t ret = esp_netif_init();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_ERROR_CHECK(ret);
    }

    esp_err_t loop_ret = esp_event_loop_create_default();
    if (loop_ret != ESP_OK && loop_ret != ESP_ERR_INVALID_STATE) {
        ESP_ERROR_CHECK(loop_ret);
    }

    if (sta_netif) {
        esp_netif_destroy(sta_netif);
        sta_netif = NULL;
    }
    sta_netif = esp_netif_create_default_wifi_sta();
    if (!sta_netif) {
        ESP_LOGE(TAG, "Failed to create default WiFi STA netif");
        return;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_ERROR_CHECK(ret);
    }

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    ESP_LOGI(TAG, "Setting WiFi config with %d credential(s)", s_cred_count);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    // 使用当前索引的凭据
    if (s_cred_count > 0) {
        wifi_config_t wifi_config = {0};
        strncpy((char *)wifi_config.sta.ssid, MY_SSID, sizeof(wifi_config.sta.ssid) - 1);
        strncpy((char *)wifi_config.sta.password, MY_PASSWORD, sizeof(wifi_config.sta.password) - 1);
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK;

        esp_err_t ret_set = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
        if (ret_set != ESP_OK) {
            ESP_LOGE(TAG, "Failed to set WiFi config: %s", esp_err_to_name(ret_set));
        }
    }

    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGW(TAG, "WiFi started, waiting for connection...");
}

void wifi_hw_init(void) {
    wifi_load_from_nvs();

    if (s_cred_count == 0) {
        ESP_LOGW(TAG, "No WiFi credentials found in NVS, "
                      "waiting for Bluetooth provisioning...");
        s_wifi_event_group = xEventGroupCreate();
        return;
    }

    s_cred_index = 0;
    wifi_init_sta();
}

bool wifi_is_connected(void) {
    return wifi_connected;
}

bool wifi_wait_connected(TickType_t ticks_to_wait) {
    if (!s_wifi_event_group) {
        return false;
    }
    EventBits_t bits = xEventGroupWaitBits(
        s_wifi_event_group, WIFI_CONNECTED_BIT,
        pdFALSE, pdFALSE, ticks_to_wait);
    return (bits & WIFI_CONNECTED_BIT) != 0;
}