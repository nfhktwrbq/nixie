
#pragma once

#include "stm_defines.h"

#include <stdint.h>

typedef struct app_sens_cfg_s
{
    I2C_TypeDef * i2c;
    uint32_t meas_period_ms;
} app_sens_cfg_s;

void app_sensor(app_sens_cfg_s * cfg);