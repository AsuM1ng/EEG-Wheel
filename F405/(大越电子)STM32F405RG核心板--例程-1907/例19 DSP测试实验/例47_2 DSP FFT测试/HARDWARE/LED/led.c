#include "led.h" 
//////////////////////////////////////////////////////////////////////////////////	 


//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************			

//STM32F4工程模板-库函数版本

////////////////////////////////////////////////////////////////////////////////// 	 

//初始化 LED0--PA8,  LED1--PD2  输出口.并使能这两个口的时钟		

//LED IO初始化
void LED_Init(void)
{    	 
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOD, ENABLE);//使能 GPIOA, GPIOD 时钟

  //初始化设置
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 ;         //LED0--PA8
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;      //普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     //推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       //上拉
  GPIO_Init(GPIOA, &GPIO_InitStructure);              //初始化GPIO
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 ;         //LED1--PD2
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;      //普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     //推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       //上拉
  GPIO_Init(GPIOD, &GPIO_InitStructure);              //初始化GPIO
	
	GPIO_SetBits(GPIOA,GPIO_Pin_8);//设置高，灯灭
	GPIO_SetBits(GPIOD,GPIO_Pin_2);//设置高，灯灭

}



//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************	










