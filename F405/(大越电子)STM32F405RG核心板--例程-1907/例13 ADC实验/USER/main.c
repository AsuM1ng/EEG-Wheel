#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "adc.h"
#include "joystick.h"
#include "GUI.h"
#include "Lcd_Driver.h"

// ============================================================
// JC2000 Joystick Test Program
// Based on: Example 13 ADC Experiment
// Hardware: STM32F405RG + PG Drives JC2000 Joystick (8-wire)
// ============================================================
//
// WIRING (verify cable colors with multimeter first!):
//
// JC2000 Cable      STM32F405RG Pin
// -----------        ----------------
// Red (+5V or 3.3V) -> 3.3V (or external 5V if JC2000 needs 5V)
// Black (GND)       -> GND
// White (X-axis)    -> PB0  (ADC1_IN8)
// Yellow (Y-axis)   -> PB1  (ADC1_IN9)
// Green (Button1)   -> PC4  (GPIO input, pull-up)
// Blue (Button2)    -> PC5  (GPIO input, pull-up)
// Orange (Dir+)     -> PA4  (GPIO input, pull-up)
// Purple (Dir-)     -> PA5  (GPIO input, pull-up)
//
// IMPORTANT: If JC2000 outputs 5V analog signals,
// add 10k+10k voltage dividers on White/Yellow lines
// before connecting to PB0/PB1.
//
// ============================================================

 u8 tbuf[50];
 u8 t = 0;
 u16 adcx_raw;
 u16 adcy_raw;
 float volt_x;
 float volt_y;
 s16 norm_x;
 s16 norm_y;
 u8 btn1, btn2, dirp, dirn;

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    delay_init(168);
    uart_init(115200);
    LED_Init();

    // 1.44 TFT init
    Lcd_Init();
    Lcd_Clear(GRAY0);
    Gui_DrawFont_GBK16(0, 0,  RED,  GRAY0, "  JC2000 Joystick Test  ");
    Gui_DrawFont_GBK16(0, 16, BLUE, GRAY0, " PG Drives 8-Wire Joystick");
    Gui_DrawFont_GBK16(0, 32, BLUE, GRAY0, " STM32F405RG ADC Demo    ");
    Gui_DrawFont_GBK16(0, 50, BLUE, GRAY0, "Xraw:        Yraw:       ");
    Gui_DrawFont_GBK16(0, 66, BLUE, GRAY0, "Xvolt:        V          ");
    Gui_DrawFont_GBK16(0, 82, BLUE, GRAY0, "Yvolt:        V          ");
    Gui_DrawFont_GBK16(0, 98, BLUE, GRAY0, "Xnorm:       Ynorm:      ");
    Gui_DrawFont_GBK16(0,114, BLUE, GRAY0, "BTN1:   BTN2:   DIR:     ");

    // Init existing ADC hardware (PB0, used by adc.c)
    Adc_Init();
    // Init JC2000: additional ADC channel PB1 + all GPIO buttons
    Joystick_Init();

    printf("\r\n========== JC2000 Joystick Test ==========\r\n");
    printf("PB0=X-axis(ADC_IN8)  PB1=Y-axis(ADC_IN9)\r\n");
    printf("PC4=BTN1  PC5=BTN2  PA4=DIR+  PA5=DIR-\r\n");
    printf("==========================================\r\n\r\n");

    while (1)
    {
        // ---- Read raw ADC ----
        adcx_raw = Joystick_GetAdcX_Avg(20);  // X-axis: PB0
        adcy_raw = Joystick_GetAdcY_Avg(20);  // Y-axis: PB1

        // ---- Calculate voltage ----
        volt_x = (float)adcx_raw * JOYSTICK_VREF / 4096.0f;
        volt_y = (float)adcy_raw * JOYSTICK_VREF / 4096.0f;

        // ---- Normalized value (-1000 ~ +1000) ----
        norm_x = Joystick_GetNormalizedX();
        norm_y = Joystick_GetNormalizedY();

        // ---- Read buttons (0=pressed, 1=released) ----
        btn1 = Joystick_GetBtn1();
        btn2 = Joystick_GetBtn2();
        dirp = Joystick_GetDirP();
        dirn = Joystick_GetDirN();

        // ---- Update TFT display ----
        sprintf((char*)tbuf, "Xraw:%04d ", adcx_raw);
        Gui_DrawFont_GBK16(60, 50, RED, GRAY0, tbuf);

        sprintf((char*)tbuf, "Yraw:%04d", adcy_raw);
        Gui_DrawFont_GBK16(120, 50, RED, GRAY0, tbuf);

        sprintf((char*)tbuf, "Xvolt:%5.3fV", volt_x);
        Gui_DrawFont_GBK16(60, 66, RED, GRAY0, tbuf);

        sprintf((char*)tbuf, "Yvolt:%5.3fV", volt_y);
        Gui_DrawFont_GBK16(60, 82, RED, GRAY0, tbuf);

        sprintf((char*)tbuf, "Xnorm:%5d", norm_x);
        Gui_DrawFont_GBK16(60, 98, RED, GRAY0, tbuf);

        sprintf((char*)tbuf, "Ynorm:%5d", norm_y);
        Gui_DrawFont_GBK16(120, 98, RED, GRAY0, tbuf);

        sprintf((char*)tbuf, "BTN1:%1d ", btn1 == 0 ? 1 : 0);
        Gui_DrawFont_GBK16(60, 114, RED, GRAY0, tbuf);

        sprintf((char*)tbuf, "BTN2:%1d ", btn2 == 0 ? 1 : 0);
        Gui_DrawFont_GBK16(102, 114, RED, GRAY0, tbuf);

        sprintf((char*)tbuf, "DIR:%1d%1d", dirp == 0 ? 1 : 0, dirn == 0 ? 1 : 0);
        Gui_DrawFont_GBK16(144, 114, RED, GRAY0, tbuf);

        // ---- Serial output (115200, 8N1) ----
        printf("Xraw=%04d  Yraw=%04d  Xv=%5.3fV  Yv=%5.3fV  Xn=%5d  Yn=%5d  BTN=%d%d DIR=%d%d\r\n",
               adcx_raw, adcy_raw,
               volt_x, volt_y,
               norm_x, norm_y,
               btn1 == 0 ? 1 : 0,
               btn2 == 0 ? 1 : 0,
               dirp == 0 ? 1 : 0,
               dirn == 0 ? 1 : 0);

        LED1 = !LED1;
        delay_ms(200);
    }
}
