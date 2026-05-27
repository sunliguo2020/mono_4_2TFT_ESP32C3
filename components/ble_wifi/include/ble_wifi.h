#ifndef __BLE_WIFI_H__
#define __BLE_WIFI_H__
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 配网完成回调函数类型
 * 当 BLE 收到 WiFi 凭据并保存到 NVS 后调用此回调
 */
typedef void (*ble_provisioning_done_cb_t)(void);

esp_err_t ble_wifi_init(void);
void ble_wifi_deinit(void);

/**
 * @brief 注册配网完成回调
 * @param cb 回调函数，配网完成后调用
 */
void ble_wifi_register_done_cb(ble_provisioning_done_cb_t cb);

/**
 * @brief 检查 BLE 是否正在运行
 * @return true BLE 正在运行
 */
bool ble_wifi_is_running(void);

#ifdef __cplusplus
}
#endif

#endif
