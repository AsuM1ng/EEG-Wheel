#include "sys.h"
#include "delay.h"  
#include "usart.h"  
#include "led.h"

#include "rtc.h"

#include "GUI.h"
#include "Lcd_Driver.h"


//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	





//RTC实时时钟 实验 -库函数版本
//STM32F4工程模板-库函数版本
//淘宝店铺：http://mcudev.taobao.com	

	u8 tbuf[40];
	u8 t=0;
	
int main(void)
{ 

	RTC_TimeTypeDef RTC_TimeStruct;
	RTC_DateTypeDef RTC_DateStruct;


	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);      //初始化延时函数
	uart_init(115200);		//初始化串口波特率为115200
		
	LED_Init();					  //初始化LED
	
 	Lcd_Init();	 //1.44寸液晶屏--初始化配置
	Lcd_Clear(GRAY0);//清屏
	Gui_DrawFont_GBK16(0,16,RED,GRAY0, "    STM32 RTC     ");
  Gui_DrawFont_GBK16(0,32,BLUE,GRAY0,"  嵌入式开发网    ");	 
	Gui_DrawFont_GBK16(0,48,BLUE,GRAY0,"mcudev.taobao.com "); 
	
	
	My_RTC_Init();		 		//初始化RTC
 
	RTC_Set_WakeUp(RTC_WakeUpClock_CK_SPRE_16bits,0);		//配置WAKE UP中断,1秒钟中断一次
	
	
  while(1) 
	{		
		t++;
		if((t%10)==0)	//每100ms更新一次显示数据
		{
;
			RTC_GetTime(RTC_Format_BIN,&RTC_TimeStruct);
			
			sprintf((char*)tbuf,"Time:%02d:%02d:%02d",RTC_TimeStruct.RTC_Hours,RTC_TimeStruct.RTC_Minutes,RTC_TimeStruct.RTC_Seconds); 
				
			Gui_DrawFont_GBK16(0,64,RED,GRAY0,tbuf);
			
			RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
			
			sprintf((char*)tbuf,"Date:20%02d-%02d-%02d",RTC_DateStruct.RTC_Year,RTC_DateStruct.RTC_Month,RTC_DateStruct.RTC_Date); 
			
			Gui_DrawFont_GBK16(0,80,RED,GRAY0,tbuf);	
			
			sprintf((char*)tbuf,"Week:%d",RTC_DateStruct.RTC_WeekDay); 
			
			Gui_DrawFont_GBK16(0,96,RED,GRAY0,tbuf);
		} 
		if((t%20)==0)LED1=!LED1;	//每200ms,翻转一次LED1
		delay_ms(10);
	}	
}






//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	















