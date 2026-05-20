#ifndef __MOTION_ELBOW_H
#define __MOTION_ELBOW_H

#include "sys.h"
#include "timer2.h"

// 动作2：肘关节弯曲
// 返回值：0=完成，1=执行中
int motion_elbow_step(void);

// 立即停止动作（供 motions.c 调用）
void motion_elbow_stop(void);

#endif
