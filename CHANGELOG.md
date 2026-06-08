# 项目变更日志

## [2026-06-08] 和风天气获取与显示 + 温湿度显示修复 + 屏幕布局优化

### 1. 和风天气 API 集成

**涉及文件**：`components/weather/weather.c`、`components/weather/include/weather.h`

**修改内容**：
- 接入和风天气 API（`mg7fc3p9rj.re.qweatherapi.com`），获取实时天气数据
- 实现 gzip 解压：使用 miniz 库对压缩响应解压（纯 deflate 模式）
- 使用 `crt_bundle_attach` 全局证书包验证 TLS 连接
- 新增 `weather_poll_once_blocking()` 异步阻塞函数（信号量同步）
- 时间同步等待超时调整为 60 秒

### 2. 栈溢出修复

**涉及文件**：`components/weather/weather.c`、`main/main.c`

**问题根因**：TLS 握手需要大量栈空间（约 10-12KB），main 任务栈不足导致崩溃

**修复方案**：
- 将天气请求移至独立高栈任务（16384 字节），通过信号量实现异步阻塞
- 在 `app_main` 中调用 `weather_poll_once_blocking()` 等待任务完成

### 3. 初始化顺序优化

**涉及文件**：`main/main.c`

**修改内容**：调整 `app_main` 初始化顺序，解决 LVGL 初始化卡死问题
1. 传感器初始化（AHT30/OPT3001/BATTERY/AS312）— 不依赖屏幕
2. WiFi 连接（阻塞等待 STATE_WIFI_CONNECTED）
3. SNTP 时间同步（60 秒超时）
4. 天气请求（独立高栈任务）
5. LVGL 屏幕初始化
6. 推送传感器/天气数据到 UI
7. 启动 sleep 任务

### 4. 温湿度传感器数据显示修复

**涉及文件**：`components/AHT30_Y/AHT30_Y.c`、`components/GUI/generated/gui_guider.c`

**修复内容**：
- AHT30_Y.c：补充缺失的 `s_consecutive_failures` 和 `MAX_CONSECUTIVE_FAILURES` 声明
- gui_guider.c：`setup_ui()` 中添加 `setup_scr_screen_main_temp_humi()` 调用

### 5. 屏幕布局重构

**涉及文件**：`components/GUI/generated/setup_scr_screen_main_temp_humi.c`、`components/GUI/generated/setup_scr_screen_main_bottom.c`

**布局调整**：
- 温湿度区域改为**左右平分布局**（290×65）：
  - 左侧温度：竖排标签"温\n度"（30×50）+ SMG_40 数值（120×48）
  - 右侧湿度：竖排标签"湿\n度"（30×50）+ SMG_40 数值（120×48）
- 天气区域移至屏幕底部（5, 248, 290×152），全宽显示
- 各区域之间保留 5px 一致间距

### 6. 内存竞争修复

**涉及文件**：`components/sleep/sleep.c`、`main/main.c`

**修复内容**：
- 移除 sleep 任务中对 `weather_is_busy()` 的调用
- 天气请求在 sleep 任务启动前完成，两个高栈任务错开运行
- 移除 sleep.c 中 `sleep_step_enter_sleep()` 中重复的天气等待逻辑

### 7. 文档更新

**涉及文件**：`README.md`

**更新内容**：屏幕布局 ASCII 图、详细尺寸表、温湿度/天气区域详细说明

---

## [2026-05-27] BLE 配网 + WiFi 自动重连 + 电池 ADC 修正

### 1. BLE 配网功能（WiFi 失败后自动开启）

**涉及文件**：`main/main.c`、`components/ble_wifi/ble_wifi.c`、`components/ble_wifi/include/ble_wifi.h`

**修改内容**：
- 主循环改为**状态机**方式运行，每 100ms 轮询一次
- 新增状态：`STATE_INIT` → `STATE_WIFI_CONNECTING` → `STATE_BLE_PROVISIONING` → `STATE_PROVISIONING_DONE` → `STATE_WIFI_CONNECTED`
- WiFi 连接失败（30 秒超时）自动进入 BLE 配网模式
- BLE 配网完成后不立即重启，而是尝试连接 WiFi
- 配网成功后执行后续初始化（时间同步、睡眠任务、HTTP 更新）

**关键修复**：
- 使用 `volatile` 标志位替代事件队列，避免 NimBLE 任务中操作队列导致死锁
- BLE 数据分片接收：添加累积接收缓冲区，支持数据分多次写入特征值
- 结束符识别：手机发送的是字面 `\n`（`5c 6e`）而非换行符 `0x0A`，添加兼容处理

### 2. WiFi 连接优化

**涉及文件**：`components/Wifi_hw_Y/Wifi_hw_Y.c`、`components/Wifi_hw_Y/include/Wifi_hw_Y.h`

**修改内容**：
- 移除 `wifi_download_init()` 中的阻塞等待（`xEventGroupWaitBits` + `portMAX_DELAY`）
- 移除 `wifi_hw_init()` 中的重复 NVS 初始化（NVS 已在 `main.c` 中初始化）
- 添加 `wifi_wait_connected(TickType_t ticks_to_wait)` 非阻塞等待接口
- 添加 `wifi_save_to_nvs()` 和 `wifi_load_from_nvs()` 用于 BLE 配网保存/读取凭据
- 修复配网后 WiFi 连接崩溃：`esp_netif_create_default_wifi_sta()` 前先销毁旧的 netif
- 修复 WiFi 图标不更新：在 `IP_EVENT_STA_GOT_IP` 事件中调用 `wifi_online()`

### 3. BLE 初始化修复

**涉及文件**：`components/ble_wifi/ble_wifi.c`

**修改内容**：
- 启动 BLE 前先关闭 WiFi 驱动（`esp_wifi_deinit()`），解决 WiFi/BLE 射频冲突
- BLE 关闭后重新初始化 WiFi 驱动

### 4. 任务看门狗修复

**涉及文件**：`main/main.c`

**修改内容**：
- 主循环改用状态机非阻塞方式，移除所有阻塞等待
- 在 `sdkconfig` 中禁用任务看门狗（`CONFIG_ESP_TASK_WDT_EN=n`）

### 5. LVGL 初始化顺序修复

**涉及文件**：`main/main.c`

**修改内容**：
- 先初始化 LVGL 显示，再尝试 WiFi 连接
- 避免 WiFi 阻塞等待时 LVGL 未初始化导致 UI 更新崩溃

### 6. 电池 ADC 分压比修正

**涉及文件**：`components/BATTERY_Y/BATTERY_Y.c`

**修改内容**：
- 分压电路从 10k+22k 改为 100k+200k（实际焊接值）
- `BAT_DIV_NUM`: 32 → 300, `BAT_DIV_DEN`: 22 → 200
- `Vbat = Vadc * 300 / 200 = Vadc * 1.5`
- 添加详细的硬件连接注释（ADC 引脚、分压电路、MOSFET 控制、电量计算公式）
- 添加函数注释说明初始化、读取、控制流程

### 7. PC_INT_IO5 开机功能

**涉及文件**：`main/main.c`、`components/sleep/sleep.c`

**功能说明**：
- GPIO5 作为 PC 开机按钮，下降沿触发中断
- 按下 IO5 后发送 HTTP 请求唤醒 PC
- 支持从 light sleep 唤醒时检测 IO5 唤醒源

---

## 状态机流程图

```
app_main()
  ├─ NVS 初始化
  ├─ LVGL 初始化（显示 GUI）
  ├─ 传感器初始化（AHT30、OPT3001、BATTERY、AS312）
  └─ 主循环（每 100ms 运行一次状态机）

STATE_INIT
  ├─ 有 WiFi 凭据？→ STATE_WIFI_CONNECTING
  └─ 无 WiFi 凭据？→ STATE_BLE_PROVISIONING

STATE_WIFI_CONNECTING
  ├─ WiFi 已连接？→ STATE_WIFI_CONNECTED
  └─ 30秒超时？→ STATE_BLE_PROVISIONING

STATE_BLE_PROVISIONING
  ├─ 启动 BLE 广播（设备名: "ESP32-C3-WiFi"）
  ├─ 等待手机发送数据（格式: ssid:xxx,pwd:xxx）
  ├─ 解析成功？→ 保存到 NVS → 设置标志位
  └─ 主循环检测到标志位 → STATE_PROVISIONING_DONE

STATE_PROVISIONING_DONE
  ├─ 关闭 BLE
  ├─ 重新加载 WiFi 凭据
  ├─ 连接 WiFi
  ├─ WiFi 已连接？→ STATE_WIFI_CONNECTED
  └─ 30秒超时？→ STATE_PROVISIONING_WIFI_FAIL

STATE_PROVISIONING_WIFI_FAIL
  └─ 重新进入 BLE 配网模式

STATE_WIFI_CONNECTED
  ├─ 设置默认时间（非阻塞）
  ├─ 启动时间同步（后台任务）
  ├─ 注册 HTTP 更新回调（PC 状态、抖音粉丝）
  ├─ 启动睡眠灯光
  └─ 更新 WiFi 图标（wifi_online）
```

## 手机配网操作

```
1. 设备启动 → 无 WiFi 凭据 → 自动进入 BLE 配网模式
2. 手机打开 BLE 扫描 → 找到 "ESP32-C3-WiFi"
3. 手机连接设备 → 找到特征值 UUID 0xFFE1
4. 手机写入数据（分两次发送）:
   第一次: "ssid:rizhixinxi,pwd:ruizhi2016"
5. 设备累积接收 → 解析成功 → 保存到 NVS
6. 设备关闭 BLE → 连接 WiFi
7. WiFi 连接成功 → 更新图标 → 开始正常工作
