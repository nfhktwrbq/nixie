#pragma once

#include "stm_defines.h"

#include <stdint.h>

typedef enum gpio_cfg_e
{
    GPIO_MODE_PUSH_PULL = 0,
    GPIO_MODE_OPEN_DRAIN = 1,
    GPIO_MODE_MIN = GPIO_MODE_PUSH_PULL,
    GPIO_MODE_MAX = GPIO_MODE_OPEN_DRAIN,
} gpio_cfg_e;

typedef enum gpio_speed_e
{
    GPIO_SPEED_10_MHZ = 1,
    GPIO_SPEED_2_MHZ = 2,
    GPIO_SPEED_50_MHZ = 3,
    GPIO_SPEED_MIN = GPIO_SPEED_10_MHZ,
    GPIO_SPEED_MAX = GPIO_SPEED_50_MHZ,
} gpio_speed_e;

void gpio_output_init(GPIO_TypeDef* GPIOx, uint8_t pin, gpio_cfg_e cfg, gpio_speed_e speed);
void gpio_toggle(GPIO_TypeDef* GPIOx, uint8_t pin);