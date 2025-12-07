#include "platform.h"

#ifdef USE_SERIAL_OSD

#include "pg/pg.h"
#include "pg/pg_ids.h"
#include "pg/pg_serial_osd.h"

PG_REGISTER_WITH_RESET_FN(serialOsdConfig_t, serialOsdConfig, PG_SERIAL_OSD_CONFIG, 0);

void pgResetFn_serialOsdConfig(serialOsdConfig_t *config)
{
    config->selected_port = 0;  // Disabled by default
}

#endif