
#include "sys_init.h"
#include "services/sensor.h"
#include "services/keyboard.h"
#include "debug.h"
#include "drivers/buttons.h"
#include "modules/rtc.h"
#include "drivers/display.h"

#include "FreeRTOS.h"
#include "task.h"


void vApplicationIdleHook(void)
{
    if (rtc_second_flag_get())
    {
        DBG_INFO("+\n");
        display_test_ll();
    }
}

int main(void)
{
    system_init();    

    DBG_INFO("Hello\n");    
    
    // for (uint32_t key_num = BUTTON_MIN; key_num < BUTTONS_QTY; key_num++)
    // {
    //     if (buttons_is_pressed(key_num))
    //     {
    //         DBG_INFO("Pressed %u\n", key_num); 
    //     }
    // }
    
    // while(1)
    // {
    //     // buttons_debug_print();
    //     test_buttons_directly();
    //     for (volatile uint32_t i = 0; i < 1000000; i++);
    // }


    keyboard_service();
    sensor_service(&(app_sens_cfg_s){ .i2c = I2C1, .meas_period_ms = 10000 });

    vTaskStartScheduler();

    while (1)
    {
    }
}