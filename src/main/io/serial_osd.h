#pragma once

#include "common/time.h"
#include <stdbool.h>
#include <stdint.h>

void initSerialOsd(void);
void processSerialOsd(timeUs_t currentTimeUs);

// OSD interface
const char* getSerialOsdMessage(void);
bool isSerialOsdMessageValid(timeUs_t currentTime);

// Configuration interface  
uint8_t getSerialOsdSelectedPort(void);

bool isSerialOsdPortConfigured(void);

bool wasSerialOsdInitCalled(void);

bool isSerialOsdPortOpen(void);
