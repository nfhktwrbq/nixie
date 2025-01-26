
#include "core_functions.h"
#include "sys_init.h"
#include "i2c_stm.h"

#define BME280_ADDR     (0x76 << 1)

void delay(uint32_t ticks)
{
    for(volatile uint32_t i = 0; i < ticks; i++);
}

I2C_HandleTypeDef hi2c;

int main(void)
{
    uint8_t buf[8];

    uart_send_char('1');
    uart_send_char('2');
    uart_send_char('3');
    uart_send_char('4');


    __enable_irq();
    // i2c_master_init(36);
    
    hi2c.Devaddress = BME280_ADDR;
    hi2c.Instance = I2C1;
    hi2c.Init.ClockSpeed = 100000;
    hi2c.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c.Init.OwnAddress1 = 0x11;
    hi2c.Init.OwnAddress2 = 0x00;
    hi2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    HAL_I2C_Init(&hi2c);
    hi2c.pBuffPtr = buf;
    /*подготавливаем сообщение*/
    // buf[0] = (BME280_ADDR << 1) | 1; // бит R/W установлен в 1
    buf[0] = 0xd0;
    

    while (1)
    {
        buf[0] = 0xd0;
        /*отправляем его*/
        HAL_I2C_Master_Transmit_IT(&hi2c, BME280_ADDR, buf, 1);
        // в байты buf[1]..buf[4] запишется принятое сообщение
        
        while (HAL_I2C_STATE_READY != HAL_I2C_GetState(&hi2c));
        buf[0] = 0;
        buf[1] = 1;
        HAL_I2C_Master_Receive_IT(&hi2c, BME280_ADDR, buf, 1);

        while (HAL_I2C_STATE_READY != HAL_I2C_GetState(&hi2c));

        uart_send_char('-');
        uart_send_char(buf[0]);
        uart_send_char(buf[1]);
        uart_send_char('\n');

        delay(10000000);
    }

    while (1)
    {
    }
}