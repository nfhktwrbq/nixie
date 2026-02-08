
#pragma once

#include "modules/i2c.h"
#include "drivers/settings.h"


#include <stdint.h>

typedef struct sens_cfg_s
{
    i2c_inst_s * i2c;
    settings_s * settings;
} sens_cfg_s;


uint32_t sensor_service_get(void);

void sensor_service(sens_cfg_s * cfg);