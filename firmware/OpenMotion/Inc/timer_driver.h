#ifndef TIMER_DRIVER_H
#define TIMER_DRIVER_H

#include "stm32f4xx.h"
#include <stdint.h>

/*----------------------------------------------------------------
 * SERVICE 1 — Microsecond timebase (TIM2)
 *----------------------------------------------------------------*/
void     TIM2_Init(void);
void     Delay_us(uint32_t us);
uint32_t TIM2_GetMicros(void);

/*----------------------------------------------------------------
 * SERVICE 2 — PWM Output and Step Control (TIM3)
 *----------------------------------------------------------------*/
void TIM3_PWM_Init(void);
void TIM3_SetStepperDelay(uint32_t delay_us);
void TIM3_SetServoPulse(uint16_t pulse_us);
void TIM3_StepperStart(uint32_t steps, uint32_t delay_us); /* Fixed: takes speed */
void TIM3_StepperStop(void);

#endif /* TIMER_DRIVER_H */
