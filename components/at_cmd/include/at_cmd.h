#ifndef AT_CMD_H
#define AT_CMD_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化 AT 指令 UART 接收任务
 * 默认使用 UART1 (TX=GPIO6, RX=GPIO7, 115200bps)
 */
void at_cmd_init(void);

#ifdef __cplusplus
}
#endif

#endif /* AT_CMD_H */