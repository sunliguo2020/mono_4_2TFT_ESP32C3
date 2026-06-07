#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"

#define MAX_WIFI_CREDS   10      // 最多保存 10 组 WiFi 凭据
#define WIFI_SSID_LEN    33      // SSID 最大长度（含 \0）
#define WIFI_PASS_LEN    65      // 密码最大长度（含 \0）

/**
 * @brief WiFi 凭据结构体
 */
typedef struct {
    char ssid[WIFI_SSID_LEN];
    char password[WIFI_PASS_LEN];
} wifi_cred_t;

/**
 * @brief 初始化 WiFi STA 并尝试连接
 * 内部会从 NVS 加载凭据，自动尝试连接
 */
void wifi_hw_init(void);

/**
 * @brief 检查当前 WiFi 是否已连接
 */
bool wifi_is_connected(void);

/**
 * @brief 阻塞等待 WiFi 连接成功
 * @param ticks_to_wait 超时时间（FreeRTOS tick）
 * @return true 连接成功
 */
bool wifi_wait_connected(TickType_t ticks_to_wait);

/**
 * @brief 从 NVS 加载所有已保存的 WiFi 凭据
 */
void wifi_load_from_nvs(void);

/**
 * @brief 添加一组 WiFi 凭据到 NVS 存储
 * @param ssid  WiFi 名称
 * @param pass  WiFi 密码
 * @return 当前凭据总数，-1 表示失败
 */
int wifi_save_cred_to_nvs(const char *ssid, const char *pass);

/**
 * @brief 获取已保存的凭据总数
 */
int wifi_get_cred_count(void);

/**
 * @brief 获取指定索引的凭据
 * @param idx  索引（0 ~ count-1）
 * @param cred 输出凭据
 * @return true 获取成功
 */
bool wifi_get_cred(int idx, wifi_cred_t *cred);

/**
 * @brief 切换到下一个凭据重试连接
 * 内部调用 esp_wifi_disconnect + esp_wifi_connect 切换
 */
void wifi_try_next_cred(void);

/**
 * @brief 重置凭据尝试索引，从头开始
 */
void wifi_reset_cred_index(void);

/**
 * @brief 删除所有保存的 WiFi 凭据
 */
void wifi_clear_all_creds(void);

// 以下全局变量供外部快速访问当前使用的凭据
extern char MY_SSID[WIFI_SSID_LEN];
extern char MY_PASSWORD[WIFI_PASS_LEN];