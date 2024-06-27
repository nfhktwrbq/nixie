#include <stdint.h>
#include "sys_init.h"


// Register definitions for STM32F103C8T6
#define RCC_BASE   0x40021000
#define GPIOC_BASE 0x40011000

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

void SystemInit(void)
{
    init_gpio_pc13();
    volatile uint32_t i = 50;
    for (uint32_t k = 0; k < 10; k++)
    {
        i++;
        toggle_gpio_pc13();
    }
}