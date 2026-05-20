#ifndef __WHEEL_H
#define __WHEEL_H

#include "sys.h"

// 轮子速度/时间配置（可在 main.c 覆盖）
#define WHEEL_SPEED       200     // 占空比速度 500=50.0%，范围-990~990
#define WHEEL_RUN_MS      2000    // 轮子运行时间2秒

#define WHEEL_STATE_IDLE  0
#define WHEEL_STATE_RUN   1

// 初始化（上电调用一次）
void wheel_init(void);

// 状态机（主循环每圈调用）
void wheel_step(void);

// 急停（供 motions.c 调用）
void wheel_emergency_stop(void);

// 命令处理（在 main.c 的 switch 里调用）
// 返回: 0=已处理, 1=轮子忙忽略
u8 wheel_cmd_handler(u8 cmd);

#endif
