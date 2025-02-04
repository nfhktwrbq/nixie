#include <stdint.h>
#include <string.h>
#include "sys_init.h"
#include "stm_defines.h"
#include "core/core_cm3.h"
#include "FreeRTOS.h"
#include "task.h"


static void system_clock_config_(void)
{
    // Enable HSE
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY))
        ;

    // Configure PLL
    RCC->CFGR &= ~RCC_CFGR_PLLSRC;  // Set PLL source to HSE
    RCC->CFGR |= RCC_CFGR_PLLMULL9; // Set PLL multiplication factor to 9
    RCC->CR |= RCC_CR_PLLON;        // Enable PLL
    while (!(RCC->CR & RCC_CR_PLLRDY))
        ;

    // Select PLL as system clock source
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
        ;

    // Enable PWR
    RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;

    // Enable to write to RCC->BDCR and the backup domain
    PWR->CR |= PWR_CR_DBP;

    // Configure RTC clock source to LSE
    RCC->BDCR |= RCC_BDCR_RTCSEL_0;

    // Enable RTC
    RCC->BDCR |= RCC_BDCR_RTCEN;

    // Enable LSE
    RCC->BDCR |= RCC_BDCR_LSEON;
    while (!(RCC->BDCR & RCC_BDCR_LSERDY))
        ;

    // Select LSE as the RTC clock source
    RCC->BDCR |= RCC_BDCR_RTCSEL_LSE;

    // Enable the RTC clock
    RCC->BDCR |= RCC_BDCR_RTCEN;

    // Configure peripheral clocks
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2; // APB1 clock = HCLK/2
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV1; // APB2 clock = HCLK
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;  // AHB clock = SYSCLK
}

static void uart_init_(void)
{
    // Enable GPIOA clock
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    // Configure PA9 (TX) as Alternate Function Push-Pull
    GPIOA->CRH &= ~(GPIO_CRH_MODE9 | GPIO_CRH_CNF9);    // Clear mode and CNF
    GPIOA->CRH |= (GPIO_CRH_MODE9_1 | GPIO_CRH_CNF9_1); // 2 MHz, AF Push-Pull

    // Configure PA10 (RX) as Input Floating
    GPIOA->CRH &= ~(GPIO_CRH_MODE10 | GPIO_CRH_CNF10); // Clear mode and CNF
    GPIOA->CRH |= GPIO_CRH_CNF10_0;                    // Input floating

    // Enable USART1 clock
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN; 
}

static void i2c_init_(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;

    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    RCC->APB2RSTR |= RCC_APB2RSTR_AFIORST | RCC_APB2RSTR_IOPBRST;
    RCC->APB2RSTR &= ~(RCC_APB2RSTR_AFIORST | RCC_APB2RSTR_IOPBRST);

    GPIOB->CRL &= ~(GPIO_CRL_MODE6 | GPIO_CRL_MODE7 | GPIO_CRL_CNF6 | GPIO_CRL_CNF7);
    GPIOB->CRL |= GPIO_CRL_MODE6 | GPIO_CRL_MODE7 | GPIO_CRL_CNF6 | GPIO_CRL_CNF7;

    NVIC_EnableIRQ(I2C1_EV_IRQn);
    NVIC_EnableIRQ(I2C1_ER_IRQn);
    NVIC_SetPriorityGrouping(3);
}

static void periphery_init_(void)
{
    i2c_init_();
    uart_init_();
}

static void systick_init_(uint32_t ticks)
{
    // Disable SysTick timer
    SysTick->CTRL = 0;

    // Set the reload value
    SysTick->LOAD = ticks - 1;

    // Clear the current value
    SysTick->VAL = 0;

    // Enable SysTick timer with processor clock and enable interrupt
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | // Use processor clock
                    SysTick_CTRL_TICKINT_Msk |   // Enable interrupt
                    SysTick_CTRL_ENABLE_Msk;     // Enable SysTick timer

    NVIC_SetPriority(SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
}

void system_init(void)
{
    system_clock_config_();
    periphery_init_();
    systick_init_(SYSTEM_CORE_CLOCK_HZ / 1000);    
}

void _close(void)
{
}
void _lseek(void)
{
}
void _read(void)
{
}
void _write(void)
{
}