
#include "core/core_cm3.h"
#include "stm_defines.h"
#include "sys_init.h"
#include "i2c.h"
#include "FreeRTOS.h"
#include "task.h"

#define BME280_ADDR     (0x76)

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

/* Task to be created. */
void i2c_task( void * pvParameters )
{
    (void)pvParameters;

    i2c_inst_s i2c = 
    {
        .inst = I2C1,
        .slave_address = BME280_ADDR,
    };

    i2c_master_init(&i2c);

    uint8_t i2c_buf[8] = {0};

    for( ;; )
    {
        i2c_read(&i2c,  0xd0, i2c_buf, 1);
        uart_send_char('1');
        uart_send_char(i2c_buf[0]);
        uart_send_char('2');
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
                    64,      /* Stack size in words, not bytes. */
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

    uart_send_char('1');
    uart_send_char('2');
    uart_send_char('3');
    uart_send_char('4');


    __enable_irq();
    

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