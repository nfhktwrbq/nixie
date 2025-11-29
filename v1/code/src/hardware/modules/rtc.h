#pragma once

#include "stm_defines.h"

#include <stdint.h>
#include <stdbool.h>

void rtc_init(void);
void rtc_datetime_set(uint32_t timestamp);
uint32_t rtc_datetime_get(void);
bool rtc_second_flag_get(void);
