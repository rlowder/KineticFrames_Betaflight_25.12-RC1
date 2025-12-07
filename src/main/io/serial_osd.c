#include "platform.h"

#ifdef USE_SERIAL_OSD

#include "common/utils.h"
#include "drivers/serial.h"
#include "io/serial.h"
#include "scheduler/scheduler.h"
#include "pg/pg_serial_osd.h"
#include "common/maths.h"
#include "common/printf.h"
#include <string.h>

// Active serial port for OSD display
static serialPort_t *serialOsdPort = NULL;
static uint8_t activePortNumber = 0;

// Buffer management - ALL UNCOMMENTED
#define SERIAL_BUFFER_SIZE 256
#define OSD_MESSAGE_SIZE 30
static char serialBuffer[SERIAL_BUFFER_SIZE];
static char osdMessageBuffer[OSD_MESSAGE_SIZE];
static uint16_t bufferIndex = 0;
static timeUs_t lastMessageTime = 0;
static bool hasValidMessage = false;
static uint32_t bytesReceived = 0;
#define MESSAGE_LENGTH 20  // Set to your message length

// Statistics
static uint32_t messageCount = 0;

// Convert port number to identifier - UNCOMMENTED
static serialPortIdentifier_e getPortIdentifier(uint8_t port)
{
    switch (port) {
        case 1: return SERIAL_PORT_USART1;
        case 2: return SERIAL_PORT_USART2;
        case 3: return SERIAL_PORT_USART3;
        case 4: return SERIAL_PORT_UART4;
        case 5: return SERIAL_PORT_UART5;
        case 6: return SERIAL_PORT_USART6;
        case 7: return SERIAL_PORT_USART7;
        case 8: return SERIAL_PORT_USART8;
        default: return SERIAL_PORT_NONE;
    }
}

void initSerialOsd(void)
{
    hasValidMessage = true;
    strcpy(osdMessageBuffer, "SERIAL: STARTING");
    
    const serialOsdConfig_t *config = serialOsdConfig();
    
    // Close existing port if open
    if (serialOsdPort) {
        closeSerialPort(serialOsdPort);
        serialOsdPort = NULL;
    }
    
    activePortNumber = 0;
    bufferIndex = 0;
    
    // Check if disabled
    if (config->selected_port == 0) {
        strcpy(osdMessageBuffer, "SERIAL: DISABLED");
        return;
    }
    
    // Get port identifier
    serialPortIdentifier_e portId = getPortIdentifier(config->selected_port);
    if (portId == SERIAL_PORT_NONE) {
        tfp_sprintf(osdMessageBuffer, "UART%d: BAD PORT", config->selected_port);
        return;
    }
    
    tfp_sprintf(osdMessageBuffer, "UART%d: OPENING...", config->selected_port);
    
    // Try to open the port
serialOsdPort = openSerialPort(
    portId,
    FUNCTION_NONE,
    NULL, NULL,
    115200,
    MODE_RX,
    SERIAL_NOT_INVERTED | SERIAL_STOPBITS_2 | SERIAL_PARITY_NO
    );
    
    if (serialOsdPort) {
        activePortNumber = config->selected_port;
        tfp_sprintf(osdMessageBuffer, "UART%d: READY", config->selected_port);
        lastMessageTime = 0;
    } else {
        tfp_sprintf(osdMessageBuffer, "UART%d: OPEN FAILED", config->selected_port);
    }
}

void processSerialOsd(timeUs_t currentTimeUs)
{
    if (!serialOsdPort) {
        return;
    }
    
    // Process limited bytes per call to avoid blocking
    const int MAX_BYTES_PER_CALL = 32;
    int bytesProcessed = 0;
    
    while (serialRxBytesWaiting(serialOsdPort) > 0 && bytesProcessed < MAX_BYTES_PER_CALL) {
        uint8_t byte = serialRead(serialOsdPort);
        bytesReceived++;
        bytesProcessed++;
        
        if (byte == 0x00) {
            if (bufferIndex > 0) {
                serialBuffer[bufferIndex] = '\0';
                
                // Only copy to display buffer when needed
                memcpy(osdMessageBuffer, serialBuffer, MIN(bufferIndex + 1, OSD_MESSAGE_SIZE));
                osdMessageBuffer[OSD_MESSAGE_SIZE - 1] = '\0';
                
                lastMessageTime = currentTimeUs;
                messageCount++;
                hasValidMessage = true;
                bufferIndex = 0;
            }
        } else if (byte >= 32 && byte <= 126) {
            if (bufferIndex < SERIAL_BUFFER_SIZE - 1) {
                serialBuffer[bufferIndex++] = byte;
            } else {
                bufferIndex = 0; // Reset on overflow
            }
        }
    }
    
    // Check timeout separately, less frequently
    static timeUs_t lastTimeoutCheck = 0;
    if (currentTimeUs - lastTimeoutCheck > 500000) { // Check every 500ms
        lastTimeoutCheck = currentTimeUs;
        if (lastMessageTime != 0 && currentTimeUs - lastMessageTime > 2000000) {
            tfp_sprintf(osdMessageBuffer, "UART%d: TIMEOUT", activePortNumber);
            hasValidMessage = true;
            lastMessageTime = currentTimeUs;
        }
    }
}

bool isSerialOsdPortOpen(void)
{
    return serialOsdPort != NULL;
}

const char* getSerialOsdMessage(void)
{
    return hasValidMessage ? osdMessageBuffer : "NO VALID MSG";
}

bool isSerialOsdMessageValid(timeUs_t currentTime)
{
    UNUSED(currentTime);
    return hasValidMessage;
}

uint8_t getSerialOsdSelectedPort(void)
{
    const serialOsdConfig_t *config = serialOsdConfig();
    return config->selected_port;
}

bool isSerialOsdPortConfigured(void)
{
    const serialOsdConfig_t *config = serialOsdConfig();
    return config->selected_port > 0;
}

#endif