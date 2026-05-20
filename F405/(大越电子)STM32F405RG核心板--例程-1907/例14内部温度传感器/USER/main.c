#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"

#include "adc.h"

#include "GUI.h"
#include "Lcd_Driver.h"


//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************		



//内部温度传感器实验 -库函数版本 
//STM32F4工程模板-库函数版本
//淘宝店铺：http://mcudev.taobao.com	
 
	u8 tbuf[64];
	u8 t=0; 
	float temp; 
//short temp; 	
int main(void)
{ 
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);     //初始化延时函数
	uart_init(115200);	 //初始化串口波特率为115200

	LED_Init();					//初始化LED 
	
 	 	Lcd_Init();	 //1.44寸液晶屏--初始化配置
	Lcd_Clear(GRAY0);//清屏
	Gui_DrawFont_GBK16(0,16,RED,GRAY0,"    STM32 TEMP ");
  Gui_DrawFont_GBK16(0,32,BLUE,GRAY0,"  嵌入式开发网   ");	 
	Gui_DrawFont_GBK16(0,48,BLUE,GRAY0,"mcudev.taobao.com "); 
	
	
	Adc_Init();         //内部温度传感器ADC初始化
    
	Gui_DrawFont_GBK16(0,64,RED,GRAY0,"TEMP:00.00C");//先在固定位置显示小数点	   
	
	while(1)
	{
		temp=Get_Temprate();	//得到温度值 
		temp=temp/100;
		sprintf((char*)tbuf,"TEMP:%05.2fC",temp); 
		
		printf("DevEBox 大越电子 \r\n");//串口打印结果
		printf("mcudev.taobao.com \r\n");//串口打印结果
		printf("CPU温度:%05.2fC\r\n",temp);//串口打印结果
		
		Gui_DrawFont_GBK16(0,64,RED,GRAY0,tbuf);
		LED0=!LED0; 
		LED1=!LED1;
		delay_ms(250);	
	}
}




//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************		



