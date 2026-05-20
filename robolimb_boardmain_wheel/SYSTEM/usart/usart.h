#ifndef __USART_H
#define __USART_H

#include "sys.h" 
#include "main.h"	

void USART1_Init(u32 bound);
u8 get_serial_cmd(void);
extern volatile u8 serial_cmd;
extern volatile u8 emergency_stop_flag;

#endif
