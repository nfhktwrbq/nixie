
#include "sys_init.h"
#include "app/sensor.h"
#include "debug.h"

#include "FreeRTOS.h"
#include "task.h"




int main(void)
{
    system_init();    

    DBG_INFO("Hello\n");    
    
    app_sensor(&(app_sens_cfg_s){ .i2c = I2C1, .meas_period_ms = 1000 });

    vTaskStartScheduler();

    while (1)
    {
    }
}