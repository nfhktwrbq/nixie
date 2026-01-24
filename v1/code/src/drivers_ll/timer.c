#include "timer.h"

#include <stm_defines.h>
#include "consts.h"
#include "sys_init.h" //todo


void timer_init_ll(uint32_t irq_period_us)
{
    // Enable TIM2 clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // Timer base configuration
    // Prescaler: (1 us per tick)
    TIM2->PSC = cc_tim2_clk_hz_get() / C_HZ_TO_MHZ - 1;   
    
    // Auto-reload: 500 us period
    TIM2->ARR = (irq_period_us & C_TWO_BYTE_MASK) - 1;       

    // Enable update interrupt
    TIM2->DIER |= TIM_DIER_UIE;

    // Enable TIM2
    TIM2->CR1 |= TIM_CR1_CEN;

    // NVIC enable for TIM2 interrupt
    NVIC_EnableIRQ(TIM2_IRQn);
}


