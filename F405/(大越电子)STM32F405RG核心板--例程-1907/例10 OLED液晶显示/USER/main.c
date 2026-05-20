



#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "oled.h"
#include "bmp.h"   //显示图片的文件


//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	


//0.96寸/1.3寸  OLED液晶屏  显示实验-库函数版本
//STM32F4工程模板-库函数版本
//淘宝店铺：http://mcudev.taobao.com		

int main(void)
{ 

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);      //初始化延时函数
	uart_init(115200);		//初始化串口波特率为115200
	
	LED_Init();					  //初始化LED
	OLED_Init();			      //初始化OLED     
	
 while(1) 
	{		 
		OLED_Clear();
		OLED_ShowCHinese(0,0,0); //嵌
		OLED_ShowCHinese(18,0,1);//入
		OLED_ShowCHinese(36,0,2);//式
		OLED_ShowCHinese(54,0,3);//开
		OLED_ShowCHinese(72,0,4);//发
		OLED_ShowCHinese(90,0,5);//网
		
    OLED_ShowString(0,3,"mcudev.taobao.com"); 
		OLED_ShowString(0,6,"0.96/1.30' OLED ");
		delay_ms(800);
		delay_ms(800);

		OLED_Clear();
		
		OLED_DrawBMP(0,0,128,8,BMP1);  //图片显示(图片显示慎用，生成的字表较大，会占用较多空间，FLASH空间8K以下慎用)
		delay_ms(800);	

		OLED_DrawBMP(0,0,128,8,BMP3);
		delay_ms(800);	

		OLED_DrawBMP(0,0,128,8,BMP4);
		delay_ms(800);	

		OLED_DrawBMP(0,0,128,8,BMP5);
		delay_ms(800);	

		OLED_DrawBMP(0,0,128,8,BMP6);
		delay_ms(800);
		
		LED0=!LED0;	 
		
		delay_ms(1000);	
		
	} 
}



//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	






