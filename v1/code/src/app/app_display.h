#pragma once

#include "app_display.h"

#include "software/datetime.h"

void app_display_relax(void);
void app_display_time_set(datetime_s * datetime);
void app_display_month_day_set(datetime_s * datetime);
void app_display_year_set(datetime_s * datetime);