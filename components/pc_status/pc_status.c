#include "pc_status.h"

#include <inttypes.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "Wifi_hw_Y.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "esp_tls.h"
#include "lvgl_display.h"
#include "lwip/dns.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"

#define TAG "pc_status"
#define MAX_HTTP_OUTPUT_BUFFER 2048
#define HTTPS_TIMEOUT_MS 30000
#define PC_STATUS_NO_UI 0

#define PC_STATUS_HOST "open.iot.vlinkc.com"
#define PC_STATUS_PATH "/device/control"
#define PC_STATUS_QUERY                                                   \
  "action=status&token="                                                  \
  "R1JTWWNsTk9ldlhCZFBxUjhhOUsyeE1Qck5nZ3V0YU5KbkM4bkxlTjhoaHd5aUF2"      \
  "M3FNOUVuaVBZVlNCcWhmeVpWTzJkYWRQOE1Gcjh3WmFsSDdMcFE3Y2haS2owRFl5"      \
  "cVFOaVE3aGlzZzA9"

/* Root cert for open.iot.vlinkc.com, taken from howsmyssl_com_root_cert.pem. */
extern const char howsmyssl_com_root_cert_pem_start[]
    asm("_binary_howsmyssl_com_root_cert_pem_start");
extern const char howsmyssl_com_root_cert_pem_end[]
    asm("_binary_howsmyssl_com_root_cert_pem_end");

typedef enum {
  PC_STATE_UNKNOWN = 0,
  PC_STATE_ON,
  PC_STATE_OFF,
  PC_STATE_OFFLINE
} pc_state_t;

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

static void apply_pc_state(pc_state_t state) {
  if (state == PC_STATE_OFFLINE) {
    ESP_LOGW(TAG, "pc status offline");
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

  if (!lvgl_port_lock(100)) {
    ESP_LOGW(TAG, "lvgl lock timeout, skip ui update");
    return;
  }
  if (state == PC_STATE_ON) {
    lvgl_ui_set_pc_state_image(true);
  } else if (state == PC_STATE_OFF) {
    lvgl_ui_set_pc_state_image(false);
  }
  lvgl_port_unlock();
}

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

static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
  static char *output_buffer;
  static int output_len;

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
      if (output_len == 0 && evt->user_data) {
        memset(evt->user_data, 0, MAX_HTTP_OUTPUT_BUFFER);
      }
      if (!esp_http_client_is_chunked_response(evt->client)) {
        int copy_len = 0;
        if (evt->user_data) {
          copy_len = MIN(evt->data_len, (MAX_HTTP_OUTPUT_BUFFER - output_len));
          if (copy_len) {
            memcpy(evt->user_data + output_len, evt->data, copy_len);
          }
        } else {
          int content_len = esp_http_client_get_content_length(evt->client);
          if (output_buffer == NULL) {
            output_buffer = (char *)calloc(content_len + 1, sizeof(char));
            output_len = 0;
            if (output_buffer == NULL) {
              ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
              return ESP_FAIL;
            }
          }
          copy_len = MIN(evt->data_len, (content_len - output_len));
          if (copy_len) {
            memcpy(output_buffer + output_len, evt->data, copy_len);
          }
        }
        output_len += copy_len;
      }
      break;
    }
    case HTTP_EVENT_ON_FINISH:
      if (output_buffer != NULL) {
        free(output_buffer);
        output_buffer = NULL;
      }
      output_len = 0;
      break;
    case HTTP_EVENT_DISCONNECTED: {
      int mbedtls_err = 0;
      esp_err_t err = esp_tls_get_and_clear_last_error(
          (esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
      if (err != 0) {
        ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
        ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
      }
      if (output_buffer != NULL) {
        free(output_buffer);
        output_buffer = NULL;
      }
      output_len = 0;
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

void pc_status_poll_once(void) {
  char *response_buffer = (char *)calloc(MAX_HTTP_OUTPUT_BUFFER + 1, 1);
  if (response_buffer == NULL) {
    ESP_LOGW(TAG, "response buffer alloc failed");
    return;
  }

  if (!wifi_wait_connected(pdMS_TO_TICKS(5000))) {
    ESP_LOGW(TAG, "wifi not connected, skip http");
    free(response_buffer);
    return;
  }

  log_dns_info(PC_STATUS_HOST);
  esp_http_client_config_t cfg = {
      .host = PC_STATUS_HOST,
      .path = PC_STATUS_PATH,
      .query = PC_STATUS_QUERY,
      .transport_type = HTTP_TRANSPORT_OVER_SSL,
      .event_handler = http_event_handler,
      .user_data = response_buffer,
      .cert_pem = howsmyssl_com_root_cert_pem_start,
      .timeout_ms = HTTPS_TIMEOUT_MS,
  };

  ESP_LOGI(TAG, "HTTPS GET https://%s%s?action=status&token=...", PC_STATUS_HOST,
           PC_STATUS_PATH);
  esp_http_client_handle_t client = esp_http_client_init(&cfg);
  if (client == NULL) {
    ESP_LOGW(TAG, "http client init failed");
    free(response_buffer);
    return;
  }

  esp_err_t err = esp_http_client_perform(client);
  if (err == ESP_OK) {
    ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %" PRId64,
             esp_http_client_get_status_code(client),
             esp_http_client_get_content_length(client));
  } else {
    ESP_LOGW(TAG, "http perform failed: %s", esp_err_to_name(err));
  }
  esp_http_client_cleanup(client);

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
