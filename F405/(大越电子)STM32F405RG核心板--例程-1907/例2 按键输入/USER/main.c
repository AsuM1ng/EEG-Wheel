#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"

#include "key.h"

#include "GUI.h"
#include "Lcd_Driver.h"


//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	



//按键输入实验-库函数版本 
//STM32F4工程模板-库函数版本
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

int main(void)
{ 
 
  u8 key;           //保存键值
	delay_init(168);  //初始化延时函数
	LED_Init();				//初始化LED端口 
	
	Lcd_Init();	 //1.44寸液晶屏--初始化配置
	Lcd_Clear(GRAY0);//清屏
	Gui_DrawFont_GBK16(0,16,RED,GRAY0,"    STM32 Key     ");
  Gui_DrawFont_GBK16(0,48,BLUE,GRAY0,"  嵌入式开发网   ");	 
	Gui_DrawFont_GBK16(0,64,BLUE,GRAY0,"mcudev.taobao.com "); 
	
	KEY_Init();       //初始化与按键连接的硬件接口
	LED0=0;				  	//先点亮D0
	while(1)
	{
		key=KEY_Scan(0);		//得到键值
	  
		if(key)
		{						   
			switch(key)
			{				 
				case KEY0_PRES:	//控制D0翻转
 					   LED0=!LED0;
				     break;
        case KEY1_PRES:	//控制D1翻转
 					   LED1=!LED1;
				     break;
        case WKUP_PRES:	//控制D0D1  翻转
 					   LED0=!LED0;
				     LED1=!LED0;								
					   break;

			}
		}else delay_ms(10); 
	}

}



//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	
