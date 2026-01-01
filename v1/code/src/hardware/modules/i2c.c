#include "i2c.h"

#include "stm_defines.h"
#include "consts.h"
#include "sys_init.h"
#include "sync_object.h"

#include <stdbool.h>
#include <stddef.h>


#define I2C_100KHZ_COEFF        (5u)
#define DEFAULT_ADDR            (0x11)
#define I2C_WAIT_BUSY_TO_MS     (1000u)
#define I2C_WAIT_RX_TO_MS       (10000u)
#define RETRY_QTY               (2u)
#define READING                 (true)
#define WRITING                 (false)

#ifndef I2C_USE_IRQ
#define I2C_USE_IRQ     (0)
#endif

static void init(const i2c_inst_s * i2c)
{
    const uint32_t freq = cc_i2c_clk_hz_get();
    i2c->inst->CR1 &= ~I2C_CR1_PE;

    i2c->inst->CR1 |= I2C_CR1_SWRST;
    i2c->inst->CR1 &= ~I2C_CR1_SWRST;

    i2c->inst->CR2 = ((freq << I2C_CR2_FREQ_Pos) & I2C_CR2_FREQ_Msk) |
                (I2C_CR2_DMAEN * 0) |
                #if I2C_USE_IRQ
                (I2C_CR2_ITBUFEN) |
                (I2C_CR2_ITEVTEN) |
                (I2C_CR2_ITERREN) |
                #endif
                (I2C_CR2_LAST * 0);

    i2c->inst->CCR = (I2C_CCR_FS * 0) |
                (I2C_CCR_DUTY * 0) |
                (((freq * I2C_100KHZ_COEFF) << I2C_CCR_CCR_Pos) & I2C_CCR_CCR_Msk);

    i2c->inst->TRISE = freq + 1;

    i2c->inst->OAR1 = 0x11;
    i2c->inst->OAR2 = DEFAULT_ADDR;

    i2c->inst->CR1 |= I2C_CR1_PE;
}

i2c_error_t i2c_master_init(i2c_inst_s * i2c)
{
    if (i2c->inited)
    {
        return I2C_OK;
    }

    if (i2c->sync_object == NULL)
    {
        i2c->sync_object = sync_object_init();
    }

    sync_object_take(i2c->sync_object);

    init(i2c);

    sync_object_release(i2c->sync_object);

    i2c->inited = true;

    return I2C_OK;
}

#if I2C_USE_IRQ

typedef enum i2c_state_e
{
    I2C_IDLE,
    I2C_WAIT_BTF_FOR_RESTART,
    I2C_REPEATED_START,
    I2C_REG_ADDR_TX,
    I2C_DATA_TX,
    I2C_DATA_RX,
    I2C_FINISHED
} i2c_state_e;

typedef struct i2c_ctx_s
{
    const i2c_inst_s * i2c;
    uint8_t slave_address;
    uint32_t reg_addr;
    uint32_t reg_addr_size;
    uint8_t * data;
    const uint8_t * c_data;
    uint32_t data_size;
    i2c_state_e state;
    uint8_t reg_addr_byte_pos;
    uint8_t data_byte_pos;
    bool reading;
    i2c_error_t error;
} i2c_ctx_s;

static volatile i2c_ctx_s i2c_ctx;

void i2c_irq_err_handler(void)
{
    const uint32_t err_st = i2c_ctx.i2c->inst->SR1;

    i2c_ctx.i2c->inst->CR1 |= I2C_CR1_STOP;
    
    if (err_st & I2C_SR1_TIMEOUT)
    {
        i2c_ctx.i2c->inst->SR1 &= ~I2C_SR1_TIMEOUT;
        i2c_ctx.state = I2C_FINISHED;
        i2c_ctx.error = I2C_SCL_TIMEOUT;        
    }

    if (err_st & I2C_SR1_OVR)
    {
        i2c_ctx.i2c->inst->SR1 &= ~I2C_SR1_OVR;
        i2c_ctx.error = I2C_OVR;
    }

    if (err_st & I2C_SR1_AF)
    {
        i2c_ctx.i2c->inst->SR1 &= ~I2C_SR1_AF;
        i2c_ctx.error = I2C_AF;
        i2c_ctx.i2c->inst->CR1 |= I2C_CR1_STOP;
        i2c_ctx.state = I2C_FINISHED;
    }

    if (err_st & I2C_SR1_ARLO)
    {
        i2c_ctx.i2c->inst->SR1 &= ~I2C_SR1_ARLO;
        i2c_ctx.state = I2C_FINISHED;
        i2c_ctx.error = I2C_ARLO;
    }

    if (err_st & I2C_SR1_BERR)
    {
        i2c_ctx.i2c->inst->SR1 &= ~I2C_SR1_BERR;
        i2c_ctx.state = I2C_FINISHED;
        i2c_ctx.error = I2C_BERR;
    }
    
}

volatile uint32_t s_counter = 0;
volatile uint32_t statuses[32] = {0};

static void add_rec(uint32_t rec)
{
    statuses[s_counter%32] = rec;
    s_counter++;
}

void i2c_irq_event_handler(void)
{
    const uint32_t evt_st = i2c_ctx.i2c->inst->SR1;

    add_rec(evt_st);

    if (evt_st & I2C_SR1_SB)
    {
        // Enable TxE and RxNE 
        i2c_ctx.i2c->inst->CR2 |= I2C_CR2_ITBUFEN;

        if ((i2c_ctx.state == I2C_REPEATED_START && i2c_ctx.reading) || 
            (i2c_ctx.reg_addr_size == 0 && i2c_ctx.reading))
        {            
            i2c_ctx.i2c->inst->DR = (i2c_ctx.slave_address << 1) | 0x01;
            i2c_ctx.state = I2C_DATA_RX;
        }
        else
        {
            i2c_ctx.i2c->inst->DR = i2c_ctx.slave_address << 1;
            if (i2c_ctx.reg_addr_size)
            {
                i2c_ctx.state = I2C_REG_ADDR_TX;
            }
            else
            {
                i2c_ctx.state = I2C_DATA_TX;
            }
        }

        // Dummy read. This will cause the previous write to complete first and disable the interrupt pending request on the NVIC
        (void)i2c_ctx.i2c->inst->CR1;
        return;
    }
    
    if (evt_st & I2C_SR1_ADDR)
    {
        // Read SR2 for clear ADDR bit
        (void) i2c_ctx.i2c->inst->SR2;
        
        if (i2c_ctx.state == I2C_REG_ADDR_TX)
        {
            const uint32_t addr_m = (i2c_ctx.reg_addr >> ((i2c_ctx.reg_addr_size - 1) * C_BITS_PER_BYTE)) & C_BYTE_MASK;
            i2c_ctx.i2c->inst->DR = addr_m;
            add_rec(addr_m);
            if (i2c_ctx.reg_addr_byte_pos == 1)
            {
                i2c_ctx.state = I2C_DATA_TX;
            }
        }
        else
        {
            if (i2c_ctx.reading)
            {
                i2c_ctx.data_byte_pos = 0;
                if (i2c_ctx.data_size)
                {
                    // Read data
                    if (i2c_ctx.data_byte_pos == i2c_ctx.data_size - 1) 
                    {
                        // Generate NACK for the last byte
                        i2c_ctx.i2c->inst->CR1 &= ~I2C_CR1_ACK;
                        i2c_ctx.i2c->inst->CR1 |= I2C_CR1_STOP;
                    }
                    else 
                    {
                        // Generate ACK for all but the last byte
                        i2c_ctx.i2c->inst->CR1 |= I2C_CR1_ACK;
                    }
                }
                else
                {
                    // Generate STOP condition
                    i2c_ctx.i2c->inst->CR1 |= I2C_CR1_STOP;
                    i2c_ctx.state = I2C_FINISHED;
                }
            }
            else
            {
                if (i2c_ctx.data_byte_pos < i2c_ctx.data_size)
                {
                    i2c_ctx.i2c->inst->DR = i2c_ctx.c_data[i2c_ctx.data_byte_pos];
                    i2c_ctx.data_byte_pos++;
                }
            }
        }

        return;
    }

    if (evt_st & I2C_SR1_ADD10)
    {
        // todo
    }

    if (evt_st & I2C_SR1_STOPF)
    {
        // todo
    }

    if (evt_st & I2C_SR1_RXNE)
    {
        if (i2c_ctx.data_byte_pos < i2c_ctx.data_size)
        {
            i2c_ctx.data[i2c_ctx.data_byte_pos] = i2c_ctx.i2c->inst->DR;
        }
        else
        {
            // Clear irq
            (void)i2c_ctx.i2c->inst->DR;
        }
        
        i2c_ctx.data_byte_pos++;

        if (i2c_ctx.data_size)
        {
            if (i2c_ctx.data_byte_pos == i2c_ctx.data_size)
            {
                i2c_ctx.state = I2C_FINISHED;
            }
            else if (i2c_ctx.data_byte_pos == i2c_ctx.data_size - 1) 
            {
                // Generate NACK for the last byte
                i2c_ctx.i2c->inst->CR1 &= ~I2C_CR1_ACK;            
                // Generate STOP condition
                i2c_ctx.i2c->inst->CR1 |= I2C_CR1_STOP;
                i2c_ctx.state = I2C_FINISHED;
            }
            else 
            {
                // Generate ACK for all but the last byte
                i2c_ctx.i2c->inst->CR1 |= I2C_CR1_ACK;
            }
        }
        return;
    }

    if (evt_st & I2C_SR1_TXE)
    {
        if (i2c_ctx.state == I2C_REG_ADDR_TX)
        {
            i2c_ctx.reg_addr_byte_pos++;
            if (i2c_ctx.reg_addr_byte_pos == i2c_ctx.reg_addr_size)
            {
                if (i2c_ctx.reading)
                {
                    i2c_ctx.i2c->inst->CR2 &= ~I2C_CR2_ITBUFEN;
                    i2c_ctx.i2c->inst->CR1 |= I2C_CR1_START;
                    i2c_ctx.state = I2C_REPEATED_START;
                    return;                 
                }
                else
                {
                    i2c_ctx.state = I2C_DATA_TX;
                }
            }
            else
            {
                const uint32_t byte_offset = i2c_ctx.reg_addr_size - i2c_ctx.reg_addr_byte_pos - 1;
                i2c_ctx.i2c->inst->DR = (i2c_ctx.reg_addr >> (C_BITS_PER_BYTE * byte_offset)) & C_BYTE_MASK;
                // Dummy read. This will cause the previous write to complete first and disable the interrupt pending request on the NVIC
                (void)i2c_ctx.i2c->inst->CR1;
                return;
            }
        }  

        if (i2c_ctx.state == I2C_DATA_TX)
        {
            if (i2c_ctx.data_byte_pos < i2c_ctx.data_size)
            {
                i2c_ctx.i2c->inst->DR = i2c_ctx.c_data[i2c_ctx.data_byte_pos];
                i2c_ctx.data_byte_pos++;
                return;
            }
            if (i2c_ctx.data_byte_pos >= i2c_ctx.data_size)
            {
                i2c_ctx.i2c->inst->CR2 &= ~I2C_CR2_ITBUFEN;
            }
        }
    }

    if (evt_st & I2C_SR1_BTF)
    {        
        if (i2c_ctx.state == I2C_FINISHED || i2c_ctx.data_byte_pos >= i2c_ctx.data_size)
        {
            if (i2c_ctx.reading)
            {
                // Generate NACK for the last byte
                i2c_ctx.i2c->inst->CR1 &= ~I2C_CR1_ACK;
            } 
            i2c_ctx.i2c->inst->CR1 |= I2C_CR1_STOP;
            i2c_ctx.state = I2C_FINISHED;
        }

        // Unconditional BTF clear
        (void)i2c_ctx.i2c->inst->DR;
        add_rec(0xb7f);
    }
}

static i2c_error_t i2c_try_execute(void)
{
    // Wait until I2C is not busy
    uint32_t ms_qty = 0;
    while (i2c_ctx.i2c->inst->SR2 & I2C_SR2_BUSY)
    {
        i2c_ctx.i2c->delay_ms(1);
        ms_qty++;
        if (ms_qty > I2C_WAIT_BUSY_TO_MS)
        {
            return I2C_WAIT_BUSY_TO;
        }
    }

    // Generate START condition
    i2c_ctx.i2c->inst->CR1 |= I2C_CR1_START;

    ms_qty = 0;
    while (i2c_ctx.state != I2C_FINISHED)
    {
        i2c_ctx.i2c->delay_ms(1);
        ms_qty++;
        if (ms_qty > I2C_WAIT_RX_TO_MS)
        {
            return I2C_RX_TO;
        }
    }

    return i2c_ctx.error;
}

static i2c_error_t i2c_execute(void)
{    
    i2c_error_t err = I2C_OK;

    for (uint32_t i = 0; i < RETRY_QTY; i++)
    {    
        i2c_ctx.state               = I2C_IDLE;
        i2c_ctx.error               = I2C_OK;
        i2c_ctx.data_byte_pos       = 0;
        i2c_ctx.reg_addr_byte_pos   = 0;

        err = i2c_try_execute();

        if (err != I2C_OK)
        {
            i2c_ctx.i2c->delay_ms(5);
            init(i2c_ctx.i2c);
            i2c_ctx.i2c->delay_ms(1);
        }
        else
        {
            break;
        }
    }
    return err;
}

i2c_error_t i2c_read(i2c_inst_s * i2c, uint8_t slave_address, uint32_t reg_addr, uint32_t reg_addr_size, uint8_t * data, uint32_t data_size)
{
    sync_object_take(i2c->sync_object);

    i2c_ctx.i2c                 = i2c;
    i2c_ctx.slave_address       = slave_address;
    i2c_ctx.reg_addr            = reg_addr;
    i2c_ctx.reg_addr_size       = reg_addr_size;
    i2c_ctx.data                = data;
    i2c_ctx.data_size           = data_size;
    i2c_ctx.reading             = true;

    i2c_error_t err = i2c_execute();

    sync_object_release(i2c->sync_object);

    return err;
}

i2c_error_t i2c_write(i2c_inst_s * i2c, uint8_t slave_address, uint32_t reg_addr, uint32_t reg_addr_size, const uint8_t * data, uint32_t data_size)
{
    sync_object_take(i2c->sync_object);

    i2c_ctx.i2c                 = i2c;
    i2c_ctx.slave_address       = slave_address;
    i2c_ctx.reg_addr            = reg_addr;
    i2c_ctx.reg_addr_size       = reg_addr_size;
    i2c_ctx.c_data                = data;
    i2c_ctx.data_size           = data_size;
    i2c_ctx.reading             = false;

    i2c_error_t err = i2c_execute();

    sync_object_release(i2c->sync_object);

    return err;
}

#else

i2c_error_t i2c_read(i2c_inst_s * i2c, uint32_t reg_addr, uint8_t * data, uint32_t len) 
{
    sync_object_take(i2c->sync_object);

    // Wait until I2C is not busy
    while (i2c_ctx.i2c->inst->SR2 & I2C_SR2_BUSY);

    // Generate START condition
    i2c_ctx.i2c->inst->CR1 |= I2C_CR1_START;

    // Wait for SB flag (start bit generated)
    while (!(i2c_ctx.i2c->inst->SR1 & I2C_SR1_SB));

    // Send slave address with write bit (LSB = 0)
    i2c_ctx.i2c->inst->DR = slave_address << 1; // Replace 0x68 with your device's address

    // Wait for ADDR flag (address sent)
    while (!(i2c_ctx.i2c->inst->SR1 & I2C_SR1_ADDR));

    // Clear ADDR flag by reading SR2
    (void)i2c_ctx.i2c->inst->SR2;

    // Send register address to read from
    i2c_ctx.i2c->inst->DR = reg_addr;

    // Wait for TXE flag (data register empty)
    while (!(i2c_ctx.i2c->inst->SR1 & I2C_SR1_TXE));

    // Generate repeated START condition
    i2c_ctx.i2c->inst->CR1 |= I2C_CR1_START;

    // Wait for SB flag (start bit generated)
    while (!(i2c_ctx.i2c->inst->SR1 & I2C_SR1_SB));

    // Send slave address with read bit (LSB = 1)
    i2c_ctx.i2c->inst->DR = (slave_address << 1) | 0x01;

    // Wait for ADDR flag (address sent)
    while (!(i2c_ctx.i2c->inst->SR1 & I2C_SR1_ADDR));

    // Clear ADDR flag by reading SR2
    (void)i2c_ctx.i2c->inst->SR2;

    // Read data
    for (uint32_t i = 0; i < len; i++) {
        if (i == len - 1) {
            // Generate NACK for the last byte
            i2c_ctx.i2c->inst->CR1 &= ~I2C_CR1_ACK;
        } else {
            // Generate ACK for all but the last byte
            i2c_ctx.i2c->inst->CR1 |= I2C_CR1_ACK;
        }

        // Wait for RXNE flag (data received)
        while (!(i2c_ctx.i2c->inst->SR1 & I2C_SR1_RXNE));

        // Read data
        data[i] = (uint8_t)(i2c_ctx.i2c->inst->DR & C_BYTE_MASK);
    }

    // Generate STOP condition
    i2c_ctx.i2c->inst->CR1 |= I2C_CR1_STOP;

    sync_object_release(i2c->sync_object);

    return 0; // Success
}

i2c_error_t i2c_write(i2c_inst_s * i2c, uint32_t reg_addr, const uint8_t * data, uint32_t len) 
{
    sync_object_take(i2c->sync_object);

    // Wait until I2C is not busy
    while (i2c_ctx.i2c->inst->SR2 & I2C_SR2_BUSY);

    // Generate START condition
    i2c_ctx.i2c->inst->CR1 |= I2C_CR1_START;

    // Wait for SB flag (start bit generated)
    while (!(i2c_ctx.i2c->inst->SR1 & I2C_SR1_SB));

    // Send slave address with write bit (LSB = 0)
    i2c_ctx.i2c->inst->DR = slave_address << 1; // Replace 0x68 with your device's address

    // Wait for ADDR flag (address sent)
    while (!(i2c_ctx.i2c->inst->SR1 & I2C_SR1_ADDR));

    // Clear ADDR flag by reading SR2
    (void)i2c_ctx.i2c->inst->SR2;

    // Send register address to write to
    i2c_ctx.i2c->inst->DR = reg_addr;

    // Wait for TXE flag (data register empty)
    while (!(i2c_ctx.i2c->inst->SR1 & I2C_SR1_TXE));

    // Send data
    for (uint32_t i = 0; i < len; i++) {
        i2c_ctx.i2c->inst->DR = c_data[i];

        // Wait for TXE flag (data register empty)
        while (!(i2c_ctx.i2c->inst->SR1 & I2C_SR1_TXE));
    }

    // Wait for BTF flag (byte transfer finished)
    while (!(i2c_ctx.i2c->inst->SR1 & I2C_SR1_BTF));

    // Generate STOP condition
    i2c_ctx.i2c->inst->CR1 |= I2C_CR1_STOP;


    sync_object_release(i2c->sync_object);

    return 0; // Success
}

#endif