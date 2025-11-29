#include "gpio.h"

#include <stddef.h>

// Функция инициализации пина как выхода
void gpio_output_init(GPIO_TypeDef* GPIOx, uint8_t pin, gpio_cfg_e cfg, gpio_speed_e speed) 
{
    volatile uint32_t * config_register = NULL;
    const uint32_t pins_per_reg = 8;

    if (cfg > GPIO_MODE_MAX)
    {
        cfg = GPIO_MODE_PUSH_PULL;
    }

    if (speed < GPIO_SPEED_MIN || speed > GPIO_SPEED_MAX)
    {
        speed = GPIO_SPEED_2_MHZ;
    }
    
    // Выбираем регистр конфигурации (CRL или CRH)
    if (pin < pins_per_reg) 
    {
        config_register = &GPIOx->CRL;
    } 
    else 
    {
        config_register = &GPIOx->CRH;
        pin -= pins_per_reg;  // Для CRH позиция 0-7
    }
    
    // Создаем маску и значение для конфигурации
    const uint32_t config_mask = GPIO_CRL_MODE0 | GPIO_CRL_CNF0;
    const uint32_t config_value = (cfg << GPIO_CRL_CNF_Pos) | (speed << GPIO_CRL_MODE_Pos);
    
    // Сбрасываем старую конфигурацию и устанавливаем новую
    *config_register &= ~(config_mask << (pin * 4));
    *config_register |= (config_value << (pin * 4));
}

// Установить вывод в 1 (SET)
void gpio_set(GPIO_TypeDef* GPIOx, uint8_t pin) 
{
    GPIOx->BSRR = (1 << pin);
}

// Установить вывод в 0 (RESET) 
void gpio_reset(GPIO_TypeDef* GPIOx, uint8_t pin) 
{
    GPIOx->BRR = (1 << pin);
}

// Инвертировать состояние вывода
void gpio_toggle(GPIO_TypeDef* GPIOx, uint8_t pin) 
{
    GPIOx->ODR ^= (1 << pin);
}

// Прочитать состояние вывода
uint8_t gpio_read(GPIO_TypeDef* GPIOx, uint8_t pin) 
{
    return (GPIOx->IDR & (1 << pin));
}