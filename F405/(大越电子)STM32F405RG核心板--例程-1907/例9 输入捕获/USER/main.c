#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "timer.h"

#include "GUI.h"
#include "Lcd_Driver.h"



//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	
 
 
//输入捕获实验 -库函数版本 
 
 
extern u8  TIM5CH1_CAPTURE_STA;		//输入捕获状态		    				
extern u32	TIM5CH1_CAPTURE_VAL;	//输入捕获值  
  
	
int main(void)
{
	
	long long temp=0;  
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);  //初始化延时函数
	uart_init(115200);//初始化串口波特率为115200
  LED_Init();				//初始化LED端口
	Lcd_Init();	      //1.44寸液晶屏--初始化配置
	
	Lcd_Clear(GRAY0);//清屏
	
	Gui_DrawFont_GBK16(0,16,RED,GRAY0,"  STM32 ICAPTURE  ");
  Gui_DrawFont_GBK16(0,48,BLUE,GRAY0,"  嵌入式开发网   ");	 
	Gui_DrawFont_GBK16(0,64,BLUE,GRAY0,"mcudev.taobao.com "); 
	
 	TIM3_PWM_Init(500-1,84-1);       	//84M/84=1Mhz的计数频率计数到500,PWM频率为1M/500=2Khz     
 	TIM5_CH1_Cap_Init(0xFFFFFFFF,84-1); //以1Mhz的频率计数 
	
	LED0=!LED0;      //D0  翻转
	LED1=!LED1;      //D1  翻转
	
  while(1)
	{
		
 		delay_ms(100);
		
		TIM_SetCompare1(TIM3,TIM_GetCapture1(TIM3)+1); 
		if(TIM_GetCapture1(TIM3)==300)TIM_SetCompare1(TIM3,0);	
		
 		if(TIM5CH1_CAPTURE_STA&0X80)        //成功捕获到了一次高电平
		{
			temp=TIM5CH1_CAPTURE_STA&0X3F; 
			temp*=0xFFFFFFFF;		 		         //溢出时间总和
			temp+=TIM5CH1_CAPTURE_VAL;		   //得到总的高电平时间
			printf("HIGH:%lld us\r\n",temp); //打印总的高点平时间
			TIM5CH1_CAPTURE_STA=0;			     //开启下一次捕获
		}
		
	 LED0=!LED0;      //D0  翻转
	 LED1=!LED1;      //D1  翻转
		
		
	}
}
