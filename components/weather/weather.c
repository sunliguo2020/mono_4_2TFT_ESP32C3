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

#define WEATHER_API_KEY    "6b2ed874ff8a4414926f5c26c9737d02"
#define WEATHER_LOCATION   "118.73,36.86"

#define WEATHER_API_HOST   "mg7fc3p9rj.re.qweatherapi.com"
#define WEATHER_API_PATH   "/v7/weather/3d"

#define MAX_HTTP_OUTPUT_BUFFER 4096
#define HTTPS_TIMEOUT_MS 30000
#define HTTP_CLIENT_RX_BUFFER 1024
#define HTTP_CLIENT_TX_BUFFER 512
#define WEATHER_DEBUG_DNS 1

extern const char weather_root_cert_pem_start[]
    asm("_binary_weather_root_cert_pem_start");
extern const char weather_root_cert_pem_end[]
    asm("_binary_weather_root_cert_pem_end");

static char s_weather_text[32] = "";
static char s_weather_icon[8] = "";
static char s_weather_fxdate[16] = "";
static int s_weather_temp_max = 0;
static int s_weather_temp_min = 0;
static int s_weather_humidity = 0;
static char s_weather_wind_dir[32] = "";
static char s_weather_wind_scale[16] = "";
static bool s_weather_valid = false;
static int s_weather_last_ymd = 0;
static int s_weather_update_attempts = 0;

typedef struct {
    char *buf;
    size_t len;
    size_t cap;
} http_resp_t;

static int weather_calc_ymd(void) {
    time_t now = 0;
    struct tm timeinfo = {0};
    time(&now);
    localtime_r(&now, &timeinfo);
    if (timeinfo.tm_year < 120) return 0;
    return (timeinfo.tm_year + 1900) * 10000 + (timeinfo.tm_mon + 1) * 100 +
           timeinfo.tm_mday;
}

static bool parse_weather_json(const char *buf) {
    if (!buf || buf[0] == '\0') return false;
    const char *code_str = strstr(buf, "\"code\"");
    if (!code_str) { ESP_LOGW(TAG, "no code field"); return false; }
    const char *code_val = strchr(code_str, ':');
    if (!code_val) return false;
    code_val++;
    while (*code_val == ' ' || *code_val == '\t' || *code_val == '"') code_val++;
    if (code_val[0] != '2' || code_val[1] != '0' || code_val[2] != '0') {
        ESP_LOGW(TAG, "API code not 200: %.3s", code_val); return false; }

    // 提取预报日期 fxDate
    const char *fxd_key = strstr(buf, "\"fxDate\"");
    if (fxd_key) {
        const char *p = strchr(fxd_key, ':');
        if (p) { p++; while (*p == ' ' || *p == '\t' || *p == '"') p++;
            const char *start = p; const char *end = strchr(p, '"');
            if (end && end > start) { size_t len = end - start;
                if (len >= sizeof(s_weather_fxdate)) len = sizeof(s_weather_fxdate) - 1;
                memcpy(s_weather_fxdate, start, len); s_weather_fxdate[len] = '\0'; } } }

    const char *text_key = strstr(buf, "\"textDay\"");
    if (!text_key) text_key = strstr(buf, "\"text\"");
    if (text_key) {
        const char *p = strchr(text_key, ':');
        if (p) { p++; while (*p == ' ' || *p == '\t' || *p == '"') p++;
            const char *start = p; const char *end = strchr(p, '"');
            if (end && end > start) { size_t len = end - start;
                if (len >= sizeof(s_weather_text)) len = sizeof(s_weather_text) - 1;
                memcpy(s_weather_text, start, len); s_weather_text[len] = '\0'; } } }

    const char *icon_key = strstr(buf, "\"iconDay\"");
    if (!icon_key) icon_key = strstr(buf, "\"icon\"");
    if (icon_key) {
        const char *p = strchr(icon_key, ':');
        if (p) { p++; while (*p == ' ' || *p == '\t' || *p == '"') p++;
            const char *start = p; const char *end = strchr(p, '"');
            if (end && end > start) { size_t len = end - start;
                if (len >= sizeof(s_weather_icon)) len = sizeof(s_weather_icon) - 1;
                memcpy(s_weather_icon, start, len); s_weather_icon[len] = '\0'; } } }

    const char *tmax_key = strstr(buf, "\"tempMax\"");
    if (tmax_key) {
        const char *p = strchr(tmax_key, ':');
        if (p) { p++; while (*p == ' ' || *p == '\t' || *p == '"') p++;
            if (*p >= '0' && *p <= '9') { s_weather_temp_max = 0; bool neg = false;
                if (*p == '-') { neg = true; p++; }
                while (*p >= '0' && *p <= '9') { s_weather_temp_max = s_weather_temp_max * 10 + (*p - '0'); p++; }
                if (neg) s_weather_temp_max = -s_weather_temp_max; } } }

    const char *tmin_key = strstr(buf, "\"tempMin\"");
    if (tmin_key) {
        const char *p = strchr(tmin_key, ':');
        if (p) { p++; while (*p == ' ' || *p == '\t' || *p == '"') p++;
            if (*p >= '0' && *p <= '9') { s_weather_temp_min = 0; bool neg = false;
                if (*p == '-') { neg = true; p++; }
                while (*p >= '0' && *p <= '9') { s_weather_temp_min = s_weather_temp_min * 10 + (*p - '0'); p++; }
                if (neg) s_weather_temp_min = -s_weather_temp_min; } } }

    const char *wdir_key = strstr(buf, "\"windDirDay\"");
    if (wdir_key) {
        const char *p = strchr(wdir_key, ':');
        if (p) { p++; while (*p == ' ' || *p == '\t' || *p == '"') p++;
            const char *start = p; const char *end = strchr(p, '"');
            if (end && end > start) { size_t len = end - start;
                if (len >= sizeof(s_weather_wind_dir)) len = sizeof(s_weather_wind_dir) - 1;
                memcpy(s_weather_wind_dir, start, len); s_weather_wind_dir[len] = '\0'; } } }

    const char *wsc_key = strstr(buf, "\"windScaleDay\"");
    if (wsc_key) {
        const char *p = strchr(wsc_key, ':');
        if (p) { p++; while (*p == ' ' || *p == '\t' || *p == '"') p++;
            const char *start = p; const char *end = strchr(p, '"');
            if (end && end > start) { size_t len = end - start;
                if (len >= sizeof(s_weather_wind_scale)) len = sizeof(s_weather_wind_scale) - 1;
                memcpy(s_weather_wind_scale, start, len); s_weather_wind_scale[len] = '\0'; } } }

    const char *humi_key = strstr(buf, "\"humidity\"");
    if (humi_key) {
        const char *p = strchr(humi_key, ':');
        if (p) { p++; while (*p == ' ' || *p == '\t' || *p == '"') p++;
            if (*p >= '0' && *p <= '9') { s_weather_humidity = 0;
                while (*p >= '0' && *p <= '9') { s_weather_humidity = s_weather_humidity * 10 + (*p - '0'); p++; } } } }

    if (s_weather_text[0] == '\0' && s_weather_temp_max == 0) {
        ESP_LOGW(TAG, "weather parse: no valid data"); return false; }
    ESP_LOGI(TAG, "weather: %s %s, %d~%dC, %s %s, hum=%d%%",
             s_weather_fxdate, s_weather_text,
             s_weather_temp_min, s_weather_temp_max,
             s_weather_wind_dir, s_weather_wind_scale, s_weather_humidity);
    return true;
}

static const char* weather_icon_to_symbol(const char *icon) {
    if (!icon) return LV_SYMBOL_HOME;
    int code = atoi(icon);
    if (code == 100 || code == 102 || code == 103) return LV_SYMBOL_OK;
    if (code == 101 || code == 104) return LV_SYMBOL_LOOP;
    if (code >= 300 && code <= 399) return LV_SYMBOL_DOWN;
    if (code >= 400 && code <= 499) return LV_SYMBOL_SHUFFLE;
    if (code >= 500 && code <= 515) return LV_SYMBOL_WARNING;
    if (code >= 800 && code <= 804) return LV_SYMBOL_LOOP;
    return LV_SYMBOL_HOME;
}

static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    http_resp_t *resp = (http_resp_t *)evt->user_data;
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR: ESP_LOGD(TAG, "HTTP_EVENT_ERROR"); break;
        case HTTP_EVENT_ON_CONNECTED: ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED"); break;
        case HTTP_EVENT_HEADER_SENT: ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT"); break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "Header: %s: %s", evt->header_key, evt->header_value); break;
        case HTTP_EVENT_ON_DATA: {
            if (!resp || !resp->buf || resp->cap == 0 || evt->data_len <= 0) break;
            size_t copy_len = (size_t)evt->data_len;
            if (resp->len + copy_len >= resp->cap) {
                if (resp->len < resp->cap) copy_len = resp->cap - resp->len - 1;
                else copy_len = 0; }
            if (copy_len) { memcpy(resp->buf + resp->len, evt->data, copy_len); resp->len += copy_len; resp->buf[resp->len] = '\0'; }
            break; }
        case HTTP_EVENT_ON_FINISH: break;
        case HTTP_EVENT_DISCONNECTED: {
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
            if (err != 0) { ESP_LOGI(TAG, "TLS err: 0x%x, mbedtls: 0x%x", err, mbedtls_err); }
            break; }
        case HTTP_EVENT_REDIRECT: ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT"); esp_http_client_set_redirection(evt->client); break;
        default: break; }
    return ESP_OK;
}

static esp_err_t http_perform_with_retry(esp_http_client_config_t *cfg, const char *tag_prefix) {
    esp_err_t err = ESP_FAIL;
    for (int attempt = 0; attempt < 3; attempt++) {
        if (attempt > 0) { ESP_LOGW(TAG, "%s retry %d", tag_prefix, attempt + 1); vTaskDelay(pdMS_TO_TICKS(2000)); }
        esp_http_client_handle_t client = esp_http_client_init(cfg);
        if (!client) { ESP_LOGW(TAG, "%s init failed", tag_prefix); continue; }
        err = esp_http_client_perform(client);
        if (err == ESP_OK)
            ESP_LOGI(TAG, "%s status=%d len=%" PRId64, tag_prefix, esp_http_client_get_status_code(client), esp_http_client_get_content_length(client));
        else
            ESP_LOGW(TAG, "%s perform failed: %s", tag_prefix, esp_err_to_name(err));
        esp_http_client_cleanup(client);
        if (err == ESP_OK) break; }
    return err;
}

bool weather_need_update(void) {
    int ymd = weather_calc_ymd();
    if (ymd == 0) return true;
    return s_weather_last_ymd != ymd;
}

void weather_force_update(void) { s_weather_last_ymd = 0; s_weather_update_attempts = 0; }
const char* weather_get_text(void) { return s_weather_text; }
bool weather_is_valid(void) { return s_weather_valid; }
const char* weather_get_icon_symbol(void) { return weather_icon_to_symbol(s_weather_icon); }

void weather_poll_once(void) {
    int ymd = weather_calc_ymd();
    if (ymd != 0 && s_weather_last_ymd == ymd) { ESP_LOGI(TAG, "weather update skipped"); return; }
    if (ymd != 0 && s_weather_last_ymd != ymd) s_weather_update_attempts = 0;
    s_weather_update_attempts++;

    char *response_buffer = (char *)calloc(MAX_HTTP_OUTPUT_BUFFER + 1, 1);
    if (!response_buffer) { ESP_LOGW(TAG, "buffer alloc failed"); return; }
    http_resp_t resp = { .buf = response_buffer, .len = 0, .cap = MAX_HTTP_OUTPUT_BUFFER + 1 };

    if (!wifi_wait_connected(pdMS_TO_TICKS(5000))) { ESP_LOGW(TAG, "wifi not connected"); free(response_buffer); return; }
    vTaskDelay(pdMS_TO_TICKS(3000));

    if (!hw_time_is_synced()) {
        ESP_LOGI(TAG, "waiting for time sync...");
        if (hw_time_wait_for_sync(60000)) ESP_LOGI(TAG, "time synced");
        else ESP_LOGW(TAG, "time sync timeout"); }

#if WEATHER_DEBUG_DNS
    for (int i = 0; i < DNS_MAX_SERVERS; i++) {
        const ip_addr_t *dns = dns_getserver(i);
        if (dns && !ip_addr_isany(dns)) { char s[IPADDR_STRLEN_MAX]; ipaddr_ntoa_r(dns, s, sizeof(s)); ESP_LOGI(TAG, "DNS %d: %s", i, s); } }
    struct addrinfo hints = {0}, *ai = NULL;
    hints.ai_family = AF_INET; hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(WEATHER_API_HOST, NULL, &hints, &ai) == 0 && ai) {
        for (struct addrinfo *p = ai; p; p = p->ai_next) {
            if (p->ai_family == AF_INET) { char s[INET_ADDRSTRLEN]; inet_ntoa_r(((struct sockaddr_in*)p->ai_addr)->sin_addr, s, sizeof(s)); ESP_LOGI(TAG, "DNS result: %s", s); } }
        freeaddrinfo(ai); }
#endif

    char query[256];
    snprintf(query, sizeof(query), "location=%s&key=%s", WEATHER_LOCATION, WEATHER_API_KEY);
    esp_http_client_config_t cfg = {
        .host = WEATHER_API_HOST, .path = WEATHER_API_PATH, .query = query,
        .transport_type = HTTP_TRANSPORT_OVER_SSL, .event_handler = http_event_handler, .user_data = &resp,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .buffer_size = HTTP_CLIENT_RX_BUFFER, .buffer_size_tx = HTTP_CLIENT_TX_BUFFER, .timeout_ms = HTTPS_TIMEOUT_MS };

    ESP_LOGI(TAG, "HTTPS GET https://%s%s?location=%s&key=...", WEATHER_API_HOST, WEATHER_API_PATH, WEATHER_LOCATION);
    esp_err_t err = http_perform_with_retry(&cfg, "weather");
    if (err != ESP_OK && response_buffer[0] == '\0') { ESP_LOGW(TAG, "request failed"); free(response_buffer); return; }
    if (response_buffer[0] == '\0') { ESP_LOGW(TAG, "empty response"); free(response_buffer); return; }

    bool is_gzip = (resp.len >= 2 && (uint8_t)response_buffer[0] == 0x1f && (uint8_t)response_buffer[1] == 0x8b);
    if (is_gzip) {
        ESP_LOGI(TAG, "gzip compressed (%u bytes), decompressing...", (unsigned)resp.len);
        size_t offset = 10; uint8_t flg = (uint8_t)response_buffer[3];
        if ((flg & 0x04) && offset + 2 <= resp.len) { uint16_t xlen = (uint8_t)response_buffer[offset] | ((uint8_t)response_buffer[offset+1]<<8); offset += 2 + xlen; }
        if (flg & 0x08) { while (offset < resp.len && response_buffer[offset]) offset++; offset++; }
        if (flg & 0x10) { while (offset < resp.len && response_buffer[offset]) offset++; offset++; }
        if ((flg & 0x02) && offset + 2 <= resp.len) offset += 2;
        if (offset < resp.len) {
            size_t out_cap = (resp.len - offset) * 4; if (out_cap < 512) out_cap = 512;
            char *decomp_buf = (char *)calloc(1, out_cap + 1);
            if (decomp_buf) {
                tinfl_decompressor inflator; tinfl_init(&inflator);
                size_t in_rem = resp.len - offset, out_rem = out_cap;
                tinfl_status st = tinfl_decompress(&inflator, (const mz_uint8*)(response_buffer+offset), &in_rem, (mz_uint8*)decomp_buf, (mz_uint8*)decomp_buf, &out_rem, TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF);
                if (st == TINFL_STATUS_DONE || st == TINFL_STATUS_HAS_MORE_OUTPUT) {
                    decomp_buf[out_cap - out_rem] = '\0';
                    ESP_LOGI(TAG, "decompressed: %u -> %u bytes", (unsigned)resp.len, (unsigned)(out_cap - out_rem));
                    free(response_buffer); response_buffer = decomp_buf; resp.buf = decomp_buf; resp.len = out_cap - out_rem; }
                else { ESP_LOGW(TAG, "decompress failed status=%d", (int)st); free(decomp_buf); } } } }
    ESP_LOGI(TAG, "weather resp: %s", response_buffer);

    if (!parse_weather_json(response_buffer)) { ESP_LOGW(TAG, "parse failed"); free(response_buffer); return; }

    s_weather_valid = true;
    if (ymd != 0) s_weather_last_ymd = ymd;
    ESP_LOGI(TAG, "weather fetched: %s %s, %d~%dC, %s %s, hum=%d%%",
             s_weather_fxdate, s_weather_text,
             s_weather_temp_min, s_weather_temp_max,
             s_weather_wind_dir, s_weather_wind_scale, s_weather_humidity);
    free(response_buffer);
}

// 简单天气中文转英文
static const char* translate_weather(const char* cn) {
    if (!cn || cn[0]=='\0') return "--";
    if (strcmp(cn, "晴")==0 || strcmp(cn, "晴间多云")==0) return "Sunny";
    if (strcmp(cn, "多云")==0 || strcmp(cn, "少云")==0) return "Cloudy";
    if (strcmp(cn, "阴")==0) return "Overcast";
    if (strcmp(cn, "阵雨")==0 || strcmp(cn, "小雨")==0) return "Light Rain";
    if (strcmp(cn, "中雨")==0) return "Mod Rain";
    if (strcmp(cn, "大雨")==0 || strcmp(cn, "暴雨")==0) return "Heavy Rain";
    if (strcmp(cn, "雷阵雨")==0) return "T-Storm";
    if (strcmp(cn, "小雪")==0) return "Light Snow";
    if (strcmp(cn, "中雪")==0) return "Mod Snow";
    if (strcmp(cn, "大雪")==0) return "Heavy Snow";
    if (strcmp(cn, "雾")==0 || strcmp(cn, "霾")==0) return "Foggy";
    return cn;
}

// 风向中文转英文
static const char* translate_wind(const char* cn) {
    if (!cn) return "";
    if (strcmp(cn, "北风")==0) return "N";
    if (strcmp(cn, "南风")==0) return "S";
    if (strcmp(cn, "西风")==0) return "W";
    if (strcmp(cn, "东风")==0) return "E";
    if (strcmp(cn, "东北风")==0) return "NE";
    if (strcmp(cn, "西北风")==0) return "NW";
    if (strcmp(cn, "东南风")==0) return "SE";
    if (strcmp(cn, "西南风")==0) return "SW";
    if (strcmp(cn, "北")==0) return "N";
    if (strcmp(cn, "南")==0) return "S";
    if (strcmp(cn, "西")==0) return "W";
    if (strcmp(cn, "东")==0) return "E";
    static char buf[16];
    snprintf(buf, sizeof(buf), "%s", cn);
    return buf;
}

void weather_apply_to_ui(void) {
    if (!s_weather_valid) { ESP_LOGW(TAG, "no valid weather data to apply"); return; }
    
    // 判断是今天还是明天
    const char *day_label = "";
    int ymd = weather_calc_ymd();
    if (ymd != 0 && s_weather_fxdate[0]) {
        int fxd = atoi(s_weather_fxdate);
        if (fxd == ymd) day_label = "Today";
        else if (fxd == ymd + 1) day_label = "Tomor";
        else day_label = "Day+2";
    }
    
    const char *en_weather = translate_weather(s_weather_text);
    const char *en_wind = translate_wind(s_weather_wind_dir);
    
    char weather_line[48];
    snprintf(weather_line, sizeof(weather_line), "%s %s", day_label, en_weather);
    char temp_hi_lo[32];
    snprintf(temp_hi_lo, sizeof(temp_hi_lo), "%d~%dC", s_weather_temp_min, s_weather_temp_max);
    char info_str[64];
    snprintf(info_str, sizeof(info_str), "%s %s Hum:%d%%", en_wind, s_weather_wind_scale, s_weather_humidity);
    
    lvgl_ui_set_weather(weather_line, temp_hi_lo, info_str);
    ESP_LOGI(TAG, "weather UI: %s, %s, %s", weather_line, temp_hi_lo, info_str);
}

static SemaphoreHandle_t s_weather_done = NULL;
static void weather_poll_task(void *arg) {
    (void)arg;
    weather_poll_once();
    xSemaphoreGive(s_weather_done);
    vTaskDelete(NULL); }

void weather_poll_once_blocking(void) {
    if (!s_weather_done) s_weather_done = xSemaphoreCreateBinary();
    xTaskCreate(weather_poll_task, "weather_poll", 16384, NULL, 5, NULL);
    xSemaphoreTake(s_weather_done, portMAX_DELAY); }