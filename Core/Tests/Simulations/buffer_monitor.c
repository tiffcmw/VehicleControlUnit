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

static struct {
    uint32_t totalMessages;
    uint32_t overflows;
    uint32_t warningCount;
    uint32_t errorCount;
    uint32_t startTime;
} bufferStats = {0};

void logBufferMessage(LogLevel level, const char* message) {
    if (level >= currentLogLevel && logFile != NULL) {
        time_t now;
        time(&now);
        fprintf(logFile, "%lu,%d,%s\n", (unsigned long)now, level, message);
        
        bufferStats.totalMessages++;
        switch(level) {
            case LOG_LEVEL_WARNING:
                bufferStats.warningCount++;
                break;
            case LOG_LEVEL_ERROR:
                bufferStats.errorCount++;
                break;
        }
    }
}

void getBufferStats(void) {
    if (logFile) {
        fprintf(logFile, "\nBuffer Statistics:\n");
        fprintf(logFile, "Total Messages: %lu\n", bufferStats.totalMessages);
        fprintf(logFile, "Overflows: %lu\n", bufferStats.overflows);
        fprintf(logFile, "Warnings: %lu\n", bufferStats.warningCount);
        fprintf(logFile, "Errors: %lu\n", bufferStats.errorCount);
    }
}