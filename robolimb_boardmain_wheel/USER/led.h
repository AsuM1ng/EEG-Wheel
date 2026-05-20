#ifndef __LED_H
#define __LED_H

#include "sys.h"

// LED初始化
void LED_Init(void);

// 启动闪烁（上电自检）
void LED_StartupBlink(void);

// 动作指示闪烁
void LED0_Blink(void);   // 动作1指示
void LED1_Blink(void);   // 动作2指示
void LED_AllBlink(void); // 停止指示

#endif
