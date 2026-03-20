#include "pc_status.h"

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
#include "lvgl_display.h"
#include "lvgl.h"
#include "lwip/dns.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"

#define TAG "pc_status"
#define DOUYIN_FANS_ENABLED 1
#define PC_STATUS_DEBUG_DNS 0
#define PC_STATUS_DEBUG_HEAP 0
#define MAX_HTTP_OUTPUT_BUFFER 2048
#define HTTPS_TIMEOUT_MS 30000
#define HTTP_CLIENT_RX_BUFFER 1024
#define HTTP_CLIENT_TX_BUFFER 512
#define PC_STATUS_NO_UI 0

#define PC_STATUS_HOST "open.iot.vlinkc.com"
#define PC_STATUS_PATH "/device/control"
#define PC_STATUS_QUERY                                                   \
  "action=status&token="                                                  \
  "R1JTWWNsTk9ldlhCZFBxUjhhO换成你自己的xlTjhoaHd5aUF2"      \
  "M3FNOUVuaVBZVlNCcWhmeVpWT换成你自己的DdMcFE3Y2haS2owRFl5"      \
  "c换成你自己的zA9"
#define PC_POWER_ON_QUERY                                                 \
  "action=on&token="                                                      \
  "R1JT换成你自己的xUjhhOUsyeE1Qck5nZ3V0YU5KbkM4bkxlTjhoaHd5aUF2"      \
  "M3FNOU换成你自己的eVpWTzJkYWRQOE1Gcjh3WmFsSDdMcFE3Y2haS2owRFl5"      \
  "cVFOa换成你自己的9"

/* Root cert for open.iot.vlinkc.com, taken from howsmyssl_com_root_cert.pem. */
extern const char howsmyssl_com_root_cert_pem_start[]
    asm("_binary_howsmyssl_com_root_cert_pem_start");
extern const char howsmyssl_com_root_cert_pem_end[]
    asm("_binary_howsmyssl_com_root_cert_pem_end");
/* Root cert chain for api.tikhub.io, taken from tikhub_root_cert.pem. */
extern const char tikhub_root_cert_pem_start[]
    asm("_binary_tikhub_root_cert_pem_start");
extern const char tikhub_root_cert_pem_end[]
    asm("_binary_tikhub_root_cert_pem_end");

typedef enum {
  PC_STATE_UNKNOWN = 0,
  PC_STATE_ON,
  PC_STATE_OFF,
  PC_STATE_OFFLINE
} pc_state_t;

typedef struct {
  char *buf;
  size_t len;
  size_t cap;
  bool parse_follower;
  bool follower_done;
  uint32_t follower_count;
  int key_match;
  bool value_active;
  bool parsing_digits;
} http_resp_t;

static void log_heap_state(const char *stage);
static esp_err_t http_perform_with_retry(esp_http_client_config_t *cfg,
                                         const char *auth_header,
                                         const char *tag_prefix);

// Parse JSON response into a simplified PC state.
static pc_state_t parse_pc_state(const char *buf) {
  if (!buf || buf[0] == '\0') {
    return PC_STATE_UNKNOWN;
  }

  if (strstr(buf, "\"online\":false") || strstr(buf, "\"online\":0") ||
      strstr(buf, "offline") || strstr(buf, "OFFLINE")) {
    return PC_STATE_OFFLINE;
  }
  if (strstr(buf, "\"power\":\"on\"") || strstr(buf, "\"power\":true") ||
      strstr(buf, "\"power\":1") || strstr(buf, "\"status\":1") ||
      strstr(buf, "\"status\":true")) {
    return PC_STATE_ON;
  }
  if (strstr(buf, "\"power\":\"off\"") || strstr(buf, "\"power\":false") ||
      strstr(buf, "\"power\":0") || strstr(buf, "\"status\":0") ||
      strstr(buf, "\"status\":false")) {
    return PC_STATE_OFF;
  }

  return PC_STATE_UNKNOWN;
}

static bool s_pc_on = false;
static uint32_t s_session_elapsed_min = 0;
static uint32_t s_daily_total_min = 0;
static int s_daily_ymd = 0;
static int s_last_tick_ymd = 0;
static int s_last_tick_min_of_day = -1;

static int pc_calc_ymd(const struct tm *info) {
  return (info->tm_year + 1900) * 10000 + (info->tm_mon + 1) * 100 +
         info->tm_mday;
}

// Apply parsed state to UI (or log only).
static void apply_pc_state(pc_state_t state) {
  if (state == PC_STATE_OFFLINE) {
    ESP_LOGW(TAG, "pc status offline");
    if (s_pc_on) {
      s_pc_on = false;
    }
    lvgl_ui_set_pc_state_text("X");
    {
      uint32_t session_hours = s_session_elapsed_min / 60;
      uint32_t session_minutes = s_session_elapsed_min % 60;
      uint32_t total_hours = s_daily_total_min / 60;
      uint32_t total_minutes = s_daily_total_min % 60;
      lvgl_ui_set_pc_session_time(session_hours, session_minutes);
      lvgl_ui_set_pc_total_time(total_hours, total_minutes);
    }
    return;
  }

#if PC_STATUS_NO_UI
  if (state == PC_STATE_ON) {
    ESP_LOGI(TAG, "pc status: ON");
  } else if (state == PC_STATE_OFF) {
    ESP_LOGI(TAG, "pc status: OFF");
  }
  return;
#endif

  if (state == PC_STATE_ON) {
    if (!s_pc_on) {
      s_session_elapsed_min = 0;
    }
    s_pc_on = true;
    lvgl_ui_set_pc_state_text("ON");
  } else if (state == PC_STATE_OFF) {
    if (s_pc_on) {
      s_pc_on = false;
    }
    lvgl_ui_set_pc_state_text("off");
    {
      uint32_t session_hours = s_session_elapsed_min / 60;
      uint32_t session_minutes = s_session_elapsed_min % 60;
      uint32_t total_hours = s_daily_total_min / 60;
      uint32_t total_minutes = s_daily_total_min % 60;
      lvgl_ui_set_pc_session_time(session_hours, session_minutes);
      lvgl_ui_set_pc_total_time(total_hours, total_minutes);
    }
  }
}

// Update runtime counters on the main UI.
void pc_status_update_runtime_ui(void) {
  time_t now = 0;
  struct tm timeinfo = {0};
  time(&now);
  localtime_r(&now, &timeinfo);

  int ymd = (timeinfo.tm_year + 1900) * 10000 + (timeinfo.tm_mon + 1) * 100 + timeinfo.tm_mday;
  if (s_daily_ymd == 0) {
    s_daily_ymd = ymd;
  } else if (ymd != s_daily_ymd) {
    s_daily_ymd = ymd;
    s_daily_total_min = 0;
  }

  int min_of_day = timeinfo.tm_hour * 60 + timeinfo.tm_min;
  if (s_last_tick_ymd != ymd || s_last_tick_min_of_day != min_of_day) {
    s_last_tick_ymd = ymd;
    s_last_tick_min_of_day = min_of_day;
    if (s_pc_on) {
      s_session_elapsed_min++;
      s_daily_total_min++;
    }
  }

  uint32_t session_hours = s_session_elapsed_min / 60;
  uint32_t session_minutes = s_session_elapsed_min % 60;
  uint32_t total_hours = s_daily_total_min / 60;
  uint32_t total_minutes = s_daily_total_min % 60;

  lvgl_ui_set_pc_session_time(session_hours, session_minutes);
  lvgl_ui_set_pc_total_time(total_hours, total_minutes);
}


// Log DNS server list and resolved A records.
static void log_dns_info(const char *host) {
  for (int i = 0; i < DNS_MAX_SERVERS; i++) {
    const ip_addr_t *dns = dns_getserver(i);
    if (dns && !ip_addr_isany(dns)) {
      char dns_str[IPADDR_STRLEN_MAX] = {0};
      ipaddr_ntoa_r(dns, dns_str, sizeof(dns_str));
      ESP_LOGI(TAG, "DNS server %d: %s", i, dns_str);
    }
  }

  struct addrinfo hints = {0};
  struct addrinfo *res = NULL;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  int err = getaddrinfo(host, NULL, &hints, &res);
  if (err != 0 || res == NULL) {
    ESP_LOGE(TAG, "DNS lookup failed for %s: %d", host, err);
    return;
  }

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

// HTTP client event handler to collect response body.
static void parse_follower_stream(http_resp_t *resp, const char *data,
                                  size_t len) {
  static const char key[] = "\"follower_count\":";
  const int key_len = (int)(sizeof(key) - 1);

  for (size_t i = 0; i < len; i++) {
    char c = data[i];
    if (resp->follower_done) {
      return;
    }

    if (!resp->value_active) {
      if (c == key[resp->key_match]) {
        resp->key_match++;
        if (resp->key_match == key_len) {
          resp->value_active = true;
          resp->parsing_digits = false;
          resp->follower_count = 0;
          resp->key_match = 0;
        }
      } else {
        resp->key_match = (c == key[0]) ? 1 : 0;
      }
      continue;
    }

    if (!resp->parsing_digits) {
      if (c == ' ' || c == '\t' || c == '"') {
        continue;
      }
      if (c >= '0' && c <= '9') {
        resp->parsing_digits = true;
        resp->follower_count = (uint32_t)(c - '0');
      } else {
        resp->value_active = false;
      }
      continue;
    }

    if (c >= '0' && c <= '9') {
      resp->follower_count =
          resp->follower_count * 10 + (uint32_t)(c - '0');
    } else {
      resp->follower_done = true;
      resp->value_active = false;
      resp->parsing_digits = false;
      return;
    }
  }
}

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
      if (resp->parse_follower) {
        parse_follower_stream(resp, (const char *)evt->data,
                              (size_t)evt->data_len);
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

// Perform a single HTTPS poll and update UI.
// Fetch PC online/power status once and update UI.
void pc_status_poll_once(void) {
  char *response_buffer = (char *)calloc(MAX_HTTP_OUTPUT_BUFFER + 1, 1);
  if (response_buffer == NULL) {
    ESP_LOGW(TAG, "response buffer alloc failed");
    return;
  }
  http_resp_t resp = {
      .buf = response_buffer,
      .len = 0,
      .cap = MAX_HTTP_OUTPUT_BUFFER + 1,
      .parse_follower = false,
      .follower_done = false,
      .follower_count = 0,
      .key_match = 0,
      .value_active = false,
      .parsing_digits = false,
  };

  if (!wifi_wait_connected(pdMS_TO_TICKS(5000))) {
    ESP_LOGW(TAG, "wifi not connected, skip http");
    free(response_buffer);
    return;
  }

  if (PC_STATUS_DEBUG_DNS) {
    log_dns_info(PC_STATUS_HOST);
  }
  esp_http_client_config_t cfg = {
      .host = PC_STATUS_HOST,
      .path = PC_STATUS_PATH,
      .query = PC_STATUS_QUERY,
      .transport_type = HTTP_TRANSPORT_OVER_SSL,
      .event_handler = http_event_handler,
      .user_data = &resp,
      .cert_pem = howsmyssl_com_root_cert_pem_start,
      .buffer_size = HTTP_CLIENT_RX_BUFFER,
      .buffer_size_tx = HTTP_CLIENT_TX_BUFFER,
      .timeout_ms = HTTPS_TIMEOUT_MS,
  };

  ESP_LOGI(TAG, "HTTPS GET https://%s%s?action=status&token=...", PC_STATUS_HOST,
           PC_STATUS_PATH);
  if (PC_STATUS_DEBUG_HEAP) {
    log_heap_state("pc_before_init");
    log_heap_state("pc_before_perform");
  }
  esp_err_t err = http_perform_with_retry(&cfg, NULL, "pc_status");
  if (PC_STATUS_DEBUG_HEAP) {
    log_heap_state("pc_after_perform");
  }

  if (err != ESP_OK && response_buffer[0] == '\0') {
    ESP_LOGW(TAG, "pc status empty response");
    free(response_buffer);
    return;
  }

  if (response_buffer[0] == '\0') {
    ESP_LOGW(TAG, "pc status empty response");
    free(response_buffer);
    return;
  }

  ESP_LOGI(TAG, "pc status resp: %s", response_buffer);

  pc_state_t state = parse_pc_state(response_buffer);
  if (state == PC_STATE_UNKNOWN) {
    ESP_LOGW(TAG, "pc status unknown, resp: %s", response_buffer);
    free(response_buffer);
    return;
  }

  apply_pc_state(state);
  free(response_buffer);
}

// Extract follower_count from response JSON.
static bool parse_follower_count(const char *buf, uint32_t *out_count) {
  const char *key = "\"follower_count\":";
  const char *p = strstr(buf, key);
  if (!p) {
    // Try escaped JSON string: \"follower_count\":
    const char *ekey = "\\\"follower_count\\\":";
    p = strstr(buf, ekey);
    if (!p) {
      return false;
    }
    p += strlen(ekey);
    while (*p == ' ' || *p == '\t') {
      p++;
    }
    if (*p == '\"') {
      p++;
    }
    unsigned long val = 0;
    bool any = false;
    while (*p >= '0' && *p <= '9') {
      any = true;
      val = val * 10 + (unsigned long)(*p - '0');
      p++;
    }
    if (!any) {
      return false;
    }
    if (out_count) {
      *out_count = (uint32_t)val;
    }
    return true;
  }
  p += strlen(key);
  while (*p == ' ' || *p == '\t') {
    p++;
  }
  if (*p == '"') {
    p++;
  }
  unsigned long val = 0;
  bool any = false;
  while (*p >= '0' && *p <= '9') {
    any = true;
    val = val * 10 + (unsigned long)(*p - '0');
    p++;
  }
  if (!any) {
    return false;
  }
  if (out_count) {
    *out_count = (uint32_t)val;
  }
  return true;
}

// Fetch Douyin user profile and update follower count on UI.
static uint32_t s_fans_attempts = 0;
static bool s_fans_updated_since_last_check = false;
static int s_fans_last_ymd = 0;

static int fans_get_ymd_now(void) {
  time_t now = 0;
  struct tm timeinfo = {0};
  time(&now);
  localtime_r(&now, &timeinfo);
  if (timeinfo.tm_year < 120) {  // year < 2020 => time not set
    return 0;
  }
  return pc_calc_ymd(&timeinfo);
}

// Send power-on command to the PC control API.
bool pc_status_send_power_on(void) {
  if (!wifi_is_connected()) {
    ESP_LOGW(TAG, "wifi not connected, skip power on");
    return false;
  }

  char *response_buffer = (char *)calloc(256, 1);
  if (!response_buffer) {
    ESP_LOGW(TAG, "power on response alloc failed");
    return false;
  }
  http_resp_t resp = {
      .buf = response_buffer,
      .len = 0,
      .cap = 256,
      .parse_follower = false,
      .follower_done = false,
      .follower_count = 0,
      .key_match = 0,
      .value_active = false,
      .parsing_digits = false,
  };

  esp_http_client_config_t cfg = {
      .host = PC_STATUS_HOST,
      .path = PC_STATUS_PATH,
      .query = PC_POWER_ON_QUERY,
      .transport_type = HTTP_TRANSPORT_OVER_SSL,
      .event_handler = http_event_handler,
      .user_data = &resp,
      .cert_pem = howsmyssl_com_root_cert_pem_start,
      .buffer_size = HTTP_CLIENT_RX_BUFFER,
      .buffer_size_tx = HTTP_CLIENT_TX_BUFFER,
      .timeout_ms = HTTPS_TIMEOUT_MS,
  };

  ESP_LOGI(TAG, "HTTPS GET https://%s%s?action=on&token=...", PC_STATUS_HOST,
           PC_STATUS_PATH);
  esp_err_t err = http_perform_with_retry(&cfg, NULL, "pc_power_on");
  if (err != ESP_OK) {
    ESP_LOGW(TAG, "power on request failed");
    free(response_buffer);
    return false;
  }
  free(response_buffer);
  return true;
}

// Check if fans data should be refreshed (new day or failed before).
bool douyin_fans_need_retry(void) {
  int ymd = fans_get_ymd_now();
  if (ymd == 0) {
    return true;
  }
  return s_fans_last_ymd != ymd;
}

// Return and clear the "fans updated" flag.
bool douyin_fans_updated_take(void) {
  bool updated = s_fans_updated_since_last_check;
  s_fans_updated_since_last_check = false;
  return updated;
}

// Force a re-fetch on next wake cycle.
void douyin_fans_force_retry(void) {
  s_fans_last_ymd = 0;
  s_fans_attempts = 0;
}

static void update_fans_symbol(const char *symbol) {
  lvgl_ui_set_fan_update_symbol(symbol);
}

static void log_heap_state(const char *stage) {
  size_t free_heap = esp_get_free_heap_size();
  size_t min_free_heap = esp_get_minimum_free_heap_size();
  size_t free_internal = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
  size_t free_dma = heap_caps_get_free_size(MALLOC_CAP_DMA);
  ESP_LOGI(TAG, "heap[%s]: free=%u min=%u internal=%u dma=%u",
           stage, (unsigned)free_heap, (unsigned)min_free_heap,
           (unsigned)free_internal, (unsigned)free_dma);
}

static esp_err_t http_perform_with_retry(esp_http_client_config_t *cfg,
                                         const char *auth_header,
                                         const char *tag_prefix) {
  esp_err_t err = ESP_FAIL;
  for (int attempt = 0; attempt < 2; attempt++) {
    if (attempt > 0) {
      ESP_LOGW(TAG, "%s retry attempt %d", tag_prefix, attempt + 1);
      vTaskDelay(pdMS_TO_TICKS(500));
    }

    esp_http_client_handle_t client = esp_http_client_init(cfg);
    if (!client) {
      ESP_LOGW(TAG, "%s http client init failed", tag_prefix);
      continue;
    }
    if (auth_header) {
      esp_http_client_set_header(client, "Authorization", auth_header);
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

// Fetch Douyin follower count once and update UI.
void douyin_fans_poll_once(void) {
#if !DOUYIN_FANS_ENABLED
  // Disabled to avoid charge while debugging.
  int ymd_now = fans_get_ymd_now();
  if (ymd_now != 0) {
    s_fans_last_ymd = ymd_now;
  }
  s_fans_updated_since_last_check = true;
  ESP_LOGW(TAG, "fans update disabled");
  return;
#endif
  int ymd = fans_get_ymd_now();
  if (ymd != 0 && s_fans_last_ymd == ymd) {
    ESP_LOGI(TAG, "fans update skipped (already done today)");
    return;
  }
  if (ymd != 0 && s_fans_last_ymd != ymd) {
    s_fans_attempts = 0;
  }
  s_fans_attempts++;
  if (s_fans_attempts > 10) {
    update_fans_symbol(LV_SYMBOL_LOOP);
  }

  char *response_buffer = (char *)calloc(512, 1);
  if (!response_buffer) {
    ESP_LOGW(TAG, "fans response alloc failed");
    return;
  }
  http_resp_t resp = {
      .buf = response_buffer,
      .len = 0,
      .cap = 512,
      .parse_follower = true,
      .follower_done = false,
      .follower_count = 0,
      .key_match = 0,
      .value_active = false,
      .parsing_digits = false,
  };

  if (!wifi_wait_connected(pdMS_TO_TICKS(5000))) {
    ESP_LOGW(TAG, "wifi not connected, skip fans");
    free(response_buffer);
    return;
  }

  if (PC_STATUS_DEBUG_DNS) {
    log_dns_info("api.tikhub.io");
  }
  const char *sec_user_id =
      "换成自己的";
  const char *api_token =
      "换成自己的";
  char query[256];
  snprintf(query, sizeof(query), "sec_user_id=%s", sec_user_id);

  esp_http_client_config_t cfg = {
      .host = "api.tikhub.io",
      .path = "/api/v1/douyin/app/v3/handler_user_profile",
      .query = query,
      .transport_type = HTTP_TRANSPORT_OVER_SSL,
      .event_handler = http_event_handler,
      .user_data = &resp,
      .cert_pem = tikhub_root_cert_pem_start,
      .buffer_size = HTTP_CLIENT_RX_BUFFER,
      .buffer_size_tx = HTTP_CLIENT_TX_BUFFER,
      .timeout_ms = HTTPS_TIMEOUT_MS,
  };

  ESP_LOGI(TAG, "HTTPS GET https://api.tikhub.io%s?sec_user_id=...",
           cfg.path);
  char auth_header[192];
  snprintf(auth_header, sizeof(auth_header), "Bearer %s", api_token);
  if (PC_STATUS_DEBUG_HEAP) {
    log_heap_state("fans_before_init");
    log_heap_state("fans_before_perform");
  }
  esp_err_t err = http_perform_with_retry(&cfg, auth_header, "fans");
  if (PC_STATUS_DEBUG_HEAP) {
    log_heap_state("fans_after_perform");
  }

  if (err != ESP_OK && response_buffer[0] == '\0') {
    ESP_LOGW(TAG, "follower_count not found, resp preview: ");
    free(response_buffer);
    return;
  }

  if (!resp.follower_done && resp.parsing_digits) {
    resp.follower_done = true;
  }

  if (!resp.follower_done) {
    char preview[257];
    size_t preview_len = strlen(response_buffer);
    if (preview_len > 256) {
      preview_len = 256;
    }
    memcpy(preview, response_buffer, preview_len);
    preview[preview_len] = '\0';
    ESP_LOGW(TAG, "follower_count not found, resp preview: %s", preview);
    free(response_buffer);
    return;
  }

  uint32_t follower_count = resp.follower_count;

  lvgl_ui_set_fans_count(follower_count);

  ESP_LOGI(TAG, "fans count: %u", (unsigned)follower_count);
  update_fans_symbol(LV_SYMBOL_VIDEO);
  s_fans_updated_since_last_check = true;
  if (ymd != 0) {
    s_fans_last_ymd = ymd;
  }
  free(response_buffer);
}
