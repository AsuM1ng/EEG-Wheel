#ifndef __Lcd_Driver_H
#define __Lcd_Driver_H

#include "sys.h"
#include "stdint.h"
#include "stm32f4xx.h"




//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************


//STM32核心板例程

//库函数版本例程



#define RED  	  0xf800
#define GREEN	  0x07e0
#define BLUE 	  0x001f
#define WHITE	  0xffff
#define BLACK	  0x0000
#define YELLOW  0xFFE0
#define GRAY0   0xEF7D   	//灰色0 
#define GRAY1   0x8410    //灰色1 
#define GRAY2   0x4208    //灰色2 




//液晶控制口置1操作语句宏定义
#define	LCD_SDA_SET  	GPIO_SetBits(GPIOB,GPIO_Pin_15)
#define	LCD_SCL_SET  	GPIO_SetBits(GPIOB,GPIO_Pin_13) 

#define	LCD_CS_SET  	GPIO_SetBits(GPIOB,GPIO_Pin_12)   

#define	LCD_RS_SET  	GPIO_SetBits(GPIOC,GPIO_Pin_6)   
#define	LCD_BLK_SET  	GPIO_SetBits(GPIOC,GPIO_Pin_7)   

//液晶控制口置0操作语句宏定义

#define	LCD_SDA_CLR  	GPIO_ResetBits(GPIOB,GPIO_Pin_15)    
#define	LCD_SCL_CLR  	GPIO_ResetBits(GPIOB,GPIO_Pin_13)  

#define	LCD_CS_CLR  	GPIO_ResetBits(GPIOB,GPIO_Pin_12)  

#define	LCD_RS_CLR  	GPIO_ResetBits(GPIOC,GPIO_Pin_6) 
#define	LCD_BLK_CLR  	GPIO_ResetBits(GPIOC,GPIO_Pin_7) 


void LCD_GPIO_Init(void);
void Lcd_WriteIndex(uint8_t Index);
void Lcd_WriteData(uint8_t Data);
void Lcd_WriteReg(uint8_t Index,uint8_t Data);
uint16_t Lcd_ReadReg(uint8_t LCD_Reg);
void Lcd_Reset(void);
void Lcd_Init(void);
void Lcd_Clear(uint16_t Color);
void Lcd_SetXY(uint16_t x,uint16_t y);
void Gui_DrawPoint(uint16_t x,uint16_t y,uint16_t Data);
void Lcd_SetRegion(uint16_t x_start,uint16_t y_start,uint16_t x_end,uint16_t y_end);
void LCD_WriteData_16Bit(uint16_t Data);

#endif



//********************************************************************************

//DevEBox  大越电子（嵌入式开发网）
//淘宝店铺：mcudev.taobao.com
//淘宝店铺：shop389957290.taobao.com	

//********************************************************************************













