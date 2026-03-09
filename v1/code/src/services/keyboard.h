#pragma once

#include "drivers/buttons.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdbool.h>


TaskHandle_t keyboard_task_handle_get(void);
void keyboard_service(void);
bool keyboard_key_is_pressed(buttons_e * key);
bool keyboard_key_is_released(void);