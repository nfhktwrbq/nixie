
#include "sys_init.h"
#include "services/sensor.h"
#include "services/keyboard.h"
#include "debug.h"
#include "drivers/buttons.h"
#include "modules/rtc.h"
#include "drivers/display.h"
#include "drivers/settings.h"
#include "datetime.h"

#include "FreeRTOS.h"
#include "task.h"

static i2c_inst_s i2c_inst = 
{
    .inst = I2C1,
    .inited = false,
};

#define EEPROM_ARRD     (0x57)

static settings_s settings;
static uint32_t prev_timestamp = 0;

void vApplicationIdleHook(void)
{
    uint32_t timestamp = rtc_buferized_cnt_get();
    if (prev_timestamp != timestamp)
    {
        prev_timestamp = timestamp;
        time_s time = {0};
        datetime_time_from_timestamp(timestamp, &time) ;
        DBG_INFO("%2u:%2u:%2u (%u)\n", time.hour, time.minute, time.second, timestamp);
        display_test_ll();
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
}



int main(void)
{
    system_init();    

    DBG_INFO("Hello\n");   
    

    settings_init((settings_iface_s){
            .i2c = &i2c_inst,
            .address = EEPROM_ARRD,
    });
    (void)settings_restore(&settings);

    

    keyboard_service();
    sensor_service(&(app_sens_cfg_s){ .i2c = &i2c_inst, .meas_period_ms = settings.meas_period_ms });

    vTaskStartScheduler();

    while (1)
    {
    }
}