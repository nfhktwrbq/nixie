#include "buttons.h"

#include "stm_defines.h"
#include "debug.h"

#include <stdint.h>

typedef struct button_cfg_s
{
    uint32_t pin_num;
    uint32_t exti_imr;
    GPIO_TypeDef * gpio;
} button_cfg_s;

button_cfg_s buttons_cfg[] = 
{
    [BUTTON_OK]    = {
        .pin_num  = 0,
        .exti_imr = EXTI_IMR_MR0,
        .gpio     = GPIOA,
    },
    [BUTTON_UP]    = {
        .pin_num  = 1,
        .exti_imr = EXTI_IMR_MR1,
        .gpio     = GPIOA,
    },
    [BUTTON_DOWN]  = {
        .pin_num  = 2,
        .exti_imr = EXTI_IMR_MR2,
        .gpio     = GPIOA,
    },
    [BUTTON_LEFT]  = {
        .pin_num   = 3,
        .exti_imr = EXTI_IMR_MR3,
        .gpio     = GPIOB,
    },
    [BUTTON_RIGHT] = {
        .pin_num  = 13,
        .exti_imr = EXTI_IMR_MR13,
        .gpio     = GPIOC,
    },
};

void buttons_irq_disable(void)
{
    uint32_t imr_msk = 0;

    for (uint32_t key_num = BUTTON_MIN; key_num < BUTTONS_QTY; key_num++)
    {
        imr_msk |= buttons_cfg[key_num].exti_imr;
    }

    EXTI->IMR &= ~(imr_msk);
}

void buttons_irq_enable(void)
{
    uint32_t imr_msk = 0;

    for (uint32_t key_num = BUTTON_MIN; key_num < BUTTONS_QTY; key_num++)
    {
        imr_msk |= buttons_cfg[key_num].exti_imr;
    }

    EXTI->IMR |= imr_msk;
}

bool buttons_is_pressed(buttons_e key)
{
    if (key < BUTTONS_QTY)
    {    
        if (buttons_cfg[key].gpio->IDR & (1 << buttons_cfg[key].pin_num)) 
        {
            return true;
        }
    }
    return false;
}

void buttons_debug_print(void)
{
    DBG_INFO("\n");
    for (buttons_e key = BUTTON_MIN; key < BUTTONS_QTY; key++)
    {
        const uint32_t idr = buttons_cfg[key].gpio->IDR;
        uint32_t pin_state = idr & (1 << buttons_cfg[key].pin_num);
        (void)pin_state;
        DBG_INFO("Button %d: IDR = 0x%08lX, pin %lu state = %s\n", 
               key, 
               idr,
               buttons_cfg[key].pin_num,
               pin_state ? "PRESSED" : "RELEASED");
    }
}

void test_buttons_directly(void)
{
    DBG_INFO("GPIOA IDR: 0x%04X\n", (uint16_t)GPIOA->IDR);
    DBG_INFO("GPIOB IDR: 0x%04X\n", (uint16_t)GPIOB->IDR); 
    DBG_INFO("GPIOC IDR: 0x%04X\n", (uint16_t)GPIOC->IDR);
    
    // Проверяем конкретные биты
    DBG_INFO("PA0: %d\n", (GPIOA->IDR & GPIO_IDR_IDR0) ? 1 : 0);
    DBG_INFO("PA1: %d\n", (GPIOA->IDR & GPIO_IDR_IDR1) ? 1 : 0);
    DBG_INFO("PA2: %d\n", (GPIOA->IDR & GPIO_IDR_IDR2) ? 1 : 0);
    DBG_INFO("PB3: %d\n", (GPIOB->IDR & GPIO_IDR_IDR3) ? 1 : 0);
    DBG_INFO("PC13: %d\n", (GPIOC->IDR & GPIO_IDR_IDR13) ? 1 : 0);
}