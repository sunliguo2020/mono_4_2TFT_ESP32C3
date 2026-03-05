#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Poll PC online/power status and update UI.
void pc_status_poll_once(void);
// Poll Douyin fans count and update UI (if due).
void douyin_fans_poll_once(void);
// Return whether the next fans update should retry.
bool douyin_fans_need_retry(void);
// Return and clear the "fans updated" flag.
bool douyin_fans_updated_take(void);
// Force a retry on the next wake cycle.
void douyin_fans_force_retry(void);
// Update runtime counters UI.
void pc_status_update_runtime_ui(void);
// Send the power-on HTTP command.
bool pc_status_send_power_on(void);

#ifdef __cplusplus
}
#endif
