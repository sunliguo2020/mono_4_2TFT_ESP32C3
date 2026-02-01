#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*sleep_update_cb_t)(void);

void sleep_init(void);
void sleep_start_light(void);

// Hooks for future expansion
void sleep_register_http_update_cb(sleep_update_cb_t cb);
void sleep_register_battery_update_cb(sleep_update_cb_t cb);
void sleep_register_state_update_cb(sleep_update_cb_t cb);

#ifdef __cplusplus
}
#endif
