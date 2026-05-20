#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"

#include "adc.h"
#include "dac.h"
#include "key.h"

#include "GUI.h"
#include "Lcd_Driver.h"




//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	



//DAC实验-库函数版本
//STM32F4工程模板-库函数版本
//淘宝店铺：http://mcudev.taobao.com


	u8 tbuf[40];
	u8 t=0; 
	
	u16 adcx;
	float temp;
	
 	 
	u16 dacval=0;
	u8 key; 
  u8 Numt=0;
	
	
int main(void)
{ 
		
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);                               //初始化延时函数
	uart_init(115200);	                          	//初始化串口波特率为115200
	
	LED_Init();					                            //初始化LED 
 	
	Lcd_Init();	            //1.44寸液晶屏--初始化配置
	Lcd_Clear(GRAY0);       //清屏
	Gui_DrawFont_GBK16(0,16,RED,GRAY0,"    STM32 DAC ");
  Gui_DrawFont_GBK16(0,32,BLUE,GRAY0,"  嵌入式开发网   ");	 
	Gui_DrawFont_GBK16(0,48,BLUE,GRAY0,"mcudev.taobao.com "); 
	
	Adc_Init(); 				//adc初始化
	KEY_Init(); 				//按键初始化
	Dac1_Init();		 		//DAC通道1初始化	
	
	Gui_DrawFont_GBK16(0,64,BLUE,GRAY0,"DAC VAL:");	      
	Gui_DrawFont_GBK16(0,80,BLUE,GRAY0,"DAC VOL:0.000V");	      
	Gui_DrawFont_GBK16(0,96,BLUE,GRAY0,"ADC VOL:0.000V");
 	
  DAC_SetChannel1Data(DAC_Align_12b_R,dacval);//初始值为0	
	
	while(1)
	{
		Numt++;
		key=KEY_Scan(0);
		
		if(key==KEY1_PRES)
		{		 
			if(dacval<4000)dacval+=200;
			DAC_SetChannel1Data(DAC_Align_12b_R, dacval);       //设置DAC值
		}
		
		if(Numt>=10) 	//定时时间到了
		{	  
 			adcx=DAC_GetDataOutputValue(DAC_Channel_1);          //读取前面设置DAC的值
			sprintf((char*)tbuf,"DAC VAL:0x%02X",adcx);      	   //显示DAC寄存器值
			Gui_DrawFont_GBK16(0,64,BLUE,GRAY0,tbuf);	
			
			temp=(float)adcx*(3.3/4096);			                   //得到DAC电压值
			sprintf((char*)tbuf,"DAC VOL:%06.3f V",temp); 
      	
			Gui_DrawFont_GBK16(0,80,BLUE,GRAY0,tbuf);	
      printf("DAC电压:%06.4f V\r\n",temp);                //串口打印						

 			adcx=Get_Adc_Average(ADC_Channel_8,10);		          //得到ADC转换值	  
			temp=(float)adcx*(3.3/4096);			                  //得到ADC电压值
			
			sprintf((char*)tbuf,"ADC VOL:%06.3f V",temp);
			
			Gui_DrawFont_GBK16(0,96,BLUE,GRAY0,tbuf);
      printf("ADC电压:%06.4f V\r\n",temp);                //串口打印					
			
			
			printf("DevEBox 大越电子 \r\n");//串口打印结果
		  printf("mcudev.taobao.com \r\n");//串口打印结果
			
			LED0=!LED0;	   
			LED1=!LED1;	   
			Numt=0;
		}	    
		delay_ms(50);	 
	}	
}








//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	


















//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	







