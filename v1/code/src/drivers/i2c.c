#include <stdbool.h>
#include "i2c.h"
#include "stm_defines.h"
#include "consts.h"
#include "sys_init.h"

#define I2C_100KHZ_COEFF (5u)
#define DEFAULT_ADDR (0x11)


uint8_t i2c_master_init(i2c_inst_s * i2c)
{
    const uint32_t freq = APB1_CLOCK_HZ;
    i2c->inst->CR1 &= ~I2C_CR1_PE;

    i2c->inst->CR1 |= I2C_CR1_SWRST;
    i2c->inst->CR1 &= ~I2C_CR1_SWRST;

    i2c->inst->CR2 = ((freq << I2C_CR2_FREQ_Pos) & I2C_CR2_FREQ_Msk) |
                (I2C_CR2_DMAEN * 0) |
                (I2C_CR2_LAST * 0);

    i2c->inst->CCR = (I2C_CCR_FS * 0) |
                (I2C_CCR_DUTY * 0) |
                (((freq * I2C_100KHZ_COEFF) << I2C_CCR_CCR_Pos) & I2C_CCR_CCR_Msk);

    i2c->inst->TRISE = freq + 1;

    i2c->inst->OAR1 = 0x11;
    i2c->inst->OAR2 = DEFAULT_ADDR;

    i2c->inst->CR1 |= I2C_CR1_PE;

    return 0;
}

uint32_t i2c_read(i2c_inst_s * i2c, uint8_t reg_addr, uint8_t * reg_data, uint32_t len) 
{
    // Wait until I2C is not busy
    while (i2c->inst->SR2 & I2C_SR2_BUSY);

    // Generate START condition
    i2c->inst->CR1 |= I2C_CR1_START;

    // Wait for SB flag (start bit generated)
    while (!(i2c->inst->SR1 & I2C_SR1_SB));

    // Send slave address with write bit (LSB = 0)
    i2c->inst->DR = i2c->slave_address << 1; // Replace 0x68 with your device's address

    // Wait for ADDR flag (address sent)
    while (!(i2c->inst->SR1 & I2C_SR1_ADDR));

    // Clear ADDR flag by reading SR2
    (void)i2c->inst->SR2;

    // Send register address to read from
    i2c->inst->DR = reg_addr;

    // Wait for TXE flag (data register empty)
    while (!(i2c->inst->SR1 & I2C_SR1_TXE));

    // Generate repeated START condition
    i2c->inst->CR1 |= I2C_CR1_START;

    // Wait for SB flag (start bit generated)
    while (!(i2c->inst->SR1 & I2C_SR1_SB));

    // Send slave address with read bit (LSB = 1)
    i2c->inst->DR = (i2c->slave_address << 1) | 0x01;

    // Wait for ADDR flag (address sent)
    while (!(i2c->inst->SR1 & I2C_SR1_ADDR));

    // Clear ADDR flag by reading SR2
    (void)i2c->inst->SR2;

    // Read data
    for (uint32_t i = 0; i < len; i++) {
        if (i == len - 1) {
            // Generate NACK for the last byte
            i2c->inst->CR1 &= ~I2C_CR1_ACK;
        } else {
            // Generate ACK for all but the last byte
            i2c->inst->CR1 |= I2C_CR1_ACK;
        }

        // Wait for RXNE flag (data received)
        while (!(i2c->inst->SR1 & I2C_SR1_RXNE));

        // Read data
        reg_data[i] = (uint8_t)(i2c->inst->DR & C_BYTE_MASK);
    }

    // Generate STOP condition
    i2c->inst->CR1 |= I2C_CR1_STOP;

    return 0; // Success
}

uint32_t i2c_write(i2c_inst_s * i2c, uint8_t reg_addr, const uint8_t * reg_data, uint32_t len) 
{
    // Wait until I2C is not busy
    while (i2c->inst->SR2 & I2C_SR2_BUSY);

    // Generate START condition
    i2c->inst->CR1 |= I2C_CR1_START;

    // Wait for SB flag (start bit generated)
    while (!(i2c->inst->SR1 & I2C_SR1_SB));

    // Send slave address with write bit (LSB = 0)
    i2c->inst->DR = i2c->slave_address << 1; // Replace 0x68 with your device's address

    // Wait for ADDR flag (address sent)
    while (!(i2c->inst->SR1 & I2C_SR1_ADDR));

    // Clear ADDR flag by reading SR2
    (void)i2c->inst->SR2;

    // Send register address to write to
    i2c->inst->DR = reg_addr;

    // Wait for TXE flag (data register empty)
    while (!(i2c->inst->SR1 & I2C_SR1_TXE));

    // Send data
    for (uint32_t i = 0; i < len; i++) {
        i2c->inst->DR = reg_data[i];

        // Wait for TXE flag (data register empty)
        while (!(i2c->inst->SR1 & I2C_SR1_TXE));
    }

    // Wait for BTF flag (byte transfer finished)
    while (!(i2c->inst->SR1 & I2C_SR1_BTF));

    // Generate STOP condition
    i2c->inst->CR1 |= I2C_CR1_STOP;

    return 0; // Success
}