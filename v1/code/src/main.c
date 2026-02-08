
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
#if 0
    static uint32_t prev_timestamp = 0;
    static uint32_t ddd = 0;

    uint32_t timestamp = rtc_buferized_cnt_get();
    if (prev_timestamp != timestamp)
    {
        prev_timestamp = timestamp;
        time_s time = {0};        
        datetime_time_from_timestamp(timestamp, &time);
        
        // if (timestamp%2)
        // {
        //     display_uint_set(timestamp);
        // }
        // else
        // {
        //     uint32_t sd = sensor_service_get();
        //     display_uint_set(sd);
        // }

        ddd++;
        ddd %= 11;
        display_char_set('0' + ddd, 0, false, false);
        display_char_set('1'+ ddd, 1, false, false);
        display_char_set('2'+ ddd, 2, false, false);
        display_char_set('3'+ ddd, 3, false, false);
        display_second_set(ddd%2);
        
        // display_dot_set(0, ddd%2);
        // display_dot_set(1, ddd%2);
        // display_dot_set(2, ddd%2);
        // display_dot_set(3, ddd%2);
        // display_second_set(ddd%2);


        DBG_INFO("%02u:%02u:%02u (%u)\n", time.hour, time.minute, time.second, timestamp);
        buttons_e btn;
        if (keyboard_key_is_pressed(&btn, true))
        {
            if (btn == BUTTON_OK)
            {
                settings.meas_period_ms = 15000;
                settings_save(&settings);
            }
        }
    }
#endif

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