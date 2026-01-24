#include "sensor.h"

#include "bme280/bme280.h"
#include "modules/i2c.h"
#include "sys_init.h"
#include "debug.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdint.h>

#define BME280_ADDR             (0x76)
#define BME280_REG_ADDR_LEN     (1u)

static BME280_INTF_RET_TYPE bme_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void * intf_ptr)
{
    i2c_inst_s * i2c = (i2c_inst_s *)intf_ptr;
    return (BME280_INTF_RET_TYPE)i2c_write(i2c, BME280_ADDR, reg_addr, BME280_REG_ADDR_LEN, reg_data, len);
}

static BME280_INTF_RET_TYPE bme_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    i2c_inst_s * i2c = (i2c_inst_s *)intf_ptr;
    return (BME280_INTF_RET_TYPE)i2c_read(i2c, BME280_ADDR, reg_addr, BME280_REG_ADDR_LEN, reg_data, len);
}

static void bme_delay_us(uint32_t period, void *intf_ptr)
{
    //todo
    (void)intf_ptr;
    for (volatile uint32_t i = 0; i < period * (cc_sys_clk_hz_get() / 1000000); i++)
    {}
}

static uint32_t sens_data;

uint32_t sensor_service_get(void)
{
    return sens_data;
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

            DBG_INFO("\ntemp = %d\n", comp_data.temperature);
            sens_data = comp_data.temperature;
            idx++;
        }
    }

    return rslt;
}

/* Task to be created. */
static void bmp280_task(void * params)
{
    app_sens_cfg_s * cfg = params;

    i2c_master_init(cfg->i2c);

    struct bme280_dev dev = 
    {
        .intf = BME280_I2C_INTF,
        .intf_ptr = (void *) cfg->i2c,
        .read = bme_i2c_read,
        .write = bme_i2c_write,
        .delay_us = bme_delay_us,
    };

    int8_t rslt;
    (void)rslt;
    uint32_t period;
    struct bme280_settings settings;

    rslt = bme280_init(&dev);

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
        
        DBG_INFO("res = %u\n", rslt);
        
        vTaskDelay(cfg->meas_period_ms);
    }
}



/* Function that creates a task. */
void sensor_service(app_sens_cfg_s * cfg)
{
    BaseType_t xReturned;
    TaskHandle_t xHandle = NULL;
    app_sens_cfg_s * config = pvPortMalloc(sizeof(app_sens_cfg_s));
    *config = *cfg;

    /* Create the task, storing the handle. */
    xReturned = xTaskCreate(
                    bmp280_task,        /* Function that implements the task. */
                    "sens",             /* Text name for the task. */
                    256,                /* Stack size in words, not bytes. */
                    ( void * ) config,     /* Parameter passed into the task. */
                    tskIDLE_PRIORITY + 2,   /* Priority at which the task is created. */
                    &xHandle );         /* Used to pass out the created task's handle. */

    if (xReturned != pdPASS)
    {
        DBG_ERR("Sensors task failed\n");
    }
}