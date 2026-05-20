#ifndef __JOYSTICK_H
#define __JOYSTICK_H
#include "sys.h"

// ============================================================
// JC2000 Joystick Pin Mapping (8-wire cable)
// Reference: PG Drives Technology VR2 joystick interface standard
//
// Cable color reference (verify with multimeter!):
//   Red    -> +5V  (power supply, or 3.3V if using STM32 supply)
//   Black  -> GND
//   White  -> X-axis output (left/right, analog 0~3.3V or 0~5V)
//   Yellow -> Y-axis output (forward/back, analog 0~3.3V or 0~5V)
//   Green  -> Button 1 / function switch 1
//   Blue   -> Button 2 / function switch 2
//   Orange -> Direction+ (up/forward, digital switch)
//   Purple -> Direction- (down/back, digital switch)
//
// ADC channel mapping for STM32F405RG:
//   X-axis -> PB0 = ADC1_IN8   (Channel 8)
//   Y-axis -> PB1 = ADC1_IN9   (Channel 9)
//   Extra  -> PA4 = ADC1_IN4   (Channel 4) -- reserved for 3rd axis or button
//
// Digital switch pins (GPIOs, use internal pull-up):
//   BTN1   -> PC4  (Green wire)
//   BTN2   -> PC5  (Blue wire)
//   DIR_P  -> PA4  (Orange wire)  -- or shared with extra ADC
//   DIR_N  -> PA5  (Purple wire)
// ============================================================

// ADC GPIO ports
#define JOY_ADC_X_PORT   GPIOB
#define JOY_ADC_Y_PORT   GPIOB
#define JOY_ADC_X_PIN    GPIO_Pin_0    // PB0 = ADC1_IN8
#define JOY_ADC_Y_PIN    GPIO_Pin_1    // PB1 = ADC1_IN9

// ADC channels (used in ADC_RegularChannelConfig)
#define JOY_ADC_CH_X     ADC_Channel_8  // PB0
#define JOY_ADC_CH_Y     ADC_Channel_9  // PB1

// Digital switch GPIO ports (use as INPUT with Pull-Up)
#define JOY_BTN1_PORT    GPIOC
#define JOY_BTN1_PIN     GPIO_Pin_4    // PC4 = ADC1_IN14 (can also read as GPIO)

#define JOY_BTN2_PORT    GPIOC
#define JOY_BTN2_PIN     GPIO_Pin_5    // PC5 = ADC1_IN15 (can also read as GPIO)

#define JOY_DIRP_PORT    GPIOA
#define JOY_DIRP_PIN     GPIO_Pin_4    // PA4 = ADC1_IN4

#define JOY_DIRN_PORT    GPIOA
#define JOY_DIRN_PIN     GPIO_Pin_5    // PA5 = ADC1_IN5

// Voltage reference - set to match JC2000 output voltage
// For 3.3V-powered JC2000: use 3.3f
// For 5V-powered JC2000:   use 3.3f (after voltage divider 10k+10k) or 5.0f if directly connected (NOT recommended!)
#ifndef JOYSTICK_VREF
#define JOYSTICK_VREF    3.3f         // ADC reference voltage (Volts)
#endif

// ADC resolution (12-bit on STM32F4)
#define JOYSTICK_ADC_MAX 4095

// Raw ADC center value (for 12-bit, no load)
#define JOYSTICK_CENTER  2047

// Deadzone threshold (ignore small variations around center)
#define JOYSTICK_DEADZONE 50

// ============================================================
// Function prototypes
// ============================================================

// Initialize all joystick pins (ADC + GPIO)
void Joystick_Init(void);

// Read raw ADC value for X-axis
u16 Joystick_GetAdcX(void);

// Read raw ADC value for Y-axis
u16 Joystick_GetAdcY(void);

// Read raw X with oversampling (average of N samples)
u16 Joystick_GetAdcX_Avg(u8 times);

// Read raw Y with oversampling (average of N samples)
u16 Joystick_GetAdcY_Avg(u8 times);

// Get voltage value for X-axis (Volts)
float Joystick_GetVoltageX(void);

// Get voltage value for Y-axis (Volts)
float Joystick_GetVoltageY(void);

// Get normalized X value: -1000 ~ +1000 (center = 0)
// Returns signed value with deadzone applied
s16 Joystick_GetNormalizedX(void);

// Get normalized Y value: -1000 ~ +1000 (center = 0)
// Returns signed value with deadzone applied
s16 Joystick_GetNormalizedY(void);

// Read button states (0 = pressed, 1 = released, with internal pull-up)
u8  Joystick_GetBtn1(void);   // Green wire
u8  Joystick_GetBtn2(void);   // Blue wire
u8  Joystick_GetDirP(void);   // Orange wire (direction+)
u8  Joystick_GetDirN(void);   // Purple wire (direction-)

#endif
