#include <stdint.h>
#include <string.h>
#include "sys_init.h"
#include "stm32f103xb.h"


// Register definitions for STM32F103C8T6
// #define RCC_BASE   0x40021000
// #define GPIOC_BASE 0x40011000

#define RCC_APB2ENR (*(volatile uint32_t *)(RCC_BASE + 0x18))
#define GPIOC_CRH   (*(volatile uint32_t *)(GPIOC_BASE + 0x04))
#define GPIOC_ODR   (*(volatile uint32_t *)(GPIOC_BASE + 0x0C))

void init_gpio_pc13(void)
{
    // Enable the clock for GPIOC peripheral
    RCC_APB2ENR |= (1 << 4);

    // Configure PC13 as output push-pull
    GPIOC_CRH &= ~(0xF << 20);
    GPIOC_CRH |= (0x3 << 20);
}

void toggle_gpio_pc13(void)
{
    // Toggle the state of PC13
    GPIOC_ODR ^= (1 << 13);
}

static void SystemClockConfig(void)
{
    // Enable HSE
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY));

    // Configure PLL
    RCC->CFGR &= ~RCC_CFGR_PLLSRC; // Set PLL source to HSE
    RCC->CFGR |= RCC_CFGR_PLLMULL9; // Set PLL multiplication factor to 9
    RCC->CR |= RCC_CR_PLLON; // Enable PLL
    while (!(RCC->CR & RCC_CR_PLLRDY));

    // Select PLL as system clock source
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

    RCC->APB1ENR |= RCC_APB1ENR_PWREN;

    // Enable to write to RCC->BDCR
    PWR->CR |= PWR_CR_DBP;

    // Configure RTC clock source to LSE
    RCC->BDCR |= RCC_BDCR_RTCSEL_0;

    // Enable RTC
    RCC->BDCR |= RCC_BDCR_RTCEN;

    // Enable LSE
    RCC->BDCR |= RCC_BDCR_LSEON;
    while (!(RCC->BDCR & RCC_BDCR_LSERDY));

    PWR->CR &= ~PWR_CR_DBP;

    // Configure peripheral clocks
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2; // APB1 clock = HCLK/2
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV1; // APB2 clock = HCLK
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1; // AHB clock = SYSCLK
}

void SystemInit(void)
{
    SystemClockConfig();

    init_gpio_pc13();
    volatile uint32_t i = 50;
    for (uint32_t k = 0; k < 10; k++)
    {
        i++;
        toggle_gpio_pc13();
    }

    volatile uint8_t * p8 =   (volatile uint8_t *)(0x8000000);
    memset((void *)p8, 0, 45);
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