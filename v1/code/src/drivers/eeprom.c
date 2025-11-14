#include "eeprom.h"

#define EEPROM_ARRD     (0x57)

#if 0
i2c_inst_s i2c_eeprom = 
    {
        .inst = I2C1,
        .slave_address = EEPROM_ARRD,
    };

    i2c_inst_s i2c_bmp = 
    {
        .inst = I2C1,
        .slave_address = BME280_ADDR,
    };


    
    uint8_t buf[8] = {0};
    
    
    i2c_read(&i2c_eeprom, 0x00, buf, 8);
    
    

    for (uint8_t i = 0; i < sizeof(buf); i++)
    {
        uart_send_char(buf[i]);
    }

    if (buf[0] != 1)
    {
        for (uint8_t i = 0; i < sizeof(buf); i++)
        {
            buf[i] = i+1;
        }
        i2c_write(&i2c_eeprom, 0x00, buf, 8);
    }

    i2c_read(&i2c_bmp, 0xD0, buf, 1);

    uart_send_char(buf[0]);

    #endif