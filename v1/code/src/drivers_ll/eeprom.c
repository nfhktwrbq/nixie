#include "eeprom.h"


void eeprom_init(eeprom_s * inst)
{
    (void) i2c_master_init(inst->i2c);
}

void eeprom_read(eeprom_s * inst, void * data, size_t size, uint32_t offset)
{
    (void) i2c_read(inst->i2c, inst->address, offset, inst->reg_addr_size, data, size);
}

void eeprom_write(eeprom_s * inst, const void * data, size_t size, uint32_t offset)
{
    i2c_write(inst->i2c, inst->address, offset, inst->reg_addr_size, data, size);
}