#pragma once

#include "drivers_ll/eeprom.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct settings_s
{
    uint32_t meas_period_ms;
} settings_s;

typedef eeprom_s settings_iface_s;

void settings_init(settings_iface_s i);
bool settings_restore(settings_s * settings);
void settings_save(settings_s * settings);