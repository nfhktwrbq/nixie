#pragma once

#include "drivers_ll/eeprom.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct settings_s
{
    uint32_t meas_period_ms;
    uint32_t relax_period_s; //for nixie tubes
    uint32_t show_date_period_s;
} settings_s;

typedef eeprom_s settings_iface_s;

#define SETTINGS_EEPROM_ADDR                (0x57)
#define SETTINGS_EEPROM_REG_ADDR_SIZE       (1)

typedef enum settings_restore_state_e
{
    SETTINGS_RESTORE_OK,
    SETTINGS_RESTORE_FAILED_DEFAULT_LOADED,
} settings_restore_state_e;

void settings_init(settings_iface_s i);
settings_restore_state_e settings_restore(settings_s * settings);
void settings_save(settings_s * settings);