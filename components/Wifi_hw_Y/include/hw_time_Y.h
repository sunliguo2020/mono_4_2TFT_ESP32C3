#pragma once

#ifndef HW_TIME_TZ
#define HW_TIME_TZ "CST-8"
#endif

void hw_time_sync_start(void);
void hw_time_resync_if_due(void);
bool hw_time_is_synced(void);
bool hw_time_wait_for_sync(uint32_t timeout_ms);
void hw_time_set_default_if_unset(void);
