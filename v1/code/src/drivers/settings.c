#include "settings.h"

#include "software/crc.h"
#include "drivers/settings.h"
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
    .meas_period_ms     = 3000,
    .relax_period_s     = 120,
    .show_date_period_s = 0,
};

void settings_init(settings_iface_s i)
{
    eeprom = (eeprom_s)i;
}

settings_restore_state_e settings_restore(settings_s * settings)
{
    settings_restore_state_e result = SETTINGS_RESTORE_FAILED_DEFAULT_LOADED;

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
        result = SETTINGS_RESTORE_OK;
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
