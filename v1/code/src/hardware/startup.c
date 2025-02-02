#include <stdint.h>
#include "isr_handle.h"
#include "sys_init.h"


extern uint32_t _sidata;
/* start address for the .data section. defined in linker script */
extern uint32_t _sdata;
/* end address for the .data section. defined in linker script */
extern uint32_t _edata;
/* start address for the .bss section. defined in linker script */
extern uint32_t _sbss;
/* end address for the .bss section. defined in linker script */
extern uint32_t _ebss;
extern uint32_t _estack;
extern uint32_t BootRAM;

#define BootRAM 0xF108F85F

void Default_Handler(void);
extern int main(void);

void Reset_Handler(void) {

    __asm volatile(
    ".syntax unified\n\t"
    ".cpu cortex-m3\n\t"
    ".fpu softvfp\n\t"
    ".thumb\n\t"
    );

    /* Copy the data segment initializers from flash to SRAM */
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;

    while (dst < &_edata) {
        *dst++ = *src++;
    }

    /* Zero fill the bss segment. */
    for (dst = &_sbss; dst < &_ebss; dst++) {
        *dst = 0;
    }

    /* Call the clock system intitialization function. */
    SystemInit();

    /* Call the application's entry point. */
    main();
}


__attribute__((used, section(".isr_vector")))
void (* const g_pfnVectors[])(void) = {
    (void (*)(void))(&_estack),
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    0,
    0,
    0,
    0,
    SVC_Handler,
    DebugMon_Handler,
    0,
    PendSV_Handler,
    SysTick_Handler,
    WWDG_IRQHandler,
    PVD_IRQHandler,
    TAMPER_IRQHandler,
    RTC_IRQHandler,
    FLASH_IRQHandler,
    RCC_IRQHandler,
    EXTI0_IRQHandler,
    EXTI1_IRQHandler,
    EXTI2_IRQHandler,
    EXTI3_IRQHandler,
    EXTI4_IRQHandler,
    DMA1_Channel1_IRQHandler,
    DMA1_Channel2_IRQHandler,
    DMA1_Channel3_IRQHandler,
    DMA1_Channel4_IRQHandler,
    DMA1_Channel5_IRQHandler,
    DMA1_Channel6_IRQHandler,
    DMA1_Channel7_IRQHandler,
    ADC1_2_IRQHandler,
    USB_HP_CAN1_TX_IRQHandler,
    USB_LP_CAN1_RX0_IRQHandler,
    CAN1_RX1_IRQHandler,
    CAN1_SCE_IRQHandler,
    EXTI9_5_IRQHandler,
    TIM1_BRK_IRQHandler,
    TIM1_UP_IRQHandler,
    TIM1_TRG_COM_IRQHandler,
    TIM1_CC_IRQHandler,
    TIM2_IRQHandler,
    TIM3_IRQHandler,
    TIM4_IRQHandler,
    I2C1_EV_IRQHandler,
    I2C1_ER_IRQHandler,
    I2C2_EV_IRQHandler,
    I2C2_ER_IRQHandler,
    SPI1_IRQHandler,
    SPI2_IRQHandler,
    USART1_IRQHandler,
    USART2_IRQHandler,
    USART3_IRQHandler,
    EXTI15_10_IRQHandler,
    RTC_Alarm_IRQHandler,
    USBWakeUp_IRQHandler,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    (void (*)(void))BootRAM,
    // Add more interrupt vector entries here
};

__attribute__((weak))
void NMI_Handler(void) {
    Default_Handler();
}
__attribute__((weak))
void HardFault_Handler(void) {
    Default_Handler();
}
__attribute__((weak))
void MemManage_Handler(void) {
    Default_Handler();
}
__attribute__((weak))
void BusFault_Handler(void) {
    Default_Handler();
}
__attribute__((weak))
void UsageFault_Handler(void) {
    Default_Handler();
}
__attribute__((weak))
void SVC_Handler(void) {
    Default_Handler();
}
__attribute__((weak))
void DebugMon_Handler(void) {
    Default_Handler();
}
__attribute__((weak))
void PendSV_Handler(void) {
    Default_Handler();
}
__attribute__((weak))
void SysTick_Handler(void) {
    Default_Handler();
}
__attribute__((weak))
void WWDG_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void PVD_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void TAMPER_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void RTC_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void FLASH_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void RCC_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void EXTI0_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void EXTI1_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void EXTI2_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void EXTI3_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void EXTI4_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void DMA1_Channel1_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void DMA1_Channel2_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void DMA1_Channel3_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void DMA1_Channel4_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void DMA1_Channel5_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void DMA1_Channel6_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void DMA1_Channel7_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void ADC1_2_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void USB_HP_CAN1_TX_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void USB_LP_CAN1_RX0_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void CAN1_RX1_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void CAN1_SCE_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void EXTI9_5_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void TIM1_BRK_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void TIM1_UP_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void TIM1_TRG_COM_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void TIM1_CC_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void TIM2_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void TIM3_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void TIM4_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void I2C1_EV_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void I2C1_ER_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void I2C2_EV_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void I2C2_ER_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void SPI1_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void SPI2_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void USART1_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void USART2_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void USART3_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void EXTI15_10_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void RTC_Alarm_IRQHandler(void) {
    Default_Handler();
}
__attribute__((weak))
void USBWakeUp_IRQHandler(void) {
    Default_Handler();
}

void Default_Handler(void) {
    // Default interrupt handler implementation
    while (1) {
    }
}