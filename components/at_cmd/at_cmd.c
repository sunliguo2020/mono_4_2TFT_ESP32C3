#include "at_cmd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Wifi_hw_Y.h"
#include "weather.h"
#include "AHT30_Y.h"
#include "OPT3001_Y.h"
#include "BATTERY_Y.h"

#define TAG "at_cmd"

#define STDIN_READ_DELAY_MS 50
#define AT_CMD_BUF_LEN      256

static char s_cmd_buf[AT_CMD_BUF_LEN];
static int s_cmd_len = 0;

// ---------- 响应发送 ----------

static void send_resp(const char *fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (len > 0) write(STDOUT_FILENO, buf, len);
}

static void send_ok(void)    { send_resp("\r\nOK\r\n"); }
static void send_error(void) { send_resp("\r\nERROR\r\n"); }

// ---------- NVS WiFi 保存 ----------

static void save_wifi_to_nvs(const char *ssid, const char *password) {
    nvs_handle_t handle;
    if (nvs_open("wifi", NVS_READWRITE, &handle) == ESP_OK) {
        nvs_set_str(handle, "ssid", ssid);
        nvs_set_str(handle, "password", password);
        nvs_commit(handle);
        nvs_close(handle);
        ESP_LOGI(TAG, "WiFi saved to NVS: ssid=%s", ssid);
    }
}

// ---------- AT 指令处理 ----------

static int cmd_wifi(const char *args) {
    if (!args || args[0] == '\0') {
        send_resp("\r\n+WIFI: need ssid,password\r\n"); send_error(); return -1;
    }
    const char *comma = strchr(args, ',');
    if (!comma) {
        send_resp("\r\n+WIFI: format: ssid,password\r\n"); send_error(); return -1;
    }
    char ssid[64] = {0}, pwd[64] = {0};
    size_t ssid_len = comma - args;
    if (ssid_len >= sizeof(ssid)) ssid_len = sizeof(ssid) - 1;
    memcpy(ssid, args, ssid_len);
    strncpy(pwd, comma + 1, sizeof(pwd) - 1);
    send_resp("\r\n+WIFI: saving ssid=%s\r\n", ssid);
    save_wifi_to_nvs(ssid, pwd);
    send_resp("\r\n+WIFI: saved, restarting...\r\n");
    send_ok();
    vTaskDelay(pdMS_TO_TICKS(500));
    esp_restart();
    return 0;
}

static int cmd_weather(void) {
    if (!weather_is_valid()) {
        send_resp("\r\n+WEATHER: no data\r\n"); send_error(); return -1;
    }
    send_resp("\r\n+WEATHER: today=%s %d~%dC %s %s 日出%s\r\n"
              "         tomorrow=%s %d~%dC\r\n",
              weather_get_text(),
              weather_get_temp_min(), weather_get_temp_max(),
              weather_get_wind_dir(), weather_get_wind_scale(),
              weather_get_sunrise(),
              weather_get_text_tmrw(),
              weather_get_temp_min_tmrw(), weather_get_temp_max_tmrw());
    send_ok();
    return 0;
}

static int cmd_temp(void) {
    aht30_y_read_update_ui();
    send_resp("\r\n+TEMP: requested\r\n");
    send_ok();
    return 0;
}

static int cmd_lux(void) {
    float lux = 0;
    if (opt3001_y_get_last_lux(&lux))
        send_resp("\r\n+LUX: %.0f lux\r\n", lux);
    else
        send_resp("\r\n+LUX: no data\r\n");
    send_ok();
    return 0;
}

static int cmd_bat(void) {
    battery_y_read_update_ui();
    send_resp("\r\n+BAT: requested\r\n");
    send_ok();
    return 0;
}

static int cmd_rst(void) {
    send_resp("\r\n+RST: restarting...\r\n");
    send_ok();
    vTaskDelay(pdMS_TO_TICKS(200));
    esp_restart();
    return 0;
}

static int cmd_help(void) {
    send_resp("\r\nSupported AT commands:\r\n"
              "  AT                 Test\r\n"
              "  AT+HELP            This help\r\n"
              "  AT+WIFI=ssid,pwd   Set WiFi (restart)\r\n"
              "  AT+WEATHER?        Query weather\r\n"
              "  AT+TEMP?           Query temp/humi\r\n"
              "  AT+LUX?            Query light\r\n"
              "  AT+BAT?            Query battery\r\n"
              "  AT+RST             Restart\r\n");
    send_ok();
    return 0;
}

// ---------- 指令分发 ----------

static int process_cmd(const char *line) {
    char buf[AT_CMD_BUF_LEN];
    strncpy(buf, line, sizeof(buf));
    buf[sizeof(buf) - 1] = '\0';
    int len = strlen(buf);
    while (len > 0 && (buf[len-1] == '\r' || buf[len-1] == '\n')) buf[--len] = '\0';
    if (len == 0) return 0;

    ESP_LOGI(TAG, "AT cmd: %s", buf);

    if (strcmp(buf, "AT") == 0)                        { send_ok(); return 0; }
    if (strcmp(buf, "AT+HELP") == 0 || strcmp(buf, "AT+HELP?") == 0) return cmd_help();
    if (strcmp(buf, "AT+WEATHER?") == 0)               return cmd_weather();
    if (strcmp(buf, "AT+TEMP?") == 0)                  return cmd_temp();
    if (strcmp(buf, "AT+LUX?") == 0)                   return cmd_lux();
    if (strcmp(buf, "AT+BAT?") == 0)                   return cmd_bat();
    if (strcmp(buf, "AT+RST") == 0)                    return cmd_rst();
    if (strncmp(buf, "AT+WIFI=", 8) == 0)              return cmd_wifi(buf + 8);

    send_resp("\r\nERROR: unknown\r\n");
    send_error();
    return -1;
}

// ---------- stdin 接收任务 ----------

static void stdin_task(void *arg) {
    (void)arg;
    char c;
    while (1) {
        int ret = read(STDIN_FILENO, &c, 1);
        if (ret <= 0) {
            vTaskDelay(pdMS_TO_TICKS(STDIN_READ_DELAY_MS));
            continue;
        }
        // 回显
        write(STDOUT_FILENO, &c, 1);

        if (c == '\r' || c == '\n') {
            if (s_cmd_len > 0) {
                s_cmd_buf[s_cmd_len] = '\0';
                process_cmd(s_cmd_buf);
                s_cmd_len = 0;
            }
        } else if (c == '\b' || c == 0x7F) {
            if (s_cmd_len > 0) s_cmd_len--;
        } else {
            if (s_cmd_len < (int)sizeof(s_cmd_buf) - 2)
                s_cmd_buf[s_cmd_len++] = c;
        }
    }
}

// ---------- 初始化 ----------

void at_cmd_init(void) {
    ESP_LOGI(TAG, "AT cmd via stdin/stdout (USB console)");
    send_resp("\r\nAT ready\r\n");
    xTaskCreate(stdin_task, "at_cmd", 4096, NULL, 5, NULL);
    ESP_LOGI(TAG, "AT cmd task started");
}