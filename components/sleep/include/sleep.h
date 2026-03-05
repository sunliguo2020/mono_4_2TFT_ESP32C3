#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*sleep_update_cb_t)(void);

// Create the sleep task (idempotent).
void sleep_init(void);
// Start the light-sleep loop.
void sleep_start_light(void);

// Hooks for future expansion.
void sleep_register_http_update_cb(sleep_update_cb_t cb);
void sleep_register_battery_update_cb(sleep_update_cb_t cb);
void sleep_register_state_update_cb(sleep_update_cb_t cb);
// Notify IO5 wake so the power-on request can be executed.
void sleep_notify_io5_wake(void);

#ifdef __cplusplus
}
#endif
