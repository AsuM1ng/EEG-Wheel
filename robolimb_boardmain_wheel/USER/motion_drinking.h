#ifndef __MOTION_DRINKING_H
#define __MOTION_DRINKING_H

#include "sys.h"
#include "timer2.h"

// 动作1：抬手喝水
// 控制4个关节：肩ROLL(0x01)、肩YAW(0x02)、肩PITCH(0x03)、肘关节(0x04)
// 返回值：0=完成，1=执行中
int motion_drinking_step(void);

// 立即停止动作（供 motions.c 调用，发送 SDO_DISABLE）
void motion_drinking_stop(void);

#endif
