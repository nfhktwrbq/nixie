#include "app_display.h"

#include "drivers/display.h"
#include "software/datetime.h"
#include "utils/macro.h"

#include "FreeRTOS.h"
#include "task.h"

void app_display_relax(void)
{
    for (char c = '0'; c <= '9'; c++)
    {
        for (uint32_t pos = 0; pos < DISPLAY_WIDTH; pos++)
        {
            display_second_set(pos % 2);
            display_char_set_ex(c, pos, !(c % 2), DISPLAY_NO_BLINK);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void app_display_time_set(datetime_s * datetime)
{
    char time_str[4];
    
    time_str[0] = DIG_TO_CHAR(datetime->minute % 10);
    time_str[1] = DIG_TO_CHAR(datetime->minute / 10);
    time_str[2] = DIG_TO_CHAR(datetime->hour % 10);
    time_str[3] = DIG_TO_CHAR_EMPTY_ZERO(datetime->hour / 10);

    for (uint32_t pos = 0; pos < MIN(DISPLAY_WIDTH, ARRAY_ITEMS_QTY(time_str)); pos++)
    {
        display_char_set(time_str[pos], pos);
    }
}

void app_display_month_day_set(datetime_s * datetime)
{
    char time_str[4];

    time_str[0] = DIG_TO_CHAR(datetime->month % 10);
    time_str[1] = DIG_TO_CHAR_EMPTY_ZERO(datetime->month / 10);
    time_str[2] = DIG_TO_CHAR(datetime->day % 10);
    time_str[3] = DIG_TO_CHAR_EMPTY_ZERO(datetime->day / 10);

    for (uint32_t pos = 0; pos < MIN(DISPLAY_WIDTH, ARRAY_ITEMS_QTY(time_str)); pos++)
    {
        display_char_set(time_str[pos], pos);
    }
}

void app_display_year_set(datetime_s * datetime)
{
    char time_str[4];

    time_str[0] = DIG_TO_CHAR(datetime->year / 1 % 10);
    time_str[1] = DIG_TO_CHAR(datetime->year / 10 % 10);
    time_str[2] = DIG_TO_CHAR(datetime->year / 100 % 10);
    time_str[3] = DIG_TO_CHAR(datetime->year / 1000 % 10);

    for (uint32_t pos = 0; pos < MIN(DISPLAY_WIDTH, ARRAY_ITEMS_QTY(time_str)); pos++)
    {
        display_char_set(time_str[pos], pos);
    }
}