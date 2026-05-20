#include "stm32f4xx.h"
#include "usart.h"
#include "delay.h"


//STM32F4工程模板-库函数版本

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	


u32 t=0;

int main(void)
{
	
	delay_init(168);		  //初始化延时函数
	
	uart_init(115200);//设置串口波特率
	
	
  while(1){
    printf("t:%d\r\n",t);
		delay_ms(500);
		t++;
	}
}



