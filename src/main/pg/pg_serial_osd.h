#pragma once

#include "pg/pg.h"

typedef struct serialOsdConfig_s {
    uint8_t selected_port;  // Which port (1-8) to use for OSD, 0 = disabled
} serialOsdConfig_t;

PG_DECLARE(serialOsdConfig_t, serialOsdConfig);