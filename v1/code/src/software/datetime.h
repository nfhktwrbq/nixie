#pragma once

#include <stdint.h>

typedef struct datetime_s
{
    uint8_t day;
    uint8_t month; 
    uint16_t year; 
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} datetime_s;

void datetime_from_timestamp(uint32_t timestamp, datetime_s *datetime);
uint32_t datetime_to_timestamp(const datetime_s *datetime);
