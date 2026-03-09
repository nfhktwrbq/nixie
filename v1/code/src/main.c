
#include "sys_init.h"
#include "app/nixie.h"
#include "services/sensor.h"
#include "services/keyboard.h"
#include "debug.h"
#include "drivers/buttons.h"
#include "modules/rtc.h"
#include "drivers/display.h"
#include "datetime.h"

#include "FreeRTOS.h"
#include "task.h"

static settings_s settings;
static i2c_inst_s i2c_inst = 
{
    .inst = I2C1,
    .inited = false,
};


void vApplicationStackOverflowHook( TaskHandle_t xTask,
                                        char * pcTaskName )
{
    (void)xTask;
    (void)pcTaskName;
}


void vApplicationIdleHook(void)
{

}



int main(void)
{
    system_init(); 
    
    (void) i2c_master_init(&i2c_inst);

    DBG_INFO("Hello\n");   
    
    settings_init((settings_iface_s){
        .i2c = &i2c_inst,
        .address = SETTINGS_EEPROM_ADDR,
        .reg_addr_size = SETTINGS_EEPROM_REG_ADDR_SIZE,
    });

    if (settings_restore(&settings) == SETTINGS_RESTORE_FAILED_DEFAULT_LOADED)
    {
        settings_save(&settings);
    }

    keyboard_service();
    sensor_service(&(sens_cfg_s){ .i2c = &i2c_inst, .settings = &settings });

    app_nixie(&(nixie_cfg_s){ .i2c = &i2c_inst, .settings = &settings });

    vTaskStartScheduler();

    while (1)
    {
    }
}