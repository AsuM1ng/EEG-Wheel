
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"

#include "GUI.h"
#include "Lcd_Driver.h"



//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************

//LED灯实验 -库函数版本
//STM32F4工程模板-库函数版本

int main(void)
{ 
 
	delay_init(168);		  //初始化延时函数
	LED_Init();		        //初始化LED端口
	
	Lcd_Init();	 //1.44寸液晶屏--初始化配置
	Lcd_Clear(GRAY0);//清屏
	Gui_DrawFont_GBK16(0,16,RED,GRAY0," Test STM32 LED ");
  Gui_DrawFont_GBK16(0,48,BLUE,GRAY0,"  嵌入式开发网   ");	 
	Gui_DrawFont_GBK16(0,64,BLUE,GRAY0,"mcudev.taobao "); 
	Gui_DrawFont_GBK16(0,80,BLUE,GRAY0,"shop389957290.taobao"); 
	
  /**下面是通过直接操作库函数的方式实现IO控制**/	
	
	while(1)
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_8);  //D0对应引脚PA8拉低，亮  等同LED0=0;
		GPIO_ResetBits(GPIOD,GPIO_Pin_2);  //D1对应引脚PD2拉低，亮  等同LED1=0;	
		
		delay_ms(500);  		   //延时300ms

		GPIO_SetBits(GPIOA,GPIO_Pin_8);  //D0对应引脚PA8拉高，灭  等同LED0=1;
		GPIO_SetBits(GPIOD,GPIO_Pin_2);  //D1对应引脚PD2拉高，灭  等同LED1=1;
			
		delay_ms(500);                     //延时300ms
	}
}



 



