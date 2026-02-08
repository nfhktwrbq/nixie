#include "nixie.h"

#include "drivers/display.h"
#include "software/datetime.h"
#include "utils/macro.h"
#include "modules/rtc.h"

#include "FreeRTOS.h"
#include "task.h"


static void nixie_relax(void)
{

    for (char c = '0'; c <= '9'; c++)
    {
        for (uint32_t pos = 0; pos < DISPLAY_WIDTH; pos++)
        {
            display_second_set(pos % 2);
            display_char_set(c, pos, c % 2, DISPLAY_NO_BLINK);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

static void display_time_set(datetime_s * datetime)
{
    char time_str[4];
    
    time_str[0] = DIG_TO_CHAR(datetime->minute % 10);
    time_str[1] = DIG_TO_CHAR(datetime->minute / 10);
    time_str[2] = DIG_TO_CHAR(datetime->hour % 10);
    time_str[3] = DIG_TO_CHAR_EMPTY_ZERO(datetime->hour / 10);

    for (uint32_t pos = 0; pos < MIN(DISPLAY_WIDTH, ARRAY_ITEMS_QTY(time_str)); pos++)
    {
        display_char_set(time_str[pos], pos, DISPLAY_NO_DOT, DISPLAY_NO_BLINK);
    }
}

static void display_month_day_set(datetime_s * datetime)
{
    char time_str[4];

    time_str[0] = DIG_TO_CHAR(datetime->month % 10);
    time_str[1] = DIG_TO_CHAR_EMPTY_ZERO(datetime->month / 10);
    time_str[2] = DIG_TO_CHAR(datetime->day % 10);
    time_str[3] = DIG_TO_CHAR_EMPTY_ZERO(datetime->day / 10);

    for (uint32_t pos = 0; pos < MIN(DISPLAY_WIDTH, ARRAY_ITEMS_QTY(time_str)); pos++)
    {
        display_char_set(time_str[pos], pos, DISPLAY_NO_DOT, DISPLAY_NO_BLINK);
    }
}

static void display_year_set(datetime_s * datetime)
{
    char time_str[4];

    time_str[0] = DIG_TO_CHAR(datetime->year / 1 % 10);
    time_str[1] = DIG_TO_CHAR(datetime->year / 10 % 10);
    time_str[2] = DIG_TO_CHAR(datetime->year / 100 % 10);
    time_str[3] = DIG_TO_CHAR(datetime->year / 1000 % 10);

    for (uint32_t pos = 0; pos < MIN(DISPLAY_WIDTH, ARRAY_ITEMS_QTY(time_str)); pos++)
    {
        display_char_set(time_str[pos], pos, DISPLAY_NO_DOT, DISPLAY_NO_BLINK);
    }
}

void nixie_task(void * args)
{
    nixie_cfg_s * cfg = (nixie_cfg_s *)args;

    cfg->i2c->delay_ms = vTaskDelay;

    datetime_s datetime;
    uint32_t timestamp = 0;

    for (;;)
    {
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
            display_time_set(&datetime);
            vTaskDelay(pdMS_TO_TICKS(500));
            display_second_set(false);

            if ((timestamp % cfg->settings->relax_period_s) == 0)
            {
                nixie_relax();
            }

            if (cfg->settings->show_date_period_s && 
                ((timestamp % cfg->settings->show_date_period_s) == 0))
            {
                display_second_set(true);
                display_month_day_set(&datetime);
                vTaskDelay(pdMS_TO_TICKS(2000));
                display_second_set(false);              
                display_year_set(&datetime);
                vTaskDelay(pdMS_TO_TICKS(2000));
            }
        }
        vTaskDelay(1);
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