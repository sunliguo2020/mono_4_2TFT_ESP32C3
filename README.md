## ESP32C3_4.2寸鱼鹰全反射类墨水屏

原作者 沙发下的土豆  来自于嘉立创开源广场 [esp32c3_4.2寸鱼鹰全反射类墨水屏]( https://oshwhub.com/bob_yang/esp32c3_4-2-cun-lei-mo-shui-ping )

以下摘自开源广场介绍：

## 项目简介

- 学习ESP32C3低功耗控制，编译环境为ESP-IDF，屏幕为鱼鹰全反射类墨水屏，SPI通讯，可设置低功耗与睡眠模式。
- 学习LVGL的UI设计，使用GUIguider快速布局UI界面。
- 学习http通讯，控制电脑开关机，查询电脑开关机状态，学习通过api查询抖音粉丝数。
- 学习ntp网络校时。
- 学习OPT3001光照传感器，中断唤醒deepsleep。
- 学习人体红外AS312传感器，控制有人存在时更新UI界面。
- 学习使用AHT30温湿度传感器，与opt3001共用i2c总线。
- 学习使用TP4056进行锂电池充电管理，使用DW03进行电池充放电保护。
- 学习ADC测量电池电量，通过PMOS控制电池检测。
- 使用ME6210A33PG低功耗LDO，直接将3.7v电压降到3.3V
- 通过按键进行lightleep唤醒，并使用api将电脑主机开机。

## 项目功能

本设计是基于ESP32C3单片机设计的DIY功能板，系统在光照低于设置阈值时，会进行睡眠模式，此时ESP32C3进入deepsleep模式，adc采集停止，恢复光照时会进行deepsleep唤醒，白天进入lightsleep，每分钟采集一次温湿度，当人体红外检测室内有人时，会进行wifi的一些业务逻辑更新；设置有两个独立按键，功能分别为远程将电脑开机和复位按键。

## 项目参数

- 本设计采用ESP32C3可以进行wifi通讯，有两种低功耗睡眠模模式。
- 本设计采用鱼鹰全反射屏，屏幕没有背光，屏幕有低功耗和睡眠模式，使用功耗更低，刷新频率可以到16HZ。
- 选用全数字型温湿度传感器AHT30，具有较宽的温度测量范围，可以更准确的显示室内的温湿度；
- 选用OPT3001光照传感器，可以通过寄存器设置阈值，使中断引脚根据设置阈值进行高低电平切换，从而控制低功耗的唤醒；
- AS312人体红外传感器，可以通过红外判断室内有没有人存在，如果没有人存在，可以跳过wifi的一些业务逻辑，可以达到降低功耗的目的；
- 使用TP4056进行充电，充电电流可以达到1A,并带有DW03进行电池保护；
- 通过PMOS控制ADC采集的通断，减少ADC电池电压不采集时的漏电流；
- 通过一个ME6210A33PG的LDO进行3.3v降压供电，供电电流500ma，低压降LDO；

## 复刻过程

#### 1、将pcb中所有的0402封装更换为0603封装

注意到这个pcb空间较为宽裕，所以将上面原来0402封装的元件都改为0603封装。主要是以前准备的电容电阻都是0603封装的。

#### 2、嘉立创打板

#### 3、淘宝购买元件

| 序号 |                                               | 数量 | 价格 |
| ---- | :-------------------------------------------: | ---- | ---- |
| 1    |                     AS312                     | 1    | 1.75 |
| 2    |             ESP32-C3-WROOM-02-N4              | 1    | 7.8  |
| 3    | 鱼鹰光电4.2寸全反射TFT单色屏300*400无背光16Hz | 1    | 43   |

#### 4、焊接

主要是AHT30温湿度 和OPT3001光照 这个两个元件比较难焊接。可以使用加热台先焊接这两个元件。

pfc底座也是一个难点。

#### 5、软件部分

- 安装esp-idf环境
  - 使用ESP-IDF Installation Manager （EIM）离线安装

- vscode安装esp-idf插件

错误1：

```
Python binary path not found. Please check your Python configuration.
```

错误2：

```
The "path" argument must be of type string. Received undefined
```

错误3：

```
Error while getting targets from ESP-IDF: Error: File not found: C:\Users\sunliguo\esp\esp-idf\tools\idf_py_actions\constants.py
```

 最后发现是在C:\Espressif\tools下的 Microsoft.v6.0.1.PowerShell_profile.ps1 有语法错误。 

文件：C:\Espressif\tools\Microsoft.v6.0.1.PowerShell_profile.ps1

把：$previous = $env:’_IDF.PY_COMPLETE’
改成：$previous = ${env:_IDF.PY_COMPLETE}

把：$env:’_IDF.PY_COMPLETE’ = ‘powershell_source’
改成：${env:_IDF.PY_COMPLETE} = ‘powershell_source’

把：$env:’_IDF.PY_COMPLETE’ = $previous
改成：${env:_IDF.PY_COMPLETE} = $previous

#### 6、编译烧录

修改wifi名和密码，pc开机的密钥 ，抖音获取的粉丝数的参数

烧录的时候，需要按下重置按钮。

### 7、字体补全

如果屏幕显示中出现方框（中文字体缺失），说明 GUI Guider 生成的字体文件未包含这些字符。**三种修复方法**：

#### 方法一：GUI Guider 中重新生成

1. 打开 GUI Guider 项目文件
2. 在 UI 设计界面中，找到对应的文本输入框
3. 输入完整的中文文本（如"今日天气"、"明日天气"）
4. 点击"Generate Code"重新生成代码
5. 重新编译烧录即可

#### 方法二：命令行工具 lv_font_conv

```
# 从字体文件中提取并添加缺失字符
lv_font_conv --font SourceHanSansSC-Bold.otf --size 24 \
  --font FontAwesome5-Solid+Brands+Regular.woff --size 16 --format lvgl \
  --symbols "今日天气明℃" \
  --output lv_font_weather_24.c
```

#### 方法三：代码中改用英文字体（快速解决方案）

如果只是少数几个中文字符需要显示，可以直接在代码中将字体改为 `&lv_font_montserratMedium_16`（英文字体，始终可用），同时将文本改为英文：

```c
lv_label_set_text(ui->screen_main_label_5, "W\nE\nA\nT\nH\nE\nR");
lv_obj_set_style_text_font(ui->screen_main_label_5, &lv_font_montserratMedium_16, 0);
```


## 屏幕布局

本设备使用 4.2 寸鱼鹰全反射类墨水屏，分辨率 300×400 像素。主屏幕布局如下：

有效显示宽度290像素

```
┌──────────────────────────────────────┐
│  ┌─ 顶部栏 (y=4, 290x52) ──────────┐ │
│  │ WiFi图标(5,1)       光照图标     │ │
│  │                    (135,2)       │ │
│  │ 人体存在图标(170,4) 电池符号     │ │
│  │                   (221,4)        │ │
│  │ WiFi文字  光照值(135,28)          │ │
│  │ (5,32)    电池百分比(221,28)     │ │
│  └──────────────────────────────────┘ │
│                                       │
│  ┌─ 时间区域 (y=61, 289x112) ──────┐ │
│  │  日期 (居中, 字体SMG_32)    ↻   │ │
│  │  ┌──────────────────────────┐    │ │
│  │  │  88  :  88               │    │ │
│  │  │ (字体SMG_90)             │    │ │
│  │  └──────────────────────────┘    │ │
│  └──────────────────────────────────┘ │  ← 间距 5px
│  ┌─ 温湿度 (y=178, 290x65) ──────┐ │
│  │ [温度] 25.5  │  [湿度] 60.3    │ │
│  │ (竖排带边框) │ (竖排带边框)    │ │
│  │ (字体SMG_40) │ (字体SMG_40)    │ │
│  └──────────────────────────────┘ │  ← 间距 5px
│  ┌─ 天气 (y=248, 290x152) ───────┐ │
│  │  天气描述 / 温度 / 湿度         │ │
│  │  (多标签重叠动态显示)           │ │
│  └──────────────────────────────┘ │
│                                       │
│  ┌─ 睡眠屏幕 ───────────────────────┐ │
│  │  背景图片(sleep2_RGB565)         │ │
│  │  文字: "苟命要紧"(右上角)        │ │
│  └──────────────────────────────────┘ │
└──────────────────────────────────────┘
```

### 各区域详细说明

| 区域 | 容器 | 位置 (x,y) | 大小 (w×h) | 内容 |
|------|------|-----------|-----------|------|
| 顶部栏 | `screen_main_cont_top_bar` | (5, 4) | 290×52 | WiFi图标/文字、光照图标/值、人体存在图标、电池符号/百分比 |
| 时间区域 | `screen_main_cont_time` | (5, 61) | 289×112 | 日期、时间(时:分)、时间同步符号(↻) |
| 温湿度 | `screen_main_cont_temp_humi` | (5, 178) | 290×65 | 温度标签/数值（左）、湿度标签/数值（右） |
| 天气 | `screen_main_cont_bottom` | (5, 248) | 290×152 | 天气描述/温度/湿度（多标签重叠动态切换，占满底部） |
| 睡眠屏幕 | `screen_sleep` | 全屏 | 300×400 | 睡眠背景图 + "苟命要紧"标签 |

### 顶部栏详细布局

**第一行 (y=0~25):**
- WiFi图标 `screen_main_img_1`: (5, 1), 33×31, 图片资源 `_wifi_down_I4_33x31`(离线) / `_wifi_fill_I4_33x31`(在线)
- 光照图标 `screen_main_img_2`: (135, 2), 29×25, 图片资源 `_light_val_I4_29x25`
- 人体存在图标 `screen_main_img_5`: (170, 4), 36×37, 图片资源 `_Human_Presence_Sensor_RGB565A8_36x37`（有人时显示，无人时隐藏）
- 电池符号 `screen_main_label_17`: (221, 4), 52×22, LVGL电池图标符号，根据电量动态切换（EMPTY/1/2/3/FULL）

**第二行 (y=26~50):**
- WiFi文字 `screen_main_label_wifi`: (5, 32), 33×12, 居中显示"WiFi", 字体 montserratMedium_12
- 光照值 `screen_main_label_light_val`: (135, 28), 60×14, 字体 montserratMedium_16
- 电池百分比 `screen_main_label_bat_val`: (221, 28), 52×14, 右对齐, 字体 montserratMedium_16, 格式"100%"

### 时间区域详细布局

区域容器 `screen_main_cont_time`: (5, 61), 289×112, 白底圆角边框

- 日期标签 `screen_main_label_date`: (0, 1), 233×28, 字体 SMG_32, 居中对齐, 格式"2026-01-30"
- 时间同步符号 `screen_main_label_time_update`: (241, 4), 30×23, 字体 SourceHanSansSCBold_24, 显示 LV_SYMBOL_LOOP(↻)
- 子容器 `screen_main_cont_2`: (2, 32), 281×89, 仅顶部边框
  - 小时 `screen_main_label_time_hour`: (-1, -10), 140×85, 字体 SMG_90, 左对齐
  - 冒号 `screen_main_label_1`: (131, -20), 22×82, 字体 SMG_90, 居中对齐
  - 分钟 `screen_main_label_time_min`: (150, -10), 136×82, 字体 SMG_90, 居中对齐

> 注：`screen_main_label_time_update` 显示时间同步状态，同步中显示 ↻，可改为 ✅ 表示同步完成。

### 温湿度区域详细布局

区域容器 `screen_main_cont_temp_humi`: (5, 178), 290×65, 白底圆角边框

温湿度区域采用**左右平分布局**：左侧显示温度，右侧显示湿度。每侧包含竖排标签（带黑色边框圆角）和数值。

**左侧温度区域（0~145）：**
- 温度标签 `screen_main_label_10`: (5, 5), 30×50, 竖排文本"温\n度", 字体 SourceHanSansSCBold_20, 带黑色边框圆角, 居中对齐
- 温度值 `screen_main_label_temp_val`: (38, 8), 120×48, 字体 SMG_40, 格式"25.5", 初始值"0.0"

**右侧湿度区域（145~290）：**
- 湿度标签 `screen_main_label_8`: (155, 5), 30×50, 竖排文本"湿\n度", 字体 SourceHanSansSCBold_20, 带黑色边框圆角, 居中对齐
- 湿度值 `screen_main_label_humi_val`: (188, 8), 120×48, 字体 SMG_40, 格式"60.3", 初始值"0.0"

### 天气区域详细布局

区域容器 `screen_main_cont_bottom`: (5, 248), 290×152, 白底圆角边框

天气区域占据屏幕最底部，全宽显示。使用多个位置重叠的标签，通过 `UI_MSG_WEATHER` 消息动态控制显示：

| 标签 | 位置 (x,y) | 大小 (w×h) | 字体 | 用途 |
|------|-----------|-----------|------|------|
| `screen_main_label_weather` | (5, 5) | 97×108 | montserratMedium_16 | 主标签（显示天气图标） |
| `screen_main_label_5` | (0, 0) | 107×118 | SourceHanSansSCBold_24 | 天气描述文本（如"晴"） |
| `screen_main_label_20` | (0, 0) | 107×118 | SourceHanSansSCBold_24 | 温度（如"28°C"） |
| `screen_main_label_run_hour` | (0, 0) | 107×118 | SourceHanSansSCBold_24 | 湿度（如"60%"） |
| `screen_main_label_19` | (0, 0) | 107×118 | SourceHanSansSCBold_24 | 备用 |
| `screen_main_label_18` | (0, 0) | 107×118 | SourceHanSansSCBold_24 | 备用 |
| `screen_main_label_run_min` | (0, 0) | 107×118 | SourceHanSansSCBold_24 | 备用 |
| `screen_main_label_2` | (0, 0) | 107×118 | SourceHanSansSCBold_24 | 备用 |

### 睡眠屏幕详细布局

| 控件 | 位置 (x,y) | 大小 (w×h) | 内容 |
|------|-----------|-----------|------|
| `screen_sleep_img_1` | (0, 0) | 301×399 | 睡眠背景图 `_sleep2_RGB565_301x399` |
| `screen_sleep_label_1` | (189, 6) | 37×108 | 文本"苟命要紧"，带黑色边框圆角，字体 SourceHanSansSCBold_24 |

### UI 消息系统（动态更新）

`lvgl_ui.c` 通过消息队列（`ui_msg_t`）在后台任务中更新 UI：

| 消息类型 | 函数接口 | 更新内容 |
|---------|---------|---------|
| `UI_MSG_WIFI_ONLINE` | `wifi_online()` | WiFi图标切换为在线图标 `_wifi_fill_I4_33x31` |
| `UI_MSG_WIFI_OFFLINE` | `wifi_downline()` | WiFi图标切换为离线图标 `_wifi_down_I4_33x31` |
| `UI_MSG_TIME_LABELS` | `lvgl_ui_set_time_labels()` | 更新日期、时、分文本 |
| `UI_MSG_TIME_SYMBOL` | `lvgl_ui_set_time_update_symbol()` | 更新时间同步符号（↻ 等） |
| `UI_MSG_TEMP_HUMI` | `lvgl_ui_set_temp_humi()` | 更新温湿度数值（保留1位小数） |
| `UI_MSG_LIGHT_LUX` | `lvgl_ui_set_light_lux()` | 更新光照强度数值（lux） |
| `UI_MSG_BATTERY` | `lvgl_ui_set_battery()` | 更新电池百分比和电池图标（EMPTY/1/2/3/FULL） |
| `UI_MSG_PRESENCE` | `lvgl_ui_set_presence()` | 人体存在：显示/隐藏人体存在图标 |
| `UI_MSG_WEATHER` | `lvgl_ui_set_weather()` | 更新天气描述、温度、湿度/图标 |
| `UI_MSG_SLEEP_SCREEN` | `lvgl_ui_show_sleep_screen()` | 切换主屏/睡眠屏 |

## 改进过程

### 1、蓝牙配网问题

原来的wifi名和密码是写死的，我想改成第一次蓝牙配网模式。

#### 配网流程

##### 1、启动流程

```
app_main()
  ├─ NVS 初始化
  ├─ LVGL 初始化（显示 GUI）
  ├─ 传感器初始化（AHT30、OPT3001、BATTERY、AS312）
  └─ 主循环（每 100ms 运行一次状态机）
```

##### 2、状态机流程

```
STATE_INIT
  ├─ 有 WiFi 凭据？→ STATE_WIFI_CONNECTING
  └─ 无 WiFi 凭据？→ STATE_BLE_PROVISIONING

STATE_WIFI_CONNECTING
  ├─ WiFi 已连接？→ STATE_WIFI_CONNECTED（调用 on_wifi_connected）
  └─ 30秒超时？→ STATE_BLE_PROVISIONING

STATE_BLE_PROVISIONING
  ├─ 启动 BLE 广播（设备名: "ESP32-C3-WiFi"）
  ├─ 等待手机发送数据（格式: ssid:xxx,pwd:xxx）
  ├─ 收到数据后累积到缓冲区
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

##### 3、手机配网操作

```
1. 设备启动 → 无 WiFi 凭据 → 自动进入 BLE 配网模式
2. 手机打开 BLE 扫描 → 找到 "ESP32-C3-WiFi"
3. 手机连接设备 → 找到特征值 UUID 0xFFE1
4. 手机写入数据（分两次发送）:
   第一次: "ssid:rizhixinxi,pwd:"
   第二次: "ruizhi2016"
5. 设备累积接收 → 解析成功 → 保存到 NVS
6. 设备关闭 BLE → 连接 WiFi
7. WiFi 连接成功 → 更新图标 → 开始正常工作

```

##### 4、关键文件

| 文件                             | 功能                            |
| -------------------------------- | ------------------------------- |
| main/main.c                      | 主循环、状态机、配网完成回调    |
| components/ble_wifi/ble_wifi.c   | BLE 服务、数据接收、凭据解析    |
| components/Wifi_hw_Y/Wifi_hw_Y.c | WiFi 初始化、事件处理、NVS 存储 |
