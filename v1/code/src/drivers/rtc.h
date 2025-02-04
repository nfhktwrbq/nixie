#pragma once

#include <stdint.h>
#include "stm_defines.h"


void rtc_init(void);
void rtc_datetime_set(uint32_t timestamp);
uint32_t rtc_datetime_get(void);
