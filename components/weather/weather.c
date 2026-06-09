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

// 今天（第1天）
static char s_weather_text[32] = "";
static char s_weather_fxdate[16] = "";
static int s_weather_temp_max = 0;
static int s_weather_temp_min = 0;
static char s_weather_wind_dir[32] = "";
static char s_weather_wind_scale[16] = "";
static char s_weather_sunrise[8] = "";

// 明天（第2天）
static char s_weather_text_tmrw[32] = "";
static char s_weather_fxdate_tmrw[16] = "";
static int s_weather_temp_max_tmrw = 0;
static int s_weather_temp_min_tmrw = 0;
static char s_weather_wind_dir_tmrw[32] = "";
static char s_weather_wind_scale_tmrw[16] = "";
static char s_weather_sunrise_tmrw[8] = "";

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

// ---------- JSON字段解析辅助函数 ----------

// 查找 buf 中的 field，读取字符串值到 out
static const char* parse_str(const char *buf, const char *field, char *out, size_t out_sz) {
    if (!buf) return NULL;
    const char *key = strstr(buf, field);
    if (!key) return buf;
    const char *p = strchr(key, ':');
    if (!p) return buf;
    p++; while (*p == ' ' || *p == '\t' || *p == '"') p++;
    const char *end = strchr(p, '"');
    if (end && end > p) {
        size_t len = end - p;
        if (len >= out_sz) len = out_sz - 1;
        memcpy(out, p, len); out[len] = '\0';
        return end + 1;
    }
    return buf;
}

// 查找 buf 中的 field，读取整数值到 out
static const char* parseInt(const char *buf, const char *field, int *out) {
    if (!buf) return NULL;
    const char *key = strstr(buf, field);
    if (!key) return buf;
    const char *p = strchr(key, ':');
    if (!p) return buf;
    p++; while (*p == ' ' || *p == '\t' || *p == '"') p++;
    if (*p >= '0' && *p <= '9') {
        *out = 0; bool neg = false;
        if (*p == '-') { neg = true; p++; }
        while (*p >= '0' && *p <= '9') { *out = *out * 10 + (*p - '0'); p++; }
        if (neg) *out = -*out;
    }
    return p;
}

// 查找 windScale（字符串或数值类型）
static const char* parse_wind_scale(const char *buf, const char *field, char *out, size_t out_sz) {
    if (!buf) return buf;
    const char *key = strstr(buf, field);
    if (!key) return buf;
    const char *p = strchr(key, ':');
    if (!p) return buf;
    p++; while (*p == ' ' || *p == '\t' || *p == '"') p++;

    // 尝试字符串值
    const char *end = strchr(p, '"');
    if (end && end > p) {
        size_t len = end - p;
        if (len >= out_sz) len = out_sz - 1;
        memcpy(out, p, len); out[len] = '\0';
        return end + 1;
    }
    // 尝试数值
    const char *nend = p;
    while (*nend >= '0' && *nend <= '9') nend++;
    if (nend > p) {
        size_t len = nend - p;
        if (len >= out_sz) len = out_sz - 1;
        memcpy(out, p, len); out[len] = '\0';
    }
    return buf;
}

// ---------- 单日数据解析 ----------

static const char* parse_one_day(const char *pos,
    char *text, size_t text_sz,
    char *fxdate, size_t fxdate_sz,
    int *tmax, int *tmin,
    char *wdir, size_t wdir_sz,
    char *wscale, size_t wscale_sz,
    char *sunrise, size_t sunrise_sz)
{
    if (!pos || pos[0] == '\0') return NULL;

    const char *fxd_key = strstr(pos, "\"fxDate\"");
    if (!fxd_key) return NULL;

    const char *next = strstr(fxd_key + 1, "\"fxDate\"");

    // fxDate
    const char *p = strchr(fxd_key, ':');
    if (p) { p++; while (*p == ' ' || *p == '\t' || *p == '"') p++;
        const char *start = p; const char *end = strchr(p, '"');
        if (end && end > start) { size_t len = end - start;
            if (len >= fxdate_sz) len = fxdate_sz - 1;
            memcpy(fxdate, start, len); fxdate[len] = '\0'; }
    }

    // textDay
    parse_str(fxd_key, "\"textDay\"", text, text_sz);
    if (text[0] == '\0') parse_str(fxd_key, "\"text\"", text, text_sz);

    // temp
    parseInt(fxd_key, "\"tempMax\"", tmax);
    parseInt(fxd_key, "\"tempMin\"", tmin);

    // windDir
    if (wdir_sz > 0 && wdir) wdir[0] = '\0';
    parse_str(fxd_key, "\"windDirDay\"", wdir, wdir_sz);
    if (wdir[0] == '\0') parse_str(fxd_key, "\"windDir\"", wdir, wdir_sz);

    // windScale (字符串或数值)
    if (wscale_sz > 0 && wscale) wscale[0] = '\0';
    parse_wind_scale(fxd_key, "\"windScaleDay\"", wscale, wscale_sz);
    if (wscale[0] == '\0') parse_wind_scale(fxd_key, "\"windScale\"", wscale, wscale_sz);

    // sunrise
    if (sunrise_sz > 0 && sunrise) sunrise[0] = '\0';
    parse_str(fxd_key, "\"sunrise\"", sunrise, sunrise_sz);

    return next;
}

// ---------- JSON解析入口 ----------

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

    // 解析第一天
    const char *next = parse_one_day(buf,
        s_weather_text, sizeof(s_weather_text),
        s_weather_fxdate, sizeof(s_weather_fxdate),
        &s_weather_temp_max, &s_weather_temp_min,
        s_weather_wind_dir, sizeof(s_weather_wind_dir),
        s_weather_wind_scale, sizeof(s_weather_wind_scale),
        s_weather_sunrise, sizeof(s_weather_sunrise));

    // 解析第二天
    if (next) {
        parse_one_day(next,
            s_weather_text_tmrw, sizeof(s_weather_text_tmrw),
            s_weather_fxdate_tmrw, sizeof(s_weather_fxdate_tmrw),
            &s_weather_temp_max_tmrw, &s_weather_temp_min_tmrw,
            s_weather_wind_dir_tmrw, sizeof(s_weather_wind_dir_tmrw),
            s_weather_wind_scale_tmrw, sizeof(s_weather_wind_scale_tmrw),
            s_weather_sunrise_tmrw, sizeof(s_weather_sunrise_tmrw));
    }

    if (s_weather_text[0] == '\0' && s_weather_temp_max == 0) {
        ESP_LOGW(TAG, "weather parse: no valid data"); return false; }
    ESP_LOGI(TAG, "day1: %s %s, %d~%dC, %s %s, sunrise=%s",
             s_weather_fxdate, s_weather_text,
             s_weather_temp_min, s_weather_temp_max,
             s_weather_wind_dir, s_weather_wind_scale, s_weather_sunrise);
    if (s_weather_fxdate_tmrw[0])
        ESP_LOGI(TAG, "day2: %s %s, %d~%dC, %s %s, sunrise=%s",
                 s_weather_fxdate_tmrw, s_weather_text_tmrw,
                 s_weather_temp_min_tmrw, s_weather_temp_max_tmrw,
                 s_weather_wind_dir_tmrw, s_weather_wind_scale_tmrw, s_weather_sunrise_tmrw);
    return true;
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

    char query[256];
    snprintf(query, sizeof(query), "location=%s&key=%s", WEATHER_LOCATION, WEATHER_API_KEY);
    esp_http_client_config_t cfg = {
        .host = WEATHER_API_HOST, .path = WEATHER_API_PATH, .query = query,
        .transport_type = HTTP_TRANSPORT_OVER_SSL, .event_handler = http_event_handler, .user_data = &resp,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .buffer_size = HTTP_CLIENT_RX_BUFFER, .buffer_size_tx = HTTP_CLIENT_TX_BUFFER, .timeout_ms = HTTPS_TIMEOUT_MS };

    ESP_LOGI(TAG, "HTTPS GET ...");
    esp_err_t err = http_perform_with_retry(&cfg, "weather");
    if (err != ESP_OK && response_buffer[0] == '\0') { ESP_LOGW(TAG, "request failed"); free(response_buffer); return; }
    if (response_buffer[0] == '\0') { ESP_LOGW(TAG, "empty response"); free(response_buffer); return; }

    bool is_gzip = (resp.len >= 2 && (uint8_t)response_buffer[0] == 0x1f && (uint8_t)response_buffer[1] == 0x8b);
    if (is_gzip) {
        ESP_LOGI(TAG, "gzip compressed (%u bytes)", (unsigned)resp.len);
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
                    free(response_buffer); response_buffer = decomp_buf; resp.buf = decomp_buf; resp.len = out_cap - out_rem; }
                else { ESP_LOGW(TAG, "decompress failed"); free(decomp_buf); } } } }
    ESP_LOGI(TAG, "weather resp: %.100s", response_buffer);

    if (!parse_weather_json(response_buffer)) { ESP_LOGW(TAG, "parse failed"); free(response_buffer); return; }
    s_weather_valid = true;
    if (ymd != 0) s_weather_last_ymd = ymd;
    free(response_buffer);
}

static void fmt_date_short(char *out, size_t sz, const char *fxdate) {
    if (!fxdate || strlen(fxdate) < 10) { out[0] = '\0'; return; }
    out[0] = fxdate[5]; out[1] = fxdate[6];
    out[2] = '-'; out[3] = fxdate[8]; out[4] = fxdate[9]; out[5] = '\0';
}

void weather_apply_to_ui(void) {
    if (!s_weather_valid) { ESP_LOGW(TAG, "no valid weather data"); return; }

    char date1[8] = "";
    fmt_date_short(date1, sizeof(date1), s_weather_fxdate);
    char weather_line[96];
    if (date1[0])
        snprintf(weather_line, sizeof(weather_line), "%s %s %s %s",
                 date1, s_weather_text, s_weather_wind_dir, s_weather_wind_scale);
    else
        snprintf(weather_line, sizeof(weather_line), "%s %s %s",
                 s_weather_text, s_weather_wind_dir, s_weather_wind_scale);
    char temp_hi_lo[32];
    snprintf(temp_hi_lo, sizeof(temp_hi_lo), "%d~%d", s_weather_temp_min, s_weather_temp_max);

    char date2[8] = "";
    fmt_date_short(date2, sizeof(date2), s_weather_fxdate_tmrw);
    char weather_line2[48];
    if (date2[0] && s_weather_text_tmrw[0])
        snprintf(weather_line2, sizeof(weather_line2), "%s %s %d~%d℃",
                 date2, s_weather_text_tmrw, s_weather_temp_min_tmrw, s_weather_temp_max_tmrw);
    else
        weather_line2[0] = '\0';

    lvgl_ui_set_weather(weather_line, temp_hi_lo, weather_line2);
    ESP_LOGI(TAG, "UI day1: %s, %s", weather_line, temp_hi_lo);
    if (weather_line2[0]) ESP_LOGI(TAG, "UI day2: %s", weather_line2);
}

static SemaphoreHandle_t s_weather_done = NULL;
static void weather_poll_task(void *arg) {
    (void)arg; weather_poll_once(); xSemaphoreGive(s_weather_done); vTaskDelete(NULL); }
void weather_poll_once_blocking(void) {
    if (!s_weather_done) s_weather_done = xSemaphoreCreateBinary();
    xTaskCreate(weather_poll_task, "weather_poll", 16384, NULL, 5, NULL);
    xSemaphoreTake(s_weather_done, portMAX_DELAY); }