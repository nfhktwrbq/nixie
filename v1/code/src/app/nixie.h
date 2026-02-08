#pragma once

#include "modules/i2c.h"
#include "drivers/settings.h"

typedef struct nixie_cfg_s
{
    i2c_inst_s * i2c;
    settings_s * settings;
} nixie_cfg_s;

void app_nixie(nixie_cfg_s * cfg);