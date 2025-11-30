#include "settings.h"

#include "software/crc.h"
#include "debug.h"

#include <stddef.h>
#include <string.h>

static eeprom_s eeprom;

typedef struct settings_srv_s
{
    settings_s settings;
    uint32_t crc;
} settings_srv_s;

static const settings_s default_settings = 
{
    .meas_period_ms = 3000,
};

void settings_init(settings_iface_s i)
{
    eeprom = (eeprom_s)i;
    eeprom_init(&eeprom);
}

bool settings_restore(settings_s * settings)
{
    bool result = false;

    settings_srv_s s_srv = {0};

    eeprom_read(&eeprom, &s_srv, sizeof(settings_srv_s), 0);    
    
    uint32_t crc = crc32(&s_srv.settings, sizeof(settings_s));
    if (crc != s_srv.crc)
    {
        DBG_WRN("Settings restore failed. Load default\n");
        memcpy(settings, &default_settings, sizeof(settings_s)); 
    }
    else
    {
        memcpy(settings, &s_srv.settings, sizeof(settings_s)); 
        result = true;
        DBG_INFO("Settings restore OK\n");
    }

    return result;
}

void settings_save(settings_s * settings)
{
    settings_srv_s s_srv = {0};

    s_srv.crc = crc32(settings, sizeof(settings_s));

    memcpy(&s_srv.settings, settings, sizeof(settings_s));
    eeprom_write(&eeprom, &s_srv, sizeof(settings_srv_s), 0);    
    DBG_INFO("Settings saved\n");
}
