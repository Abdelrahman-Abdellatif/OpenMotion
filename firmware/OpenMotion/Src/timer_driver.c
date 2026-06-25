#include "timer_driver.h"
#include "gpio_driver.h"

/*================================================================
 * PRIVATE VARIABLES & FORWARD DECLARATIONS
 *================================================================*/

/* Tracks how many step pulses are left to send before automatically stopping */
static volatile uint32_t steps_remaining = 0;

/* Hidden helper function: only visible inside this file */
static void TIM3_SetStepperFrequency(uint32_t freq_hz);

/* Public flag: 1 = motor just finished a move. Main loop resets this to 0. */
volatile uint8_t motor_move_complete_flag = 0;
/*================================================================
 * SERVICE 1 — TIM2 Microsecond Timebase
 *================================================================*/

void TIM2_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    (void)RCC->APB1ENR;

    TIM2->PSC = 84 - 1;
    TIM2->ARR = 0xFFFFFFFF;
    TIM2->CNT = 0;
    TIM2->CR1 |= TIM_CR1_CEN;
}

void Delay_us(uint32_t us)
{
    uint32_t start = TIM2->CNT;
    while ((TIM2->CNT - start) < us);
}

uint32_t TIM2_GetMicros(void)
{
    return TIM2->CNT;
}


/*================================================================
 * SERVICE 2 — TIM3 PWM Output & Automatic Step Counting
 *================================================================*/

void TIM3_PWM_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    (void)RCC->APB1ENR;

    GPIO_PinConfig_t pwm_ch1 = {
        .port  = GPIOB,
        .pin   = 4,
        .mode  = GPIO_MODE_AF,
        .otype = GPIO_OTYPE_PUSHPULL,
        .speed = GPIO_SPEED_HIGH,
        .pull  = GPIO_PULL_NONE
    };
    GPIO_Init(&pwm_ch1);

    GPIO_PinConfig_t pwm_ch2 = {
        .port  = GPIOB,
        .pin   = 5,
        .mode  = GPIO_MODE_AF,
        .otype = GPIO_OTYPE_PUSHPULL,
        .speed = GPIO_SPEED_HIGH,
        .pull  = GPIO_PULL_NONE
    };
    GPIO_Init(&pwm_ch2);

    GPIOB->AFR[0] &= ~(0xFUL << (4 * 4));
    GPIOB->AFR[0] |=  (0x2UL << (4 * 4));

    GPIOB->AFR[0] &= ~(0xFUL << (5 * 4));
    GPIOB->AFR[0] |=  (0x2UL << (5 * 4));

    TIM3->PSC = 84 - 1;
    TIM3->ARR = 20000 - 1; /* Default servo base */

    TIM3->CCMR1 |= (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1);
    TIM3->CCMR1 |= TIM_CCMR1_OC1PE;

    TIM3->CCMR1 |= (TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1);
    TIM3->CCMR1 |= TIM_CCMR1_OC2PE;

    TIM3->CCER |= TIM_CCER_CC1E;
    TIM3->CCER |= TIM_CCER_CC2E;

    TIM3->CCR1 = 0;
    TIM3->CCR2 = 1500;

    TIM3->CR1 |= TIM_CR1_ARPE;

    NVIC_SetPriority(TIM3_IRQn, 1);
    NVIC_EnableIRQ(TIM3_IRQn);

    TIM3->CR1 |= TIM_CR1_CEN;
}

void TIM3_SetStepperDelay(uint32_t delay_us)
{
    if (delay_us == 0) return;

    uint32_t freq_hz = 1000000UL / delay_us;
    TIM3_SetStepperFrequency(freq_hz);
}

void TIM3_SetServoPulse(uint16_t pulse_us)
{
    if (pulse_us < 500)  pulse_us = 500;
    if (pulse_us > 2500) pulse_us = 2500;

    TIM3->CCR2 = pulse_us;
}

void TIM3_StepperStart(uint32_t steps, uint32_t delay_us)
{
    if (steps == 0 || delay_us == 0) return;

    /* 1. Set the timing FIRST to prevent inheriting dead/servo configurations */
    TIM3_SetStepperDelay(delay_us);

    /* 2. Set up our counter variables */
    steps_remaining = steps;

    /* 3. Clear pending interrupt flags, activate update interrupt, connect pin output */
    TIM3->SR &= ~TIM_SR_UIF;
    TIM3->DIER |= TIM_DIER_UIE;
    TIM3->CCER |= TIM_CCER_CC1E;
}

void TIM3_StepperStop(void)
{
    TIM3->DIER &= ~TIM_DIER_UIE;
    TIM3->CCER &= ~TIM_CCER_CC1E;
    GPIO_WritePin(GPIOB, 4, 0);
    steps_remaining = 0;
}

void TIM3_IRQHandler(void)
{
    if (TIM3->SR & TIM_SR_UIF)
    {
        TIM3->SR &= ~TIM_SR_UIF;

        if (steps_remaining > 0)
        {
            steps_remaining--;

            if (steps_remaining == 0)
            {
                TIM3_StepperStop();
                /* Raise the flag! Main loop will see this and tell Linux */
                motor_move_complete_flag = 1;
            }
        }
    }
}

/*----------------------------------------------------------------
 * PRIVATE HELPERS
 *----------------------------------------------------------------*/
static void TIM3_SetStepperFrequency(uint32_t freq_hz)
{
    if (freq_hz == 0) return;

    uint32_t arr = (1000000UL / freq_hz) - 1;
    TIM3->ARR  = arr;
    TIM3->CCR1 = arr / 2;
}
