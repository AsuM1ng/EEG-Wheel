#ifndef __EXTI_H
#define __EXIT_H	 
#include "sys.h"  	
//////////////////////////////////////////////////////////////////////////////////	 
//STM32F4工程模板-库函数版本
//淘宝店铺：http://mcudev.taobao.com								  
////////////////////////////////////////////////////////////////////////////////// 	 

#define EXT_IO0 	GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)  //PA0

#define EXT_IO1 	GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_1)  //PC1

#define EXT_IO2 	GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13) //PC13

void EXTIX_Init(void);	//外部中断初始化		 					    
#endif

























