
#include "core/core_cm3.h"
#include "stm_defines.h"
#include "sys_init.h"
#include "i2c.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bme280/bme280.h"
#include "bme280/bme280_defs.h"
#include "uart.h"
#include "printf/printf.h"
#include "debug.h"
#include "rtc.h"

#define BME280_ADDR     (0x76)
#define EEPROM_ARRD     (0x57)

void delay(uint32_t ticks)
{
    for(volatile uint32_t i = 0; i < ticks; i++);
}


/* Task to be created. */
void vTaskCode( void * pvParameters )
{
    /* The parameter value is expected to be 1 as 1 is passed in the
       pvParameters value in the call to xTaskCreate() below. */

    (void)pvParameters;
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );

    for( ;; )
    {
        uart_send_char('4');
        vTaskDelay(300);
        /* Task code goes here. */
    }
}

BME280_INTF_RET_TYPE bme_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void * intf_ptr)
{
    i2c_inst_s * i2c = (i2c_inst_s *)intf_ptr;
    return (BME280_INTF_RET_TYPE)i2c_write(i2c, reg_addr, reg_data, len);
}

BME280_INTF_RET_TYPE bme_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    i2c_inst_s * i2c = (i2c_inst_s *)intf_ptr;
    return (BME280_INTF_RET_TYPE)i2c_read(i2c, reg_addr, reg_data, len);
}

void bme_delay_us(uint32_t period, void *intf_ptr)
{
    //todo
    (void)intf_ptr;
    for (volatile uint32_t i = 0; i < period * (SYSTEM_CORE_CLOCK_HZ / 1000000); i++)
    {}
}

#define SAMPLE_COUNT  UINT8_C(1)
/*!
 *  @brief This internal API is used to get compensated temperature data.
 */
static int8_t get_temperature(uint32_t period, struct bme280_dev *dev)
{
    int8_t rslt = BME280_E_NULL_PTR;
    int8_t idx = 0;
    uint8_t status_reg;
    struct bme280_data comp_data;

    while (idx < SAMPLE_COUNT)
    {
        rslt = bme280_get_regs(BME280_REG_STATUS, &status_reg, 1, dev);

        if (status_reg & BME280_STATUS_MEAS_DONE)
        {
            /* Measurement time delay given to read sample */
            dev->delay_us(period, dev->intf_ptr);

            /* Read compensated data */
            rslt = bme280_get_sensor_data(BME280_TEMP, &comp_data, dev);

            // printf("Temperature[%d]:   %ld deg C\n", idx, (long int)comp_data.temperature);

            // uart_send_char('5');
            // uart_send_char((char)(comp_data.temperature & 0xff));
            // uart_send_char((char)((comp_data.temperature >> 8) & 0xff));
            // uart_send_char((char)((comp_data.temperature >> 16) & 0xff));
            // uart_send_char((char)((comp_data.temperature >> 24) & 0xff));
            DBG_INFO("\ntemp = %d\n", comp_data.temperature);
            // uart_send_char('6');
            idx++;
        }
    }

    return rslt;
}

/* Task to be created. */
void i2c_task( void * pvParameters )
{
    (void)pvParameters;

    i2c_inst_s i2c = 
    {
        .inst = I2C1,
        .slave_address = BME280_ADDR,
    };

    struct bme280_dev dev = 
    {
        .intf = BME280_I2C_INTF,
        .intf_ptr = (void *) &i2c,
        .read = bme_i2c_read,
        .write = bme_i2c_write,
        .delay_us = bme_delay_us,
    };

    int8_t rslt;
    uint32_t period;
    struct bme280_settings settings;

    i2c_master_init(&i2c);

    rslt = bme280_init(&dev);


    // uint8_t i2c_buf[8] = {0};

    for( ;; )
    { 
        /* Always read the current settings before writing, especially when all the configuration is not modified */
        rslt = bme280_get_sensor_settings(&settings, &dev);

        /* Configuring the over-sampling rate, filter coefficient and standby time */
        /* Overwrite the desired settings */
        settings.filter = BME280_FILTER_COEFF_2;

        /* Over-sampling rate for humidity, temperature and pressure */
        settings.osr_h = BME280_OVERSAMPLING_1X;
        settings.osr_p = BME280_OVERSAMPLING_1X;
        settings.osr_t = BME280_OVERSAMPLING_1X;

        /* Setting the standby time */
        settings.standby_time = BME280_STANDBY_TIME_0_5_MS;

        rslt = bme280_set_sensor_settings(BME280_SEL_ALL_SETTINGS, &settings, &dev);

        /* Always set the power mode after setting the configuration */
        rslt = bme280_set_sensor_mode(BME280_POWERMODE_NORMAL, &dev);

        /* Calculate measurement time in microseconds */
        rslt = bme280_cal_meas_delay(&period, &settings);

        rslt = get_temperature(period, &dev);

        // bme280_coines_deinit();
        
        DBG_INFO("res = %u, time %u\n", rslt, rtc_datetime_get());
        
        vTaskDelay(1000);
        /* Task code goes here. */
    }
}

/* Function that creates a task. */
void vOtherFunction( void )
{
    BaseType_t xReturned;
    TaskHandle_t xHandle = NULL;

    /* Create the task, storing the handle. */
    xReturned = xTaskCreate(
                    i2c_task,       /* Function that implements the task. */
                    "NAME",          /* Text name for the task. */
                    256,      /* Stack size in words, not bytes. */
                    ( void * ) 1,    /* Parameter passed into the task. */
                    tskIDLE_PRIORITY,/* Priority at which the task is created. */
                    &xHandle );      /* Used to pass out the created task's handle. */

    // if( xReturned == pdPASS )
    // {
    //     /* The task was created. Use the task's handle to delete the task. */
    //     vTaskDelete( xHandle );
    // }
    (void)xReturned;
}

int main(void)
{
    // uint8_t buf[8];
    uart_init(921600);
    rtc_init();
    __enable_irq();
    uart_send_char('1');
    uart_send_char('2');
    uart_send_char('3');
    uart_send_char('4');

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
    
    i2c_master_init(&i2c_eeprom);
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
    

    // buf[0] = 0xd0;
    
    vOtherFunction();

    vTaskStartScheduler();

    for (;;){}

    // while (1)
    // {
    //     buf[0] = 0xd0;
    //     /*отправляем его*/
    //     HAL_I2C_Master_Transmit_IT(&hi2c, BME280_ADDR, buf, 1);
    //     // в байты buf[1]..buf[4] запишется принятое сообщение
        
    //     while (HAL_I2C_STATE_READY != HAL_I2C_GetState(&hi2c));
    //     buf[0] = 0;
    //     buf[1] = 1;
    //     HAL_I2C_Master_Receive_IT(&hi2c, BME280_ADDR, buf, 1);

    //     while (HAL_I2C_STATE_READY != HAL_I2C_GetState(&hi2c));

    //     uart_send_char('-');
    //     uart_send_char(buf[0]);
    //     uart_send_char(buf[1]);
    //     uart_send_char('\n');

    //     delay(10000000);
    // }

    while (1)
    {
    }
}