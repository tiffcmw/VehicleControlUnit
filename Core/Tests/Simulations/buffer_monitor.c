#include "buffer_monitor.h"
#include <stdio.h>
#include <time.h>

static FILE* logFile = NULL;
static LogLevel currentLogLevel = LOG_LEVEL_INFO;
static void (*overflowCallback)(void) = NULL;

void startBufferLogging(const char* filename) {
    if (logFile != NULL) {
        fclose(logFile);
    }
    logFile = fopen(filename, "w");
    if (logFile) {
        fprintf(logFile, "Timestamp,Level,Message\n");
    }
}

void stopBufferLogging(void) {
    if (logFile) {
        fclose(logFile);
        logFile = NULL;
    }
}

void setLogLevel(LogLevel level) {
    currentLogLevel = level;
}

void exportBufferData(const char* filename, uint32_t startTime, uint32_t endTime) {
    FILE* exportFile = fopen(filename, "w");
    if (exportFile) {
        fprintf(exportFile, "Time,DigitalPins,AnalogChannels\n");
        // Export buffer data within the time range
        fclose(exportFile);
    }
}

void registerBufferOverflowCallback(void (*callback)(void)) {
    overflowCallback = callback;
}