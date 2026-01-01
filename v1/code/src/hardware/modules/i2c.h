#pragma once

#include "stm_defines.h"

#include "types.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct i2c_inst_s
{
    I2C_TypeDef * inst;    
    void * sync_object;
    void_int_arg_fp delay_ms;
    bool inited;
} i2c_inst_s;

typedef enum i2c_error_t
{
    I2C_OK,
    I2C_WAIT_BUSY_TO,
    I2C_RX_TO,
    I2C_SCL_TIMEOUT,
    I2C_OVR,
    I2C_AF,
    I2C_ARLO,
    I2C_BERR,
} i2c_error_t;

i2c_error_t i2c_master_init(i2c_inst_s * i2c);
i2c_error_t i2c_read(i2c_inst_s * i2c, uint8_t slave_address, uint32_t reg_addr, uint32_t reg_addr_size, uint8_t * data, uint32_t data_size);
i2c_error_t i2c_write(i2c_inst_s * i2c, uint8_t slave_address, uint32_t reg_addr, uint32_t reg_addr_size, const uint8_t * data, uint32_t data_size);

#if I2C_USE_IRQ
void i2c_irq_err_handler(void);
void i2c_irq_event_handler(void);
#endif