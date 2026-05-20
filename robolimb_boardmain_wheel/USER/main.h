#ifndef _MAIN_H
#define	_MAIN_H

#include "sys.h"
#include "delay.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "usart.h"


/*****CANopen ���ͷ�ļ�********/

#include "can1.h"
#include "timer2.h"
#include "timer3.h"
#include "data.h"
#include "Master.h"
#include "canfestival.h"

// 轮子急停函数(供motions.c调用)
extern void wheel_emergency_stop(void);

#endif


