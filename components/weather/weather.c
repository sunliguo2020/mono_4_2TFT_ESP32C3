#include "weather.h"

#include <inttypes.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <time.h>

#include "Wifi_hw_Y.h"
#include "esp_http_client.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "esp_tls.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "lvgl_display.h"
#include "lvgl.h"
#include "lwip/dns.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "hw_time_Y.h"
#include "esp_crt_bundle.h"
#include "miniz.h"

#define TAG "weather"

// ============================================================
// 和风天气配置
// 请在此处填入你的和风天气 API Key
// 免费版注册地址: https://dev.qweather.com/
// ============================================================
#define WEATHER_API_KEY    "6b2ed874ff8a4414926f5c26c9737d02"
#define WEATHER_LOCATION   "118.73,36.86"  // 经度,纬度 (山东潍坊寿光市)
// 也可以使用城市ID，例如 "101010100" 表示北京
// #define WEATHER_LOCATION "101010100"

#define WEATHER_API_HOST   "mg7fc3p9rj.re.qweatherapi.com"
#define WEATHER_API_PATH   "/v7/weather/now"

#define MAX_HTTP_OUTPUT_BUFFER 2048
#define HTTPS_TIMEOUT_MS 30000
#define HTTP_CLIENT_RX_BUFFER 1024
#define HTTP_CLIENT_TX_BUFFER 512
#define WEATHER_DEBUG_DNS 1

// 和风天气 API 的根证书 (ISRG Root X1 / Let's Encrypt)
// devapi.qweather.com 使用 Let's Encrypt 证书
extern const char weather_root_cert_pem_start[]
    asm("_binary_weather_root_cert_pem_start");
extern const char weather_root_cert_pem_end[]
    asm("_binary_weather_root_cert_pem_end");

// 天气状态缓存
static char s_weather_text[32] = "";    // 天气描述，如 "晴"
static char s_weather_icon[8] = "";     // 天气图标代码，如 "100"
static int s_weather_temp = 0;          // 温度，单位℃
static int s_weather_feels_like = 0;    // 体感温度
static int s_weather_humidity = 0;      // 相对湿度百分比
static bool s_weather_valid = false;    // 数据是否有效
static int s_weather_last_ymd = 0;      // 上次更新日期
static int s_weather_update_attempts = 0;

// HTTP 响应收集结构
typedef struct {
    char *buf;
    size_t len;
    size_t cap;
} http_resp_t;

// 计算 YYYYMMDD 格式的日期
static int weather_calc_ymd(void) {
    time_t now = 0;
    struct tm timeinfo = {0};
    time(&now);
    localtime_r(&now, &timeinfo);
    if (timeinfo.tm_year < 120) {  // year < 2020 => time not set
        return 0;
    }
    return (timeinfo.tm_year + 1900) * 10000 + (timeinfo.tm_mon + 1) * 100 +
           timeinfo.tm_mday;
}

// 从 JSON 响应中提取天气字段
// 和风天气 v7 API 返回格式:
// {
//   "code": "200",
//   "now": {
//     "temp": "22",
//     "feelsLike": "20",
//     "text": "晴",
//     "icon": "100",
//     "humidity": "45"
//   }
// }
static bool parse_weather_json(const char *buf) {
    if (!buf || buf[0] == '\0') {
        return false;
    }

    // 检查返回码
    const char *code_str = strstr(buf, "\"code\"");
    if (!code_str) {
        ESP_LOGW(TAG, "no code field in response");
        return false;
    }
    const char *code_val = strchr(code_str, ':');
    if (!code_val) return false;
    code_val++;
    while (*code_val == ' ' || *code_val == '\t' || *code_val == '"') code_val++;
    if (code_val[0] != '2' || code_val[1] != '0' || code_val[2] != '0') {
        ESP_LOGW(TAG, "API return code not 200: %.3s", code_val);
        return false;
    }

    // 提取温度 temp
    const char *temp_key = strstr(buf, "\"temp\"");
    if (temp_key) {
        const char *p = strchr(temp_key, ':');
        if (p) {
            p++;
            while (*p == ' ' || *p == '\t' || *p == '"') p++;
            if (*p >= '0' && *p <= '9') {
                s_weather_temp = 0;
                bool neg = false;
                if (*p == '-') { neg = true; p++; }
                while (*p >= '0' && *p <= '9') {
                    s_weather_temp = s_weather_temp * 10 + (*p - '0');
                    p++;
                }
                if (neg) s_weather_temp = -s_weather_temp;
            }
        }
    }

    // 提取体感温度 feelsLike
    const char *feels_key = strstr(buf, "\"feelsLike\"");
    if (feels_key) {
        const char *p = strchr(feels_key, ':');
        if (p) {
            p++;
            while (*p == ' ' || *p == '\t' || *p == '"') p++;
            if (*p >= '0' && *p <= '9') {
                s_weather_feels_like = 0;
                bool neg = false;
                if (*p == '-') { neg = true; p++; }
                while (*p >= '0' && *p <= '9') {
                    s_weather_feels_like = s_weather_feels_like * 10 + (*p - '0');
                    p++;
                }
                if (neg) s_weather_feels_like = -s_weather_feels_like;
            }
        }
    }

    // 提取天气描述 text
    const char *text_key = strstr(buf, "\"text\"");
    if (text_key) {
        const char *p = strchr(text_key, ':');
        if (p) {
            p++;
            while (*p == ' ' || *p == '\t' || *p == '"') p++;
            const char *start = p;
            const char *end = strchr(p, '"');
            if (end && end > start) {
                size_t len = end - start;
                if (len >= sizeof(s_weather_text)) len = sizeof(s_weather_text) - 1;
                memcpy(s_weather_text, start, len);
                s_weather_text[len] = '\0';
            }
        }
    }

    // 提取图标代码 icon
    const char *icon_key = strstr(buf, "\"icon\"");
    if (icon_key) {
        const char *p = strchr(icon_key, ':');
        if (p) {
            p++;
            while (*p == ' ' || *p == '\t' || *p == '"') p++;
            const char *start = p;
            const char *end = strchr(p, '"');
            if (end && end > start) {
                size_t len = end - start;
                if (len >= sizeof(s_weather_icon)) len = sizeof(s_weather_icon) - 1;
                memcpy(s_weather_icon, start, len);
                s_weather_icon[len] = '\0';
            }
        }
    }

    // 提取湿度 humidity
    const char *humi_key = strstr(buf, "\"humidity\"");
    if (humi_key) {
        const char *p = strchr(humi_key, ':');
        if (p) {
            p++;
            while (*p == ' ' || *p == '\t' || *p == '"') p++;
            if (*p >= '0' && *p <= '9') {
                s_weather_humidity = 0;
                while (*p >= '0' && *p <= '9') {
                    s_weather_humidity = s_weather_humidity * 10 + (*p - '0');
                    p++;
                }
            }
        }
    }

    // 检查是否至少获取到了温度和天气描述
    if (s_weather_text[0] == '\0' && s_weather_temp == 0) {
        ESP_LOGW(TAG, "weather parse: no valid data found");
        return false;
    }

    ESP_LOGI(TAG, "weather: %s, temp=%d°C, feels=%d°C, humidity=%d%%",
             s_weather_text, s_weather_temp, s_weather_feels_like, s_weather_humidity);
    return true;
}

// 将和风天气图标代码转换为文本符号
// 参考: https://dev.qweather.com/docs/resource/icons/
static const char* weather_icon_to_symbol(const char *icon) {
    if (!icon) return LV_SYMBOL_HOME;
    
    int code = atoi(icon);
    switch (code) {
        case 100: // 晴
        case 102: // 少云
        case 103: // 晴间多云
            return LV_SYMBOL_OK;
        case 101: // 多云
        case 104: // 阴
            return LV_SYMBOL_LOOP;
        case 300: // 阵雨
        case 301: // 强阵雨
        case 302: // 雷阵雨
        case 303: // 强雷阵雨
        case 304: // 雷阵雨伴有冰雹
        case 305: // 小雨
        case 306: // 中雨
        case 307: // 大雨
        case 308: // 极端降雨
        case 309: // 毛毛雨/细雨
        case 310: // 暴雨
        case 311: // 大暴雨
        case 312: // 特大暴雨
        case 313: // 冻雨
        case 314: // 小到中雨
        case 315: // 中到大雨
        case 316: // 大到暴雨
        case 317: // 暴雨到大暴雨
        case 318: // 大暴雨到特大暴雨
        case 399: // 雨
            return LV_SYMBOL_DOWN;
        case 400: // 小雪
        case 401: // 中雪
        case 402: // 大雪
        case 403: // 暴雪
        case 404: // 雨夹雪
        case 405: // 雨雪天气
        case 406: // 阵雨夹雪
        case 407: // 阵雪
        case 408: // 小到中雪
        case 409: // 中到大雪
        case 410: // 大到暴雪
        case 499: // 雪
            return LV_SYMBOL_SHUFFLE;
        case 500: // 薄雾
        case 501: // 雾
        case 502: // 霾
        case 503: // 扬沙
        case 504: // 浮尘
        case 507: // 沙尘暴
        case 508: // 强沙尘暴
        case 509: // 浓雾
        case 510: // 强浓雾
        case 511: // 中度霾
        case 512: // 重度霾
        case 513: // 严重霾
        case 514: // 大雾
        case 515: // 特强浓雾
            return LV_SYMBOL_WARNING;
        case 800: // 新月
        case 801: // 蛾眉月
        case 802: // 上弦月
        case 803: // 盈凸月
        case 804: // 满月
            return LV_SYMBOL_LOOP;
        default:
            return LV_SYMBOL_HOME;
    }
}

// HTTP 事件处理
static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    http_resp_t *resp = (http_resp_t *)evt->user_data;

    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key,
                     evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA: {
            if (!resp || !resp->buf || resp->cap == 0 || evt->data_len <= 0) {
                break;
            }
            size_t copy_len = (size_t)evt->data_len;
            if (resp->len + copy_len >= resp->cap) {
                if (resp->len < resp->cap) {
                    copy_len = resp->cap - resp->len - 1;
                } else {
                    copy_len = 0;
                }
            }
            if (copy_len) {
                memcpy(resp->buf + resp->len, evt->data, copy_len);
                resp->len += copy_len;
                resp->buf[resp->len] = '\0';
            }
            break;
        }
        case HTTP_EVENT_ON_FINISH:
            break;
        case HTTP_EVENT_DISCONNECTED: {
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error(
                (esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            break;
        }
        case HTTP_EVENT_REDIRECT:
            ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
            esp_http_client_set_redirection(evt->client);
            break;
        default:
            break;
    }
    return ESP_OK;
}

// 执行 HTTP 请求（带重试，最多 3 次）
static esp_err_t http_perform_with_retry(esp_http_client_config_t *cfg,
                                         const char *tag_prefix) {
    esp_err_t err = ESP_FAIL;
    for (int attempt = 0; attempt < 3; attempt++) {
        if (attempt > 0) {
            ESP_LOGW(TAG, "%s retry attempt %d", tag_prefix, attempt + 1);
            vTaskDelay(pdMS_TO_TICKS(2000));
        }

        esp_http_client_handle_t client = esp_http_client_init(cfg);
        if (!client) {
            ESP_LOGW(TAG, "%s http client init failed", tag_prefix);
            continue;
        }
        err = esp_http_client_perform(client);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "%s status = %d, len = %" PRId64, tag_prefix,
                     esp_http_client_get_status_code(client),
                     esp_http_client_get_content_length(client));
        } else {
            ESP_LOGW(TAG, "%s perform failed: %s", tag_prefix,
                     esp_err_to_name(err));
        }
        esp_http_client_cleanup(client);
        if (err == ESP_OK) {
            break;
        }
    }
    return err;
}

// 检查是否需要更新天气（每天一次）
bool weather_need_update(void) {
    int ymd = weather_calc_ymd();
    if (ymd == 0) {
        return true;  // 时间未同步，尝试更新
    }
    return s_weather_last_ymd != ymd;
}

// 强制下次更新
void weather_force_update(void) {
    s_weather_last_ymd = 0;
    s_weather_update_attempts = 0;
}

// 获取当前天气文本（用于 UI 显示）
const char* weather_get_text(void) {
    return s_weather_text;
}

// 获取当前温度
int weather_get_temp(void) {
    return s_weather_temp;
}

// 获取体感温度
int weather_get_feels_like(void) {
    return s_weather_feels_like;
}

// 获取湿度
int weather_get_humidity(void) {
    return s_weather_humidity;
}

// 数据是否有效
bool weather_is_valid(void) {
    return s_weather_valid;
}

// 获取天气图标符号
const char* weather_get_icon_symbol(void) {
    return weather_icon_to_symbol(s_weather_icon);
}

// 执行一次天气查询并更新 UI
void weather_poll_once(void) {
    // 检查是否需要更新
    int ymd = weather_calc_ymd();
    if (ymd != 0 && s_weather_last_ymd == ymd) {
        ESP_LOGI(TAG, "weather update skipped (already done today)");
        return;
    }
    if (ymd != 0 && s_weather_last_ymd != ymd) {
        s_weather_update_attempts = 0;
    }
    s_weather_update_attempts++;

    // 检查 API Key 是否已配置
    if (strcmp(WEATHER_API_KEY, "YOUR_QWEATHER_API_KEY") == 0) {
        ESP_LOGW(TAG, "Weather API key not configured! Please set WEATHER_API_KEY in weather.c");
        // 显示提示信息
        lvgl_ui_set_weather("--", "--°C", "");
        s_weather_valid = false;
        return;
    }

    char *response_buffer = (char *)calloc(MAX_HTTP_OUTPUT_BUFFER + 1, 1);
    if (response_buffer == NULL) {
        ESP_LOGW(TAG, "response buffer alloc failed");
        return;
    }
    http_resp_t resp = {
        .buf = response_buffer,
        .len = 0,
        .cap = MAX_HTTP_OUTPUT_BUFFER + 1,
    };

    if (!wifi_wait_connected(pdMS_TO_TICKS(5000))) {
        ESP_LOGW(TAG, "wifi not connected, skip weather");
        free(response_buffer);
        return;
    }

    // 等待网络完全就绪（DNS 解析等）
    vTaskDelay(pdMS_TO_TICKS(3000));

    // 等待时间同步完成（TLS 证书验证需要正确的时间）
    if (!hw_time_is_synced()) {
        ESP_LOGI(TAG, "waiting for time sync before HTTPS request...");
        if (hw_time_wait_for_sync(20000)) {
            ESP_LOGI(TAG, "time synced, proceeding with weather request");
        } else {
            ESP_LOGW(TAG, "time sync timeout, proceeding anyway (TLS may fail)");
        }
    }

#if WEATHER_DEBUG_DNS
    // 打印 DNS 服务器信息
    for (int i = 0; i < DNS_MAX_SERVERS; i++) {
        const ip_addr_t *dns = dns_getserver(i);
        if (dns && !ip_addr_isany(dns)) {
            char dns_str[IPADDR_STRLEN_MAX] = {0};
            ipaddr_ntoa_r(dns, dns_str, sizeof(dns_str));
            ESP_LOGI(TAG, "DNS server %d: %s", i, dns_str);
        }
    }
    // 尝试解析域名
    struct addrinfo hints = {0};
    struct addrinfo *res = NULL;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int dns_err = getaddrinfo(WEATHER_API_HOST, NULL, &hints, &res);
    if (dns_err != 0 || res == NULL) {
        ESP_LOGE(TAG, "DNS lookup failed for %s: %d", WEATHER_API_HOST, dns_err);
    } else {
        int idx = 0;
        for (struct addrinfo *p = res; p != NULL; p = p->ai_next) {
            if (p->ai_family == AF_INET) {
                struct sockaddr_in *addr = (struct sockaddr_in *)p->ai_addr;
                char ip_str[INET_ADDRSTRLEN] = {0};
                inet_ntoa_r(addr->sin_addr, ip_str, sizeof(ip_str));
                ESP_LOGI(TAG, "DNS result %d: %s", idx++, ip_str);
            }
        }
        freeaddrinfo(res);
    }
#endif

    // 构建查询参数
    char query[256];
    snprintf(query, sizeof(query), "location=%s&key=%s", WEATHER_LOCATION, WEATHER_API_KEY);

    esp_http_client_config_t cfg = {
        .host = WEATHER_API_HOST,
        .path = WEATHER_API_PATH,
        .query = query,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .event_handler = http_event_handler,
        .user_data = &resp,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .buffer_size = HTTP_CLIENT_RX_BUFFER,
        .buffer_size_tx = HTTP_CLIENT_TX_BUFFER,
        .timeout_ms = HTTPS_TIMEOUT_MS,
    };

    ESP_LOGI(TAG, "HTTPS GET https://%s%s?location=%s&key=...",
             WEATHER_API_HOST, WEATHER_API_PATH, WEATHER_LOCATION);
    esp_err_t err = http_perform_with_retry(&cfg, "weather");

    if (err != ESP_OK && response_buffer[0] == '\0') {
        ESP_LOGW(TAG, "weather request failed");
        free(response_buffer);
        return;
    }

    if (response_buffer[0] == '\0') {
        ESP_LOGW(TAG, "weather empty response");
        free(response_buffer);
        return;
    }

    // gzip 解压（和风天气 API 默认使用 gzip）
    bool is_gzip = (resp.len >= 2 && (uint8_t)response_buffer[0] == 0x1f && (uint8_t)response_buffer[1] == 0x8b);
    if (is_gzip) {
        ESP_LOGI(TAG, "gzip compressed (%u bytes), decompressing...", (unsigned)resp.len);
        size_t offset = 10;
        uint8_t flg = (uint8_t)response_buffer[3];
        if ((flg & 0x04) && offset + 2 <= resp.len) { uint16_t xlen = (uint8_t)response_buffer[offset] | ((uint8_t)response_buffer[offset+1]<<8); offset += 2 + xlen; }
        if (flg & 0x08) { while (offset < resp.len && response_buffer[offset]) offset++; offset++; }
        if (flg & 0x10) { while (offset < resp.len && response_buffer[offset]) offset++; offset++; }
        if ((flg & 0x02) && offset + 2 <= resp.len) offset += 2;
        if (offset < resp.len) {
            size_t out_cap = (resp.len - offset) * 4;
            if (out_cap < 512) out_cap = 512;
            char *decomp_buf = (char *)calloc(1, out_cap + 1);
            if (decomp_buf) {
                tinfl_decompressor inflator; tinfl_init(&inflator);
                size_t in_rem = resp.len - offset, out_rem = out_cap;
                tinfl_status st = tinfl_decompress(&inflator, (const mz_uint8*)(response_buffer+offset), &in_rem, (mz_uint8*)decomp_buf, (mz_uint8*)decomp_buf, &out_rem, TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF);
                if (st == TINFL_STATUS_DONE || st == TINFL_STATUS_HAS_MORE_OUTPUT) {
                    decomp_buf[out_cap - out_rem] = '\0';
                    ESP_LOGI(TAG, "decompressed: %u -> %u bytes", (unsigned)resp.len, (unsigned)(out_cap - out_rem));
                    free(response_buffer);
                    response_buffer = decomp_buf;
                    resp.buf = decomp_buf; resp.len = out_cap - out_rem;
                } else { ESP_LOGW(TAG, "decompress failed status=%d", (int)st); free(decomp_buf); }
            }
        }
    }
    ESP_LOGI(TAG, "weather resp: %s", response_buffer);

    // 解析 JSON
    bool parsed = parse_weather_json(response_buffer);
    if (!parsed) {
        ESP_LOGW(TAG, "weather parse failed");
        free(response_buffer);
        return;
    }

    // 标记数据有效，不立即推送到 UI（由调用者决定何时推送）
    s_weather_valid = true;
    if (ymd != 0) {
        s_weather_last_ymd = ymd;
    }

    ESP_LOGI(TAG, "weather fetched: %s, temp=%d°C, humidity=%d%%", s_weather_text, s_weather_temp, s_weather_humidity);
    free(response_buffer);
}

// 将缓存的天气数据推送到 UI
void weather_apply_to_ui(void) {
    if (!s_weather_valid) {
        ESP_LOGW(TAG, "no valid weather data to apply");
        return;
    }
    char temp_str[16];
    snprintf(temp_str, sizeof(temp_str), "%d°C", s_weather_temp);
    const char *icon_sym = weather_icon_to_symbol(s_weather_icon);
    lvgl_ui_set_weather(s_weather_text, temp_str, icon_sym);
    ESP_LOGI(TAG, "weather applied to UI: %s %s", s_weather_text, temp_str);
}

// 在独立高栈任务中执行天气请求并返回前阻塞主线程
static SemaphoreHandle_t s_weather_done = NULL;
static void weather_poll_task(void *arg) {
    (void)arg;
    weather_poll_once();
    xSemaphoreGive(s_weather_done);
    vTaskDelete(NULL);
}

void weather_poll_once_blocking(void) {
    if (!s_weather_done) s_weather_done = xSemaphoreCreateBinary();
    xTaskCreate(weather_poll_task, "weather_poll", 16384, NULL, 5, NULL);
    // 等待任务完成，无超时
    xSemaphoreTake(s_weather_done, portMAX_DELAY);
}
