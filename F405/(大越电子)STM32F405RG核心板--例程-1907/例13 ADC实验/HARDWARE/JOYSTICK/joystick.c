#include "joystick.h"
#include "delay.h"

// ============================================================
// Forward declaration of hardware-level ADC read
// (implemented in adc.c / adc.h from the example project)
// ============================================================
extern u16 Get_Adc(u8 ch);
extern u16 Get_Adc_Average(u8 ch, u8 times);

// ============================================================
// GPIO and ADC init
// ============================================================
void Joystick_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_InitTypeDef   ADC_InitStructure;

    // Enable clocks
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    // ---- ADC pins: PB0 (X-axis), PB1 (Y-axis) ----
    GPIO_InitStructure.GPIO_Pin = JOY_ADC_X_PIN | JOY_ADC_Y_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(JOY_ADC_X_PORT, &GPIO_InitStructure);

    // ---- Button/switch pins: PC4, PC5, PA4, PA5 as INPUT with Pull-Up ----
    // PC4 (BTN1)
    GPIO_InitStructure.GPIO_Pin  = JOY_BTN1_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   // internal pull-up
    GPIO_Init(JOY_BTN1_PORT, &GPIO_InitStructure);

    // PC5 (BTN2)
    GPIO_InitStructure.GPIO_Pin  = JOY_BTN2_PIN;
    GPIO_Init(JOY_BTN2_PORT, &GPIO_InitStructure);

    // PA4 (DIR_P) -- NOTE: PA4 is also used by DAC! Keep as GPIO input only
    GPIO_InitStructure.GPIO_Pin  = JOY_DIRP_PIN;
    GPIO_Init(JOY_DIRP_PORT, &GPIO_InitStructure);

    // PA5 (DIR_N)
    GPIO_InitStructure.GPIO_Pin  = JOY_DIRN_PIN;
    GPIO_Init(JOY_DIRN_PORT, &GPIO_InitStructure);

    // ---- ADC1 configuration (independent mode, same clock as existing adc.c) ----
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, DISABLE);

    ADC_CommonInitStructure.ADC_Mode               = ADC_Mode_Independent;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay     = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInitStructure.ADC_DMAAccessMode       = ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStructure.ADC_Prescaler           = ADC_Prescaler_Div4;
    ADC_CommonInit(&ADC_CommonInitStructure);

    ADC_InitStructure.ADC_Resolution           = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode         = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode   = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_DataAlign           = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion     = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_Cmd(ADC1, ENABLE);
}

// ============================================================
// Raw ADC reads
// ============================================================
u16 Joystick_GetAdcX(void)
{
    return Get_Adc(JOY_ADC_CH_X);
}

u16 Joystick_GetAdcY(void)
{
    return Get_Adc(JOY_ADC_CH_Y);
}

u16 Joystick_GetAdcX_Avg(u8 times)
{
    return Get_Adc_Average(JOY_ADC_CH_X, times);
}

u16 Joystick_GetAdcY_Avg(u8 times)
{
    return Get_Adc_Average(JOY_ADC_CH_Y, times);
}

// ============================================================
// Voltage reads
// ============================================================
float Joystick_GetVoltageX(void)
{
    u16 raw = Joystick_GetAdcX_Avg(16);
    return (float)raw * JOYSTICK_VREF / (float)JOYSTICK_ADC_MAX;
}

float Joystick_GetVoltageY(void)
{
    u16 raw = Joystick_GetAdcY_Avg(16);
    return (float)raw * JOYSTICK_VREF / (float)JOYSTICK_ADC_MAX;
}

// ============================================================
// Normalized value: -1000 ~ +1000, deadzone applied
// ============================================================
static s16 apply_deadzone(u16 raw, u16 center)
{
    s32 val = (s32)raw - (s32)center;
    if (val > -(JOYSTICK_DEADZONE) && val < (JOYSTICK_DEADZONE)) {
        return 0;
    }
    // Scale to -1000 ~ +1000
    // raw range: 0 ~ 4095, center=2047, so range is ±2048
    s32 scaled = (val * 1000) / 2048;
    if (scaled >  1000) scaled =  1000;
    if (scaled < -1000) scaled = -1000;
    return (s16)scaled;
}

s16 Joystick_GetNormalizedX(void)
{
    // If joystick is powered by 3.3V: center = 2047
    // If powered by 5V with 10k+10k divider: center = 2047 (still works)
    // Normalize so: left < center < right
    u16 raw = Joystick_GetAdcX_Avg(8);
    return apply_deadzone(raw, JOYSTICK_CENTER);
}

s16 Joystick_GetNormalizedY(void)
{
    u16 raw = Joystick_GetAdcY_Avg(8);
    return apply_deadzone(raw, JOYSTICK_CENTER);
}

// ============================================================
// Button reads (0 = pressed, 1 = released)
// ============================================================
u8 Joystick_GetBtn1(void)
{
    return GPIO_ReadInputDataBit(JOY_BTN1_PORT, JOY_BTN1_PIN);
}

u8 Joystick_GetBtn2(void)
{
    return GPIO_ReadInputDataBit(JOY_BTN2_PORT, JOY_BTN2_PIN);
}

u8 Joystick_GetDirP(void)
{
    return GPIO_ReadInputDataBit(JOY_DIRP_PORT, JOY_DIRP_PIN);
}

u8 Joystick_GetDirN(void)
{
    return GPIO_ReadInputDataBit(JOY_DIRN_PORT, JOY_DIRN_PIN);
}
