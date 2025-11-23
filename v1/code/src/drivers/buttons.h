#pragma once

#include <stdbool.h>

typedef enum buttons_e
{
    BUTTON_MIN = 0,
    BUTTON_OK = BUTTON_MIN,
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTONS_QTY,
} buttons_e;


void buttons_irq_disable(void);
void buttons_irq_enable(void);
bool buttons_is_pressed(buttons_e key);
void buttons_debug_print(void);
void test_buttons_directly(void);