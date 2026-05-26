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
#include "lvgl_display.h"

#define TAG "wifi_hw_Y" // 定义打印标签

#define WIFI_CONNECTED_BIT BIT0 // 定义WIFI连接成功的标志位
#define WIFI_FAIL_BIT BIT1      // 定义WIFI连接失败的标志位
#define MY_SSID "ruizhixinxi"            // 定义AP的SSID
#define MY_PASSWORD "ruizhi2016"  // 定义AP的密�?
char ip_address_str[16] = {0}; // ����ip��ַ��������ʾ
char wifi_ssid[33] = {0};      // ���ڴ洢SSID����󳤶�?2�ֽ�
bool wifi_connected = false; // wifi����״̬��־λ
EventGroupHandle_t s_wifi_event_group; // 创建一个事件组
esp_netif_t *sta_netif = NULL;         // wifi网络接口

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
    wifi_connected = false; // wifi����״̬��־λ
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
    // lvgl_port_lock(0);
    // update_wifi_status(); // 更新wifi状�?    // lvgl_port_unlock();
    // 重试次数置为0
    s_retry_num = 0;
    // 设置事件组位
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}

// Initialize Wi-Fi STA interface and start connection.
void wifi_init_sta(void) {
    // ����wifi�¼���
  s_wifi_event_group = xEventGroupCreate();
  ESP_ERROR_CHECK(esp_netif_init());
  esp_err_t loop_ret = esp_event_loop_create_default();
  if (loop_ret != ESP_OK && loop_ret != ESP_ERR_INVALID_STATE) {
    ESP_ERROR_CHECK(loop_ret);
  }
  // 创建默认的wifi网络接口
  sta_netif = esp_netif_create_default_wifi_sta();
  // 初始化wifi配置
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  // 注册wifi事件处理程序
  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); // 设置wifi模式为STA模式
  wifi_config_t wifi_config = {
      .sta = {.ssid = MY_SSID,         // 替换为你的WiFi名称
              .password = MY_PASSWORD, // 替换为你的WiFi密码
              .threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK},
  };
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

  // 启动wifi
  ESP_ERROR_CHECK(esp_wifi_start()); // 启动wifi
  // 等待wifi连接成功或失�?  /*

  ESP_LOGW(TAG, "等待wifi连接...");

}

// Initialize NVS for Wi-Fi storage.
static void wifi_nvs_init(void) {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_LOGI(TAG, "NVS erase and re-init");
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
}

// Placeholder for time download logic (unused).
void wifi_download_time(void) {

}

// Wait for connection result and notify UI.
void wifi_download_init(void) {
  // �ȴ�WiFi���ӳɹ���ʧ��
  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                         WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                         pdFALSE, pdFALSE, portMAX_DELAY);

  if (bits & WIFI_CONNECTED_BIT) {
    ESP_LOGI(TAG, "Connected to ap SSID:%s", wifi_ssid);
     wifi_online(); // disable UI update when lvgl not initialized
  } else if (bits & WIFI_FAIL_BIT) {
    ESP_LOGI(TAG, "Failed to connect to ap");
  }
}

// Public Wi-Fi init sequence (NVS + STA + wait).
void wifi_hw_init(void) {
  wifi_nvs_init(); // 初始化NVS
  wifi_init_sta(); // 初始化wifi
  wifi_download_init(); // 初始化wifi下载
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














