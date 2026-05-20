#include "led.h"
#include "delay.h"

// LED引脚定义（PA8=LED0, PD2=LED1，低电平点亮）
#define LED0_ON()   GPIO_ResetBits(GPIOA, GPIO_Pin_8)
#define LED0_OFF()  GPIO_SetBits(GPIOA, GPIO_Pin_8)
#define LED1_ON()   GPIO_ResetBits(GPIOD, GPIO_Pin_2)
#define LED1_OFF()  GPIO_SetBits(GPIOD, GPIO_Pin_2)
#define ALL_ON()    {LED0_ON(); LED1_ON();}
#define ALL_OFF()   {LED0_OFF(); LED1_OFF();}

// ---------- LED 初始化 ----------
void LED_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOD, ENABLE);
    
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    ALL_OFF();
}

// 启动时双LED闪烁3次（硬件自检）
void LED_StartupBlink(void) {
    int i;
    for (i = 0; i < 3; i++) {
        ALL_ON();  delay_ms(200);
        ALL_OFF(); delay_ms(200);
    }
}

// D0 闪烁（动作1指示）
void LED0_Blink(void) {
    int i;
    for (i = 0; i < 3; i++) {
        LED0_ON();  delay_ms(100);
        LED0_OFF(); delay_ms(100);
    }
}

// D1 闪烁（动作2指示）
void LED1_Blink(void) {
    int i;
    for (i = 0; i < 3; i++) {
        LED1_ON();  delay_ms(100);
        LED1_OFF(); delay_ms(100);
    }
}

// 双 LED 闪烁（停止指示）
void LED_AllBlink(void) {
    int i;
    for (i = 0; i < 3; i++) {
        ALL_ON();  delay_ms(100);
        ALL_OFF(); delay_ms(100);
    }
}
