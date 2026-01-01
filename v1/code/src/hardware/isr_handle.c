#include "isr_handle.h"

#include "sys_init.h"
#include "debug.h"
#include "services/keyboard.h"
#include "modules/rtc.h"

#include "modules/i2c.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdint.h>

static void default_handler(void)
{
    while (1)
    {

    }
}

#ifdef DEBUG


static void decode_fault_status(uint32_t cfsr, uint32_t hfsr)
{
    // char fault_info[256];
    
    // Memory Management Faults
    if (cfsr & (1 << 0)) {  // IACCVIOL
        DBG_INFO("MMF: Instruction Access Violation\n");
    }
    if (cfsr & (1 << 1)) {  // DACCVIOL
        DBG_INFO("MMF: Data Access Violation\n");
    }
    if (cfsr & (1 << 3)) {  // MUNSTKERR
        DBG_INFO("MMF: MemManage Fault on Unstacking\n");
    }
    if (cfsr & (1 << 4)) {  // MSTKERR
        DBG_INFO("MMF: MemManage Fault on Stacking\n");
    }
    if (cfsr & (1 << 7)) {  // MMARVALID
        DBG_INFO("MMF: MMFAR Valid\n");
    }
    
    // Bus Faults
    if (cfsr & (1 << 8)) {  // IBUSERR
        DBG_INFO("BF: Instruction Bus Error\n");
    }
    if (cfsr & (1 << 9)) {  // PRECISERR
        DBG_INFO("BF: Precise Data Bus Error\n");
    }
    if (cfsr & (1 << 10)) { // IMPRECISERR
        DBG_INFO("BF: Imprecise Data Bus Error\n");
    }
    if (cfsr & (1 << 11)) { // UNSTKERR
        DBG_INFO("BF: Bus Fault on Unstacking\n");
    }
    if (cfsr & (1 << 12)) { // STKERR
        DBG_INFO("BF: Bus Fault on Stacking\n");
    }
    if (cfsr & (1 << 15)) { // BFARVALID
        DBG_INFO("BF: BFAR Valid\n");
    }
    
    // Usage Faults
    if (cfsr & (1 << 16)) { // UNDEFINSTR
        DBG_INFO("UF: Undefined Instruction\n");
    }
    if (cfsr & (1 << 17)) { // INVSTATE
        DBG_INFO("UF: Invalid State\n");
    }
    if (cfsr & (1 << 18)) { // INVPC
        DBG_INFO("UF: Invalid PC Load\n");
    }
    if (cfsr & (1 << 19)) { // NOCP
        DBG_INFO("UF: No Coprocessor\n");
    }
    if (cfsr & (1 << 24)) { // UNALIGNED
        DBG_INFO("UF: Unaligned Access\n");
    }
    if (cfsr & (1 << 25)) { // DIVBYZERO
        DBG_INFO("UF: Division By Zero\n");
    }
    
    // Hard Fault
    if (hfsr & (1 << 30)) { // FORCED
        DBG_INFO("HF: Escalated from other fault\n");
    }
    if (hfsr & (1 << 31)) { // DEBUGEVT
        DBG_INFO("HF: Debug Event\n");
    }
}


void HardFault_Handler_C(uint32_t* stack_pointer)
{
    // print_task_info();

     // Get stacked registers
    uint32_t stacked_r0 = stack_pointer[0];
    uint32_t stacked_r1 = stack_pointer[1];
    uint32_t stacked_r2 = stack_pointer[2];
    uint32_t stacked_r3 = stack_pointer[3];
    uint32_t stacked_r12 = stack_pointer[4];
    uint32_t stacked_lr = stack_pointer[5];
    uint32_t stacked_pc = stack_pointer[6];
    uint32_t stacked_psr = stack_pointer[7];
    
    // Get fault status registers
    uint32_t cfsr = SCB->CFSR;
    uint32_t hfsr = SCB->HFSR;
    uint32_t dfsr = SCB->DFSR;
    uint32_t afsr = SCB->AFSR;
    uint32_t mmfar = SCB->MMFAR;
    uint32_t bfar = SCB->BFAR;
    
    // Create debug info string
    char buffer[512];
    snprintf(buffer, sizeof(buffer),
        "HARDFAULT OCCURRED\n"
        "Stack Frame:\n"
        "R0:  0x%08lX\n"
        "R1:  0x%08lX\n"
        "R2:  0x%08lX\n"
        "R3:  0x%08lX\n"
        "R12: 0x%08lX\n"
        "LR:  0x%08lX\n"
        "PC:  0x%08lX\n"
        "PSR: 0x%08lX\n"
        "CFSR: 0x%08lX\n"
        "HFSR: 0x%08lX\n"
        "DFSR: 0x%08lX\n"
        "AFSR: 0x%08lX\n"
        "MMFAR:0x%08lX\n"
        "BFAR: 0x%08lX\n",
        stacked_r0, stacked_r1, stacked_r2, stacked_r3,
        stacked_r12, stacked_lr, stacked_pc, stacked_psr,
        cfsr, hfsr, dfsr, afsr, mmfar, bfar);
    
    // Print debug info
    DBG_INFO(buffer);
    
    // Decode and print fault details
    decode_fault_status(cfsr, hfsr);
    
    // Infinite loop
    while(1);
}

__attribute__((naked)) void HardFault_Handler(void)
{
    __asm volatile(
        "tst lr, #4\n"           // Check EXC_RETURN in LR bit 2
        "ite eq\n"               // if zero (equal) then
        "mrseq r0, msp\n"       // Main Stack was used, copy MSP to R0
        "mrsne r0, psp\n"       // Process Stack was used, copy PSP to R0
        "b HardFault_Handler_C\n" // Jump to C handler
    );
}

#else // DEBUG


void HardFault_Handler(void)
{
    default_handler();
}

#endif // DEBUG

static void exti_handle(uint32_t pending_bit, buttons_e key)
{
    if (EXTI->PR & pending_bit) 
    {        
        EXTI->PR = pending_bit;
        buttons_irq_disable();
        TaskHandle_t b_th = keyboard_task_handle_get();
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xTaskNotifyFromISR(b_th, key, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}


void NMI_Handler(void)
{
    default_handler();
}

void MemManage_Handler(void)
{
    default_handler();
}

void BusFault_Handler(void)
{
    HardFault_Handler();
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
    rtc_irq_handle();
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
    exti_handle(EXTI_PR_PR0, BUTTON_OK);
}
void EXTI1_IRQHandler(void) 
{
    exti_handle(EXTI_PR_PR1, BUTTON_DOWN);
}

void EXTI2_IRQHandler(void) 
{
    exti_handle(EXTI_PR_PR2, BUTTON_UP);
}

void EXTI3_IRQHandler(void) 
{
    exti_handle(EXTI_PR_PR3, BUTTON_LEFT);
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
#if I2C_USE_IRQ
    i2c_irq_event_handler();
#else
    default_handler();
#endif
}

void I2C1_ER_IRQHandler(void)
{
#if I2C_USE_IRQ
    i2c_irq_err_handler();
#else
    default_handler();
#endif
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
    exti_handle(EXTI_PR_PR13, BUTTON_RIGHT);
    
    // Check other lines in this group if you add more later
    if (EXTI->PR & EXTI_PR_PR10) { EXTI->PR = EXTI_PR_PR10; }
    if (EXTI->PR & EXTI_PR_PR11) { EXTI->PR = EXTI_PR_PR11; }
    if (EXTI->PR & EXTI_PR_PR12) { EXTI->PR = EXTI_PR_PR12; }
    if (EXTI->PR & EXTI_PR_PR14) { EXTI->PR = EXTI_PR_PR14; }
    if (EXTI->PR & EXTI_PR_PR15) { EXTI->PR = EXTI_PR_PR15; }
}

void RTC_Alarm_IRQHandler(void)
{
    default_handler();
}

void USBWakeUp_IRQHandler(void)
{
    default_handler();
}
