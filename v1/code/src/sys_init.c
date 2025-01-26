#include <stdint.h>
#include <string.h>
#include "sys_init.h"
#include "stm_defines.h"
#include "core/core_cm3.h"


static void system_clock_config(void)
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

static void uart_init(uint32_t baudrate)
{
    // Enable GPIOA clock
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    // Configure PA9 (TX) as Alternate Function Push-Pull
    GPIOA->CRH &= ~(GPIO_CRH_MODE9 | GPIO_CRH_CNF9); // Clear mode and CNF
    GPIOA->CRH |= (GPIO_CRH_MODE9_1 | GPIO_CRH_CNF9_1); // 2 MHz, AF Push-Pull

    // Configure PA10 (RX) as Input Floating
    GPIOA->CRH &= ~(GPIO_CRH_MODE10 | GPIO_CRH_CNF10); // Clear mode and CNF
    GPIOA->CRH |= GPIO_CRH_CNF10_0; // Input floating

    // Enable USART1 clock
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    // Set baud rate
    USART1->BRR = SYSTEM_CORE_CLOCK_HZ / baudrate / 2; // Assuming SystemCoreClock is set to 72MHz
    /// todo
    // uint32_t mantissa = SYSTEM_CORE_CLOCK_HZ / (baudrate * 8);
    // uint32_t fraction = ((SYSTEM_CORE_CLOCK_HZ * 100 / 8 / baudrate ) % 100) * 16 / 100;
    // USART1->BRR = (fraction & 0xF) | (mantissa << 4);

    // Configure USART1
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE; // Enable Transmitter and Receiver
    USART1->CR1 |= USART_CR1_UE; // Enable USART1
}

void uart_send_char(char c)
{
    // Wait until the transmit data register is empty
    while (!(USART1->SR & USART_SR_TXE));
    USART1->DR = c; // Send the character
}

static void i2c_init(void)
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
}

static void periphery_init(void)
{
    i2c_init();
    uart_init(921600);
}

void SystemInit(void)
{
    system_clock_config();
    periphery_init();
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