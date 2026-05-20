#ifndef __MOTIONS_H
#define __MOTIONS_H

#include "sys.h"

// 串口命令定义
#define CMD_STOP        0x00    // 停止所有动作
#define CMD_DRINKING    0x01    // 动作1：抬手喝水
#define CMD_ELBOW       0x02    // 动作2：肘关节弯曲
#define CMD_FORWARD     0x03    // 前进：轮子前进2秒
#define CMD_BACKWARD    0x04    // 后退：轮子后退2秒

// 当前运动中的动作（0=无，1=drinking，2=elbow）
extern volatile u8 current_motion;

// 动作步进（非阻塞，每10ms在主循环调用一次）
void motions_step(void);

// 立即停止所有动作（发送急停SDO）
void motion_stop_all(void);

#endif
