#include "sys_init.h"

#include "stm_defines.h"
#include "core/core_cm3.h"

#include "uart.h"
#include "rtc.h"
#include "i2c.h"

#include <stdint.h>
#include <string.h>

#include "FreeRTOSConfig.h"


#define SYSTEM_CORE_CLOCK_HZ   (72000000u)      // todo defined by system_clock_config_ll hardcoded
#define APB1_CLOCK_HZ          (36000000u)      // todo defined by system_clock_config_ll hardcoded
#define APB2_CLOCK_HZ          (72000000u)      // todo defined by system_clock_config_ll hardcoded
#define LSE_CLOCK_HZ           (32768u)         // todo defined by system_clock_config_ll hardcoded

uint32_t cc_sys_clk_hz_get(void)
{
    return SYSTEM_CORE_CLOCK_HZ;
}

uint32_t cc_i2c_clk_hz_get(void)
{
    return APB1_CLOCK_HZ;
}

uint32_t cc_rtc_clk_hz_get(void)
{
    return LSE_CLOCK_HZ;
}

uint32_t cc_dbg_uart_clk_hz_get(void)
{
    return APB2_CLOCK_HZ;
}

static void system_clock_config_ll(void)
{
    FLASH->ACR |= FLASH_ACR_PRFTBE;  // Enable Prefetch Buffer
    FLASH->ACR |= FLASH_ACR_LATENCY_2; // 2 wait states for 48-72 MHz

    // Enable HSE
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY))
        ;

    // Configure PLL
    RCC->CFGR |= RCC_CFGR_PLLSRC;  // Set PLL source to HSE
    RCC->CFGR |= RCC_CFGR_PLLMULL9; // Set PLL multiplication factor to 9
    RCC->CR |= RCC_CR_PLLON;        // Enable PLL
    while (!(RCC->CR & RCC_CR_PLLRDY))
        ;

    RCC->CR &= ~RCC_CR_HSION;  // Disable of HSI

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

static void uart_init_ll(void)
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

static void buttons_init_ll(void)
{
    // Release PB3 from SWO
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_1; 

    // Enable clocks
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | 
                    RCC_APB2ENR_IOPCEN | RCC_APB2ENR_AFIOEN;

    // Configure GPIOs as inputs PA0-2
    GPIOA->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0 | GPIO_CRL_MODE1 | GPIO_CRL_CNF1 | GPIO_CRL_MODE2 | GPIO_CRL_CNF2);
    GPIOA->CRL |= (GPIO_CRL_CNF0_0 | GPIO_CRL_CNF1_0 | GPIO_CRL_CNF2_0);
    GPIOA->ODR &= ~(GPIO_ODR_ODR0 | GPIO_ODR_ODR1 | GPIO_ODR_ODR2);
    
    // PB3
    GPIOB->CRL &= ~(GPIO_CRL_MODE3 | GPIO_CRL_CNF3);
    GPIOB->CRL |= GPIO_CRL_CNF3_0;
    GPIOB->ODR &= ~GPIO_ODR_ODR3;
    
    // PC13
    GPIOC->CRH &= ~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13);
    GPIOC->CRH |= GPIO_CRH_CNF13_0;
    GPIOC->ODR &= ~GPIO_ODR_ODR13;

    // 3. Connect GPIOs to EXTI lines
    // PA0 -> EXTI0 (EXTICR[0] for EXTI0-3)
    AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI0;
    AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI0_PA;
    
    // PA1 -> EXTI1
    AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI1;
    AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI1_PA;
    
    // PA2 -> EXTI2
    AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI2;
    AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI2_PA;
    
    // PB3 -> EXTI3
    AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI3;
    AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI3_PB;
    
    // PC13 -> EXTI13 (EXTICR[3] for EXTI12-15)
    AFIO->EXTICR[3] &= ~AFIO_EXTICR4_EXTI13;
    AFIO->EXTICR[3] |= AFIO_EXTICR4_EXTI13_PC;

    // 4. Configure trigger edges (enable both rising & falling for all)
    EXTI->RTSR |= EXTI_RTSR_TR0 | EXTI_RTSR_TR1 | EXTI_RTSR_TR2 | 
                  EXTI_RTSR_TR3 | EXTI_RTSR_TR13;
    // EXTI->FTSR |= EXTI_FTSR_TR0 | EXTI_FTSR_TR1 | EXTI_FTSR_TR2 | 
    //               EXTI_FTSR_TR3 | EXTI_FTSR_TR13;
    
    // 5. Unmask all EXTI lines
    EXTI->IMR |= EXTI_IMR_MR0 | EXTI_IMR_MR1 | EXTI_IMR_MR2 | 
                 EXTI_IMR_MR3 | EXTI_IMR_MR13;
    
    // 6. Configure NVIC for all interrupts
    // Individual handlers for EXTI0, EXTI1, EXTI2, EXTI3
    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(EXTI1_IRQn);
    NVIC_EnableIRQ(EXTI2_IRQn);
    NVIC_EnableIRQ(EXTI3_IRQn);
    
    // Grouped handler for EXTI15_10 (includes EXTI13)
    NVIC_EnableIRQ(EXTI15_10_IRQn);
    
    // Set priorities if needed
    NVIC_SetPriority(EXTI0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);
    NVIC_SetPriority(EXTI1_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);
    NVIC_SetPriority(EXTI2_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);
    NVIC_SetPriority(EXTI3_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);
    NVIC_SetPriority(EXTI15_10_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);
}

static void i2c_init_ll(void)
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

static void systick_init_ll(uint32_t ticks)
{
    #if 0
    (void)ticks;
    #else
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
    #endif
}

static void periphery_init(void)
{
    i2c_init_ll();
    uart_init_ll();
    buttons_init_ll();

    uart_init(921600);
    rtc_init();        
}


void system_init_(void)
{
#ifdef DEBUG
    // Enable fault handlers
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk | 
                  SCB_SHCSR_BUSFAULTENA_Msk | 
                  SCB_SHCSR_USGFAULTENA_Msk;
#endif
    system_clock_config_ll();    
    systick_init_ll(SYSTEM_CORE_CLOCK_HZ / 1000); 

    buttons_init_ll();
    
    // __enable_irq();
}

void system_init(void)
{
#ifdef DEBUG
    // Enable fault handlers
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk | 
                  SCB_SHCSR_BUSFAULTENA_Msk | 
                  SCB_SHCSR_USGFAULTENA_Msk;
#endif
    system_clock_config_ll();    
    systick_init_ll(SYSTEM_CORE_CLOCK_HZ / 1000); 

    periphery_init();
    
    __enable_irq();
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