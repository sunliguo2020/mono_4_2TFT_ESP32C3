#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// 执行一次天气查询并更新 UI（同步，需要大量栈空间）
void weather_poll_once(void);

// 在独立高栈任务中执行天气查询，阻塞当前线程等待完成
void weather_poll_once_blocking(void);

// 检查是否需要更新天气
bool weather_need_update(void);

// 强制下次更新
void weather_force_update(void);

// 获取当前天气文本
const char* weather_get_text(void);

// 获取当前温度
int weather_get_temp(void);

// 获取体感温度
int weather_get_feels_like(void);

// 获取湿度
int weather_get_humidity(void);

// 数据是否有效
bool weather_is_valid(void);

// 将缓存的天气数据推送到 UI
void weather_apply_to_ui(void);

// 获取天气图标符号
const char* weather_get_icon_symbol(void);

// AT 指令需要的数据访问函数
int weather_get_temp_min(void);
int weather_get_temp_max(void);
const char* weather_get_wind_dir(void);
const char* weather_get_wind_scale(void);
const char* weather_get_sunrise(void);
const char* weather_get_text_tmrw(void);
int weather_get_temp_min_tmrw(void);
int weather_get_temp_max_tmrw(void);

#ifdef __cplusplus
}
#endif
