#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//STM32F4工程模板-库函数版本


//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	

/*下面的方式是通过直接操作库函数方式读取IO*/

#define KEY0 		GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_1)   //PC1
#define KEY1 		GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13)	//PC13
#define WK_UP 	GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)	  //PA0



#define KEY0_PRES 	1
#define KEY1_PRES	  2
#define WKUP_PRES   3

void KEY_Init(void);	//IO初始化
u8 KEY_Scan(u8);  		//按键扫描函数	

#endif













//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	





