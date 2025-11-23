#include "keyboard.h"

#include "buttons.h"
#include "debug.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdint.h>

#define PRESS_CHECK_TIMEOUT_STICKS  (10u)
#define LONG_PRESS_TIMEOUT_STICKS   (2000u)

static TaskHandle_t keyboard_th = NULL;

static key_state_s k_state;

static void keyboard_task(void * args)
{
    (void)args;

    uint32_t button_id;
    const uint32_t ulBitsToClearOnEntry = 0;
    const uint32_t ulBitsToClearOnExit = 0;

    for (;;)
    {
        if(xTaskNotifyWait(
            ulBitsToClearOnEntry,
            ulBitsToClearOnExit, 
            &button_id, 
            portMAX_DELAY) == pdTRUE) 
        {
            if (k_state.pressed == false)
            {
                k_state.button_id = button_id;
                k_state.pressed = true;
                k_state.long_pressed = false;
                k_state.released = false;
                DBG_INFO("Key %u pressed\n", button_id);
                vTaskDelay(20); // debounce
                uint32_t long_press_counter = LONG_PRESS_TIMEOUT_STICKS / PRESS_CHECK_TIMEOUT_STICKS;
                while (buttons_is_pressed(button_id) && long_press_counter)
                {
                    vTaskDelay(PRESS_CHECK_TIMEOUT_STICKS);
                    long_press_counter--;
                }

                if (!long_press_counter && buttons_is_pressed(button_id))
                {
                    k_state.long_pressed = true;
                    DBG_INFO("Key %u long pressed\n", button_id);
                }

                while (buttons_is_pressed(button_id))
                {
                    vTaskDelay(PRESS_CHECK_TIMEOUT_STICKS);
                }
                k_state.released = true;
                k_state.pressed = false;
                DBG_INFO("Key %u released\n", button_id);
            }
            buttons_irq_enable();
        }
    }
}

TaskHandle_t keyboard_task_handle_get(void)
{
    return keyboard_th;
}

bool keyboard_key_is_pressed(buttons_e * key, bool reset_state)
{
    bool pressed = false;

    if (k_state.pressed)
    {
        pressed = true;
        *key = k_state.button_id;
    
        if (reset_state)
        {
            k_state.pressed = false;
        }
    }

    return pressed;
}

/* Function that creates a task. */
void keyboard_service(void)
{
    BaseType_t xReturned;

    /* Create the task, storing the handle. */
    xReturned = xTaskCreate(
                    keyboard_task,        /* Function that implements the task. */
                    "kbd",             /* Text name for the task. */
                    256,                /* Stack size in words, not bytes. */
                    ( void * ) NULL,     /* Parameter passed into the task. */
                    tskIDLE_PRIORITY + 2,   /* Priority at which the task is created. */
                    &keyboard_th);         /* Used to pass out the created task's handle. */

    if (xReturned != pdPASS)
    {
        DBG_ERR("Keyboard task failed\n");
    }
}