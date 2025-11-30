#ifndef I2CM_H
#define I2CM_H

#include "stm_defines.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct i2c_inst_s
{
    I2C_TypeDef * inst;    
    void * sync_object;
    bool inited;
} i2c_inst_s;

uint8_t i2c_master_init(i2c_inst_s * i2c);
uint32_t i2c_read(i2c_inst_s * i2c, uint8_t slave_address, uint8_t reg_addr, uint8_t * reg_data, uint32_t len);
uint32_t i2c_write(i2c_inst_s * i2c, uint8_t slave_address, uint8_t reg_addr, const uint8_t * reg_data, uint32_t len);

#endif //I2CM_H