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

| 序号 |                              | 数量 | 价格 |
| ---- | :--------------------------- | ---- | ---- |
| 1    | AS312                        | 1    | 1.75 |
| 2    | ESP32-C3-WROOM-02-N4         | 1    | 7.8  |
| 3    | 鱼鹰光电4.2寸全反射TFT单色屏 | 1    | 43   |

#### 4、焊接

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