#include <stdint.h>
#include "isr_handle.h"
#include "sys_init.h"

static void default_handler(void)
{
    while (1)
    {

    }
}

void NMI_Handler(void)
{
    default_handler();
}

void HardFault_Handler(void)
{
    default_handler();
}

void MemManage_Handler(void)
{
    default_handler();
}

void BusFault_Handler(void)
{
    default_handler();
}

void UsageFault_Handler(void)
{
    default_handler();
}

// void SVC_Handler(void)
// {
//     default_handler();
// }

void DebugMon_Handler(void)
{
    default_handler();
}


// void PendSV_Handler(void)
// {
//     default_handler();
// }

// void SysTick_Handler(void)
// {
//     systick_handler();
// }

void WWDG_IRQHandler(void)
{
    default_handler();
}

void PVD_IRQHandler(void)
{
    default_handler();
}

void TAMPER_IRQHandler(void)
{
    default_handler();
}

void RTC_IRQHandler(void)
{
    default_handler();
}

void FLASH_IRQHandler(void)
{
    default_handler();
}

void RCC_IRQHandler(void)
{
    default_handler();
}

void EXTI0_IRQHandler(void)
{
    default_handler();
}

void EXTI1_IRQHandler(void)
{
    default_handler();
}

void EXTI2_IRQHandler(void)
{
    default_handler();
}

void EXTI3_IRQHandler(void)
{
    default_handler();
}

void EXTI4_IRQHandler(void)
{
    default_handler();
}

void DMA1_Channel1_IRQHandler(void)
{
    default_handler();
}

void DMA1_Channel2_IRQHandler(void)
{
    default_handler();
}

void DMA1_Channel3_IRQHandler(void)
{
    default_handler();
}

void DMA1_Channel4_IRQHandler(void)
{
    default_handler();
}

void DMA1_Channel5_IRQHandler(void)
{
    default_handler();
}

void DMA1_Channel6_IRQHandler(void)
{
    default_handler();
}

void DMA1_Channel7_IRQHandler(void)
{
    default_handler();
}

void ADC1_2_IRQHandler(void)
{
    default_handler();
}

void USB_HP_CAN1_TX_IRQHandler(void)
{
    default_handler();
}

void USB_LP_CAN1_RX0_IRQHandler(void)
{
    default_handler();
}

void CAN1_RX1_IRQHandler(void)
{
    default_handler();
}

void CAN1_SCE_IRQHandler(void)
{
    default_handler();
}

void EXTI9_5_IRQHandler(void)
{
    default_handler();
}

void TIM1_BRK_IRQHandler(void)
{
    default_handler();
}

void TIM1_UP_IRQHandler(void)
{
    default_handler();
}

void TIM1_TRG_COM_IRQHandler(void)
{
    default_handler();
}

void TIM1_CC_IRQHandler(void)
{
    default_handler();
}

void TIM2_IRQHandler(void)
{
    default_handler();
}

void TIM3_IRQHandler(void)
{
    default_handler();
}

void TIM4_IRQHandler(void)
{
    default_handler();
}

void I2C1_EV_IRQHandler(void)
{
    default_handler();
}

void I2C1_ER_IRQHandler(void)
{
    default_handler();
}

void I2C2_EV_IRQHandler(void)
{
    default_handler();
}

void I2C2_ER_IRQHandler(void)
{
    default_handler();
}

void SPI1_IRQHandler(void)
{
    default_handler();
}

void SPI2_IRQHandler(void)
{
    default_handler();
}

void USART1_IRQHandler(void)
{
    default_handler();
}

void USART2_IRQHandler(void)
{
    default_handler();
}

void USART3_IRQHandler(void)
{
    default_handler();
}

void EXTI15_10_IRQHandler(void)
{
    default_handler();
}

void RTC_Alarm_IRQHandler(void)
{
    default_handler();
}

void USBWakeUp_IRQHandler(void)
{
    default_handler();
}
