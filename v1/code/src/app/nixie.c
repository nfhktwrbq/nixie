#include "nixie.h"

#include "app_display.h"
#include "nixie_menu.h"

#include "drivers/display.h"
#include "software/datetime.h"
#include "utils/macro.h"
#include "modules/rtc.h"
#include "drivers/buttons.h"
#include "services/keyboard.h"

#include "FreeRTOS.h"
#include "task.h"


void timedate_show(nixie_cfg_s * cfg)
{
    static datetime_s datetime;
    static uint32_t timestamp = 0;

    uint32_t cur_timestamp = rtc_buferized_cnt_get();
    if (cur_timestamp != timestamp)
    {
        timestamp = cur_timestamp;
        datetime_from_timestamp(timestamp, &datetime);
        DBG_INFO("%02u:%02u:%02u %02u.%02u.%04u (%u)\n", 
            datetime.hour, 
            datetime.minute,
            datetime.second,
            datetime.day,
            datetime.month,
            datetime.year,
            timestamp
        );

        display_second_set(true);
        app_display_time_set(&datetime);
        vTaskDelay(pdMS_TO_TICKS(500));
        display_second_set(false);

        // app_display_relax();
        if ((timestamp % cfg->settings->relax_period_s) == 0)
        {
            app_display_relax();
        }

        if (cfg->settings->show_date_period_s && 
            ((timestamp % cfg->settings->show_date_period_s) == 0))
        {
            display_second_set(true);
            app_display_month_day_set(&datetime);
            vTaskDelay(pdMS_TO_TICKS(2000));
            display_second_set(false);              
            app_display_year_set(&datetime);
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }
    vTaskDelay(1);
}

void nixie_task(void * args)
{
    nixie_cfg_s * cfg = (nixie_cfg_s *)args;

    cfg->i2c->delay_ms = vTaskDelay;

    buttons_e btn;

    bool is_in_menu = false;

    nixie_menu_init();

    for (;;)
    {        
        if (is_in_menu)
        {
            nixie_menu_handle();
            is_in_menu = false;
        }
        else
        {
            bool btn_pressed = (keyboard_key_is_pressed(&btn)); 

            timedate_show(cfg);
            if (btn_pressed && btn == BUTTON_LEFT)
            {
                is_in_menu = true;
            }
        }
    }
}

void app_nixie(nixie_cfg_s * cfg)
{
    BaseType_t xReturned;

    nixie_cfg_s * config = pvPortMalloc(sizeof(nixie_cfg_s));
    *config = *cfg;

    xReturned = xTaskCreate(
                    nixie_task,             /* Function that implements the task. */
                    "nixie",                /* Text name for the task. */
                    256,                    /* Stack size in words, not bytes. */
                    ( void * ) config,        /* Parameter passed into the task. */
                    tskIDLE_PRIORITY + 2,   /* Priority at which the task is created. */
                    NULL);          /* Used to pass out the created task's handle. */

    if (xReturned != pdPASS)
    {
        DBG_ERR("nixie task failed\n");
    }
}