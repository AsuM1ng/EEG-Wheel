#ifndef __TIMER2_H
#define __TIMER2_H

#include "sys.h"
#include "main.h"

void TIM2_Init(void);
u32 get_tim2_tick(void);  // 返回 10ms 周期计数

#endif
