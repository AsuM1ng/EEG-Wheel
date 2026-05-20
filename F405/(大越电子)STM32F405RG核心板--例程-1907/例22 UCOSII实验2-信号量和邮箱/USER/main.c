#include "sys.h"
#include "delay.h"  
#include "usart.h"   
#include "led.h"

#include "key.h"  
	 	
#include "includes.h"

#include "GUI.h"
#include "Lcd_Driver.h"



//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	



//UCOSII-信号量和邮箱    --库函数版本
//STM32F4工程模板-库函数版本
//淘宝店铺：http://mcudev.taobao.com

 

	u8 tbuf[64];
	u8 t=0; 
	float temp; 
	
/////////////////////////UCOSII任务设置///////////////////////////////////
//START 任务
//设置任务优先级
#define START_TASK_PRIO      			10 //开始任务的优先级设置为最低
//设置任务堆栈大小
#define START_STK_SIZE  				64
//任务堆栈	
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);	
 			   

//设置任务优先级
#define TOUCH_TASK_PRIO       		 	7
//设置任务堆栈大小
#define TOUCH_STK_SIZE  				64
//任务堆栈	
OS_STK TOUCH_TASK_STK[TOUCH_STK_SIZE];
//任务函数
void touch_task(void *pdata);


//LED任务
//设置任务优先级
#define LED_TASK_PRIO       			6 
//设置任务堆栈大小
#define LED_STK_SIZE  		    		64
//任务堆栈	
OS_STK LED_TASK_STK[LED_STK_SIZE];
//任务函数
void led_task(void *pdata);

//LED1任务
//设置任务优先级
#define LED1_TASK_PRIO       			5 
//设置任务堆栈大小
#define LED1_STK_SIZE  					64
//任务堆栈	
OS_STK LED1_TASK_STK[LED1_STK_SIZE];
//任务函数
void LED1_task(void *pdata);


//主任务
//设置任务优先级
#define MAIN_TASK_PRIO       			4 
//设置任务堆栈大小
#define MAIN_STK_SIZE  					128
//任务堆栈	
OS_STK MAIN_TASK_STK[MAIN_STK_SIZE];
//任务函数
void main_task(void *pdata);

//按键扫描任务
//设置任务优先级
#define KEY_TASK_PRIO       			3 
//设置任务堆栈大小
#define KEY_STK_SIZE  					64
//任务堆栈	
OS_STK KEY_TASK_STK[KEY_STK_SIZE];
//任务函数
void key_task(void *pdata);
//////////////////////////////////////////////////////////////////////////////
//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	


OS_EVENT * msg_key;			//按键邮箱事件块指针
OS_EVENT * sem_beep;		//信号量指针	 	  
//加载主界面   
void ucos_load_main_ui(void)
{

	Gui_DrawFont_GBK16(0,0,RED,GRAY0,"Mcudev STM32");	
	Gui_DrawFont_GBK16(0,16,RED,GRAY0,"UCOSII TEST2");	
	Gui_DrawFont_GBK16(0,32,BLUE,GRAY0,"mcudev.taobao.com");
 	Gui_DrawFont_GBK16(0,64,BLUE,GRAY0,"CPU:        %");	
  Gui_DrawFont_GBK16(0,80,BLUE,GRAY0,"SEM:000");	
}	 

//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	

	
int main(void)
{ 

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);  //初始化延时函数
	uart_init(115200);		//初始化串口波特率为115200
	
	LED_Init();					//初始化LED 

	KEY_Init(); 				//按键初始化  
	
	Lcd_Init();	 //1.44寸液晶屏--初始化配置
	Lcd_Clear(GRAY0);//清屏
//	Gui_DrawFont_GBK16(0,16,RED,GRAY0,"STM32 UCOSII ");
//  Gui_DrawFont_GBK16(0,32,BLUE,GRAY0,"  嵌入式开发网   ");	 
//	Gui_DrawFont_GBK16(0,48,BLUE,GRAY0,"mcudev.taobao.com ");

	
	ucos_load_main_ui();		//加载主界面	 
  OSInit();  	 				//初始化UCOSII
  OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();	    
}
  

//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	

//两个数之差的绝对值 
//x1,x2：需取差值的两个数
//返回值：|x1-x2|
u16 my_abs(u16 x1,u16 x2)
{			 
	if(x1>x2)return x1-x2;
	else return x2-x1;
}  
 
///////////////////////////////////////////////////////////////////////////////////////////////////

//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	

//开始任务
void start_task(void *pdata)
{
	
	
  OS_CPU_SR cpu_sr=0;
	pdata = pdata; 		  
	msg_key=OSMboxCreate((void*)0);	//创建消息邮箱
	sem_beep=OSSemCreate(0);		//创建信号量		 			  
	OSStatInit();					//初始化统计任务.这里会延时1秒钟左右	
 	OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)   
	
 	OSTaskCreate(touch_task,(void *)0,(OS_STK*)&TOUCH_TASK_STK[TOUCH_STK_SIZE-1],TOUCH_TASK_PRIO);	 				   
 	OSTaskCreate(led_task,(void *)0,(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],LED_TASK_PRIO);						   
 	OSTaskCreate(LED1_task,(void *)0,(OS_STK*)&LED1_TASK_STK[LED1_STK_SIZE-1],LED1_TASK_PRIO);	 				   
 	OSTaskCreate(main_task,(void *)0,(OS_STK*)&MAIN_TASK_STK[MAIN_STK_SIZE-1],MAIN_TASK_PRIO);	 				   
 	OSTaskCreate(key_task,(void *)0,(OS_STK*)&KEY_TASK_STK[KEY_STK_SIZE-1],KEY_TASK_PRIO);	 				   
 	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
	
	
}	 

//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	

//LED任务
void led_task(void *pdata)
{
	u8 t;
	while(1)
	{
		t++;
		delay_ms(10);
		if(t==8)LED0=1;	//LED0灭
		if(t==100)		//LED0亮
		{
			t=0;
			LED0=0;
		}
	}									 
}	   


//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	

//LED1任务
void LED1_task(void *pdata)
{
	u8 err;
	while(1)
	{
		OSSemPend(sem_beep,0,&err);
		LED1=1;

		delay_ms(60);

	  LED1=0;
		delay_ms(940);
	}									 
} 

//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	

//触摸屏任务
void touch_task(void *pdata)
{	  	
	u32 cpu_sr;
	
 	u16 lastpos[2];		//最后一次的数据 
	
	while(1)
	{
    //printf("一个运行的任务\r\n");//串口打印结果
	}
}

//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	

//主任务
void main_task(void *pdata)
{							 
	u32 key=0;	
	u8 err;	
	u8 semmask=0;
	u8 tcnt=0;						 
	while(1)
	{
		key=(u32)OSMboxPend(msg_key,10,&err);
		
		switch(key)
		{
			case 1://控制D3
				LED1=!LED1;
				break;
			case 2://发送信号量
				semmask=1;
				OSSemPost(sem_beep);
				break;
			case 3://清除
				Lcd_Clear(GRAY0);//清屏
				break;

		}
   	if(semmask||sem_beep->OSEventCnt)//需要显示sem		
		{
			if(sem_beep->OSEventCnt==0)semmask=0;	//停止更新
			
		}
		
		if(tcnt==50)//0.5秒更新一次CPU使用率
		{
			tcnt=0;
			
			sprintf((char*)tbuf,"CPU: %05.2f",OSCPUUsage);//显示CPU使用率  	  
			
			Gui_DrawFont_GBK16(0,64,BLUE,GRAY0,tbuf);	  
			
		}
		tcnt++;
		delay_ms(10);
	}
} 


//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	

u8 key_Num;	
u8 key;	
//按键扫描任务
void key_task(void *pdata)
{	

	
	while(1)
	{
		key=KEY_Scan(0); 
		
		if(key==KEY1_PRES)
		{
			key_Num++;
			if(key_Num>4)key_Num=0;
			
		}
		
		if(key)OSMboxPost(msg_key,(void*)key_Num);//发送消息
		
 		delay_ms(10);
	}
}
























//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	








