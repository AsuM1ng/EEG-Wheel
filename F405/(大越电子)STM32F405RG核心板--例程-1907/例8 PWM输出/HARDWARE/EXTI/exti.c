#include "exti.h"
#include "delay.h" 
#include "led.h" 
#include "key.h"

//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	

//////////////////////////////////////////////////////////////////////////////////	 
//STM32F4工程模板-库函数版本
//淘宝店铺：http://mcudev.taobao.com								  
////////////////////////////////////////////////////////////////////////////////// 




//********************************************************************************
//函数功能：外部中断0服务程序 PA0

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	


void EXTI0_IRQHandler(void)
{
	delay_ms(20);	//消抖
	
	if(EXT_IO0==1)	 
	{
		LED0=!LED0;
	}		 
	 EXTI_ClearITPendingBit(EXTI_Line0); //清除LINE0上的中断标志位 
}	


//********************************************************************************
//函数功能：外部中断1服务程序  PC1


//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	


void EXTI1_IRQHandler(void)
{
	delay_ms(20);	//消抖
	
	if(EXT_IO1==0)	  
	{				 
   LED1=!LED1;
	}		 
	EXTI_ClearITPendingBit(EXTI_Line1);//清除LINE1上的中断标志位 
	
}

//********************************************************************************
//函数功能：外部中断15_10服务程序  PC13


//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	


void EXTI15_10_IRQHandler(void)
{
	delay_ms(20);	//消抖
	
	if(EXT_IO2==0)	  
	{
   LED0=!LED0;//双灯交替亮起		
   LED1=!LED0;
	}		 
	EXTI_ClearITPendingBit(EXTI_Line13);//清除LINE13上的中断标志位 
	
}

	   


//********************************************************************************
//函数功能：//外部中断初始化程序
//初始化PA0,PA1为中断输入.

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	

void EXTIX_Init(void)
{
	NVIC_InitTypeDef   NVIC_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;
		
	KEY_Init(); //按键对应的IO口初始化
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);//使能SYSCFG时钟
	
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource1);  //PC1  连接到中断线1
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource13); //PC13 连接到中断线13
	
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);  //PA0  连接到中断线0
	
  /* 配置EXTI_Line0 */
  EXTI_InitStructure.EXTI_Line = EXTI_Line0;             //LINE0
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;    //中断事件
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //上升沿触发 
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//使能LINE0
  EXTI_Init(&EXTI_InitStructure);//配置
	
	/* 配置EXTI_Line1 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line1;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;      //中断事件
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  //下降沿触发
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;                //中断线使能
  EXTI_Init(&EXTI_InitStructure);                          //配置


	/* 配置EXTI_Line13 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line13;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;      //中断事件
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  //下降沿触发
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;                //中断线使能
  EXTI_Init(&EXTI_InitStructure);                          //配置
	

/******** 设置中断优先级****/

	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;              //外部中断0
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;  //抢占优先级0
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;         //子优先级2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;               //使能外部中断通道
  NVIC_Init(&NVIC_InitStructure);//配置
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;             //外部中断1
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03; //抢占优先级3
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;        //子优先级2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;              //使能外部中断通道
  NVIC_Init(&NVIC_InitStructure);//配置
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;             //外部中断1
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03;     //抢占优先级3
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;            //子优先级3
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                  //使能外部中断通道
  NVIC_Init(&NVIC_InitStructure);//配置
	
	   
}












