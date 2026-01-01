#pragma once

#include "modules/i2c.h"

#include <stdint.h>
#include <stddef.h>

typedef struct eeprom_s
{
    i2c_inst_s * i2c;
    uint8_t address;
    uint32_t reg_addr_size;
} eeprom_s;

void eeprom_init(eeprom_s * inst);
void eeprom_read(eeprom_s * inst, void * data, size_t size, uint32_t offset);
void eeprom_write(eeprom_s * inst, const void * data, size_t size, uint32_t offset);
