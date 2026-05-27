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

#define TAG "wifi_hw_Y" // 定义打印标签

#define WIFI_CONNECTED_BIT BIT0 // 定义WIFI连接成功的标志位
#define WIFI_FAIL_BIT BIT1      // 定义WIFI连接失败的标志位


// #define MY_SSID "ruizhixinxi"            // 定义AP的SSID
// #define MY_PASSWORD "ruizhi2016"  // 定义AP的密码

// 【从 NVS 读取，不再写死！】
char MY_SSID[33] = {0};
char MY_PASSWORD[65] = {0};


char ip_address_str[16] = {0}; // ip地址字符串
char wifi_ssid[33] = {0};      // ssid字符串

bool wifi_connected = false; // wifi状态标志 当前是否联网
EventGroupHandle_t s_wifi_event_group; // 创建一个事件组
esp_netif_t *sta_netif = NULL;         // wifi网络接口

// 从 NVS 读取 WiFi 信息
void wifi_load_from_nvs(void) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open("wifi_cfg", NVS_READONLY, &handle);
    if (err == ESP_OK) {
        size_t len = sizeof(MY_SSID);
        nvs_get_str(handle, "ssid", MY_SSID, &len);
        len = sizeof(MY_PASSWORD);
        nvs_get_str(handle, "pass", MY_PASSWORD, &len);
        nvs_close(handle);
    }
}

// 保存 WiFi 到 NVS 蓝牙配网用
void wifi_save_to_nvs(const char *ssid, const char *pass) {
    nvs_handle_t handle;
    if (nvs_open("wifi_cfg", NVS_READWRITE, &handle) == ESP_OK) {
        nvs_set_str(handle, "ssid", ssid);
        nvs_set_str(handle, "pass", pass);
        nvs_commit(handle);
        nvs_close(handle);
    }
    strncpy(MY_SSID, ssid, sizeof(MY_SSID)-1);
    strncpy(MY_PASSWORD, pass, sizeof(MY_PASSWORD)-1);
}

// 静态函数，处理事件
// Wi-Fi/IP event handler for STA connection lifecycle.
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data) {
  // 静态变量，记录重试次数
  static int s_retry_num = 0;
  // 如果事件基是WIFI_EVENT，事件ID是WIFI_EVENT_STA_START
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    // 连接WiFi
    esp_wifi_connect();
  }
  // 如果事件基是WIFI_EVENT，事件ID是WIFI_EVENT_STA_DISCONNECTED
  else if (event_base == WIFI_EVENT &&
           event_id == WIFI_EVENT_STA_DISCONNECTED) {
    wifi_event_sta_disconnected_t *disc =
        (wifi_event_sta_disconnected_t *)event_data;
    ESP_LOGW(TAG, "sta disconnected, reason=%d", disc ? (int)disc->reason : -1);
    wifi_connected = false; // wifi状态标志 当前是否联网
    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    // lvgl_port_lock(0);
    // update_wifi_status();
    // lvgl_port_unlock(); // 更新wifi状�?    // 如果重试次数小于5
    if (s_retry_num < 5) {
      // 连接WiFi
      esp_wifi_connect();
      // 重试次数�?
      s_retry_num++;
      // 打印日志
      ESP_LOGW(TAG, "retry to connect to the AP");
    }
    // 否则
    else {
      // 设置事件组位
      xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
    }
    // 打印日志
    ESP_LOGW(TAG, "connect to the AP fail");
  }
  // 如果事件基是IP_EVENT，事件ID是IP_EVENT_STA_GOT_IP
  else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    // 将事件数据转换为ip_event_got_ip_t类型
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    wifi_connected = true;
    esp_wifi_set_ps(WIFI_PS_NONE);
    // 打印日志
    ESP_LOGW(TAG, "路由器分配到的IP got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    // 将IP地址转换为字符串并存储到全局变量
    // snprintf(ip_address_str, sizeof(ip_address_str), IPSTR,
    //          IP2STR(&event->ip_info.ip));
    // // 获取当前连接的AP信息
    wifi_ap_record_t ap_info;
    esp_err_t ret = esp_wifi_sta_get_ap_info(&ap_info);
    if (ret == ESP_OK) {
      // 将SSID复制到全局变量
      snprintf(wifi_ssid, sizeof(wifi_ssid), "%.*s", sizeof(wifi_ssid) - 1,
               ap_info.ssid);
    }
    // 更新 WiFi 图标
    wifi_online();
    // 重试次数置为0
    s_retry_num = 0;
    // 设置事件组位
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}

// Initialize Wi-Fi STA interface and start connection.
void wifi_init_sta(void) {
    // wifi初始化主函数
  s_wifi_event_group = xEventGroupCreate();
  
  // 这些函数可能已经被调用过，允许 ESP_ERR_INVALID_STATE
  esp_err_t ret = esp_netif_init();
  if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    ESP_ERROR_CHECK(ret);
  }
  
  esp_err_t loop_ret = esp_event_loop_create_default();
  if (loop_ret != ESP_OK && loop_ret != ESP_ERR_INVALID_STATE) {
    ESP_ERROR_CHECK(loop_ret);
  }
  
  // 创建默认的wifi网络接口
  // 注意：如果之前已经创建过（例如 BLE 配网后重新初始化），需要先销毁旧的
  if (sta_netif) {
    esp_netif_destroy(sta_netif);
    sta_netif = NULL;
  }
  sta_netif = esp_netif_create_default_wifi_sta();
  if (!sta_netif) {
    ESP_LOGE(TAG, "Failed to create default WiFi STA netif");
    return;
  }
  
  // 初始化wifi配置
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ret = esp_wifi_init(&cfg);
  if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    ESP_ERROR_CHECK(ret);
  }

  // 注册wifi事件处理程序
  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

  ESP_LOGI(TAG, "Setting WiFi config: SSID='%s' PWD='%s'", MY_SSID, MY_PASSWORD);
  
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); // 设置wifi模式为STA模式
  
  wifi_config_t wifi_config = {0};
    strncpy((char *)wifi_config.sta.ssid, MY_SSID, sizeof(wifi_config.sta.ssid)-1);
    strncpy((char *)wifi_config.sta.password, MY_PASSWORD, sizeof(wifi_config.sta.password)-1);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK;

  esp_err_t ret_set = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
  if (ret_set != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set WiFi config: %s", esp_err_to_name(ret_set));
  }

  // 启动wifi
  ESP_ERROR_CHECK(esp_wifi_start()); // 启动wifi
  // 等待wifi连接成功或失败 /*

  ESP_LOGW(TAG, "等待wifi连接...");

}

// Initialize NVS for Wi-Fi storage.
// 注意：NVS 初始化已在 main.c 中完成，此函数不再被调用
// 保留定义以备将来可能需要独立初始化时使用
// static void wifi_nvs_init(void) {
//   esp_err_t ret = nvs_flash_init();
//   if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
//       ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
//     ESP_LOGI(TAG, "NVS erase and re-init");
//     ESP_ERROR_CHECK(nvs_flash_erase());
//     ret = nvs_flash_init();
//   }
//   ESP_ERROR_CHECK(ret);
// }

// Placeholder for time download logic (unused).
void wifi_download_time(void) {

}

// Wait for connection result and notify UI.
void wifi_download_init(void) {
  // 等待联网完成
  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                         WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                         pdFALSE, pdFALSE, portMAX_DELAY);
  //联网成功
  if (bits & WIFI_CONNECTED_BIT) {
    ESP_LOGI(TAG, "Connected to ap SSID:%s", wifi_ssid);
     wifi_online(); // disable UI update when lvgl not initialized
  } else if (bits & WIFI_FAIL_BIT) {
    ESP_LOGI(TAG, "Failed to connect to ap");
  }
}

// Public Wi-Fi init sequence (NVS + STA + wait).
void wifi_hw_init(void) {
  // NVS 初始化已在 main.c 中完成，这里不再重复初始化
  // wifi_nvs_init(); // 注释掉，避免重复初始化
  wifi_load_from_nvs(); // 从NVS加载WiFi信息

  //如果没有wifi信息，直接返回，等待蓝牙配网写入后重启
  if (strlen(MY_SSID) == 0 || strlen(MY_PASSWORD) == 0) {
    ESP_LOGW(TAG, "No WiFi credentials found in NVS, waiting for Bluetooth provisioning...");
    // Create event group anyway so wifi_wait_connected doesn't crash on NULL handle
    s_wifi_event_group = xEventGroupCreate();
    return;
  }
  
  wifi_init_sta(); // 初始化wifi
  // 不再阻塞等待，由调用方决定是否等待以及等待多久
  // wifi_download_init(); // 移除阻塞等待
}

// Return cached STA connection state.
bool wifi_is_connected(void) {
  return wifi_connected;
}

// Block until STA is connected or timeout.
bool wifi_wait_connected(TickType_t ticks_to_wait) {
  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, ticks_to_wait);
  return (bits & WIFI_CONNECTED_BIT) != 0;
}
