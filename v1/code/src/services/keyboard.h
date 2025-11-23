#pragma once

#include "drivers/buttons.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdbool.h>

typedef struct key_state_s
{
    buttons_e button_id;
    bool pressed;
    bool released;
    bool long_pressed;
} key_state_s;


TaskHandle_t keyboard_task_handle_get(void);
void keyboard_service(void);