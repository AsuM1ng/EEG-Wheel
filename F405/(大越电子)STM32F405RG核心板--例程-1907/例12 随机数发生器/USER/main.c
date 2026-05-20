#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "rng.h"
#include "key.h"

#include "GUI.h"
#include "Lcd_Driver.h"

//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	


//随机数发生器(RNG) 实验  -库函数版本
//STM32F4工程模板-库函数版本
//淘宝店铺：http://mcudev.taobao.com	
 

	u8 tbuf[40];
	u8 t=0;
	
int main(void)
{ 
	u32 random;
	u8 t=0,key;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);  //初始化延时函数
	uart_init(115200);		//初始化串口波特率为115200
	LED_Init();					//初始化LED
	KEY_Init();					//按键初始化

 	Lcd_Init();	 //1.44寸液晶屏--初始化配置
	Lcd_Clear(GRAY0);//清屏
	Gui_DrawFont_GBK16(0,16,RED,GRAY0,"  STM32 Random ");
  Gui_DrawFont_GBK16(0,32,BLUE,GRAY0,"  嵌入式开发网   ");	 
	Gui_DrawFont_GBK16(0,48,BLUE,GRAY0,"mcudev.taobao.com ");
	
	while(RNG_Init())	 		//初始化随机数发生器
	{
		Gui_DrawFont_GBK16(0,64,RED,GRAY0,"  RNG Error! ");	 
		delay_ms(200);
		Gui_DrawFont_GBK16(0,80,RED,GRAY0,"RNG Trying...");	 
	}                                 
	Gui_DrawFont_GBK16(0,64,RED,GRAY0,"RNG Ready!   ");	 
	Gui_DrawFont_GBK16(0,80,RED,GRAY0,"KEY1:Get Random Num");	 
	Gui_DrawFont_GBK16(0,96,RED,GRAY0,"Random Num[0-9]:");	 	
  

	while(1) 
	{		
		delay_ms(10);
		key=KEY_Scan(0);
		if(key==KEY1_PRES)
		{
			random=RNG_Get_RandomNum(); //获得随机数
			sprintf((char*)tbuf,"Random Num:%02d       ",random); 
			Gui_DrawFont_GBK16(0,96,RED,GRAY0,tbuf); //显示随机数

		} 
		if((t%20)==0)
		{ 
			LED0=!LED0;	//每200ms,翻转一次LED0 
			LED1=!LED1;	//每200ms,翻转一次LED1 
			random=RNG_Get_RandomRange(0,9);//获取[0,9]区间的随机数
			sprintf((char*)tbuf,"Random Num:%02d       ",random); 
			Gui_DrawFont_GBK16(0,96,RED,GRAY0,tbuf); //显示随机数
		 }
		delay_ms(10);
		t++;
	}	
}










//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	




