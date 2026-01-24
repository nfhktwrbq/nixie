
#pragma once

#include "modules/i2c.h"


#include <stdint.h>

typedef struct app_sens_cfg_s
{
    i2c_inst_s * i2c;
    uint32_t meas_period_ms;
} app_sens_cfg_s;


uint32_t sensor_service_get(void);

void sensor_service(app_sens_cfg_s * cfg);