#include "timer2.h"

// 10ms tick 计数器
static volatile u32 tim2_tick = 0;

// 以下变量供 CANopen 定时器使用
TIMEVAL last_counter_val = 0;
TIMEVAL elapsed_time = 0;

// Initializes the timer, turn on the interrupt and put the interrupt time to zero
void TIM2_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	/* TIM2 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	/* Enable the TIM2 gloabal Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Compute the prescaler value */
	uint16_t PrescalerValue =840-1; //84MHz/840=100k(��timerscfg.h����һ�¼���)����10us���

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 65535;
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	
	TIM_ClearITPendingBit(TIM2, TIM_SR_UIF);
 
	/* TIM2 enable counter */  //����TIM2
	TIM_Cmd(TIM2, ENABLE);

	/* Preset counter for a safe start */
	TIM_SetCounter(TIM2, 1);

	/* TIM Interrupts enable */
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}

//Set the timer for the next alarm.
void setTimer(TIMEVAL value)
{
  	uint32_t timer = TIM_GetCounter(TIM2);        // Copy the value of the running timer
	elapsed_time += timer - last_counter_val;
	last_counter_val = 65535-value;
	TIM_SetCounter(TIM2, 65535-value);
	TIM_Cmd(TIM2, ENABLE);
	//printf("setTimer %lu, elapsed %lu\r\n", value, elapsed_time);
}

//Return the elapsed time to tell the Stack how much time is spent since last call.
TIMEVAL getElapsedTime(void)
{
  	uint32_t timer = TIM_GetCounter(TIM2);        // Copy the value of the running timer
	if(timer < last_counter_val)
		timer += 65535;
	TIMEVAL elapsed = timer - last_counter_val + elapsed_time;
	//printf("elapsed %lu - %lu %lu %lu\r\n", elapsed, timer, last_counter_val, elapsed_time);
	return elapsed;
}

// 获取 tick 值（10ms 周期计数，供动作步进使用）
u32 get_tim2_tick(void) {
    return tim2_tick;
}

// This function handles Timer 2 interrupt request.
void TIM2_IRQHandler(void)
{
	if(TIM_GetFlagStatus(TIM2, TIM_SR_UIF) == RESET)
		return;
	last_counter_val = 0;
	elapsed_time = 0;
	TIM_ClearITPendingBit(TIM2, TIM_SR_UIF);
	
	// 递增 tick 计数器（供动作模块使用）
	tim2_tick++;
	
	TimeDispatch();
}
