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
    struct {
        uint32_t minProcessingTime;
        uint32_t maxProcessingTime;
        float avgProcessingTime;
        uint32_t samplesCount;
    } performance;
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

void updatePerformanceStats(uint32_t processingTime) {
    bufferStats.performance.minProcessingTime = 
        MIN(processingTime, bufferStats.performance.minProcessingTime);
    bufferStats.performance.maxProcessingTime = 
        MAX(processingTime, bufferStats.performance.maxProcessingTime);
    bufferStats.performance.avgProcessingTime = 
        (bufferStats.performance.avgProcessingTime * bufferStats.performance.samplesCount + processingTime) /
        (bufferStats.performance.samplesCount + 1);
    bufferStats.performance.samplesCount++;
}

// Buffer Monitoring Initialization
static bool isInitialized = false;

void initBufferMonitor(void) {
    if (!isInitialized) {
        bufferStats.totalMessages = 0;
        bufferStats.overflows = 0;
        bufferStats.warningCount = 0;
        bufferStats.errorCount = 0;
        bufferStats.startTime = (uint32_t)time(NULL);
        isInitialized = true;
    }
}

void cleanupBufferMonitor(void) {
    stopBufferLogging();
    isInitialized = false;
}

// Buffer Utilization Tracking
static uint32_t maxBufferSize = 1000; // Default size

void setMaxBufferSize(uint32_t size) {
    maxBufferSize = size;
}

float getBufferUtilization(void) {
    return (float)bufferStats.totalMessages / maxBufferSize * 100.0f;
}

void checkBufferUtilization(void) {
    float utilization = getBufferUtilization();
    if (utilization > 80.0f) {
        logBufferMessage(LOG_LEVEL_WARNING, "Buffer utilization above 80%");
        if (overflowCallback) {
            overflowCallback();
        }
    }
}

// Periodic Status Reporting
static uint32_t lastStatusTime = 0;
static const uint32_t STATUS_INTERVAL = 1000; // 1 second

void updateBufferStatus(uint32_t currentTime) {
    if (currentTime - lastStatusTime >= STATUS_INTERVAL) {
        char statusMsg[128];
        snprintf(statusMsg, sizeof(statusMsg), 
                "Buffer Status: Messages=%lu, Util=%.1f%%, Warnings=%lu",
                bufferStats.totalMessages,
                getBufferUtilization(),
                bufferStats.warningCount);
        logBufferMessage(LOG_LEVEL_INFO, statusMsg);
        lastStatusTime = currentTime;
    }
}

#define MAX_BUFFER_MESSAGES 1000

static struct {
    char* messages[MAX_BUFFER_MESSAGES];
    uint32_t head;
    uint32_t tail;
    bool full;
} messageBuffer = {0};

void addBufferMessage(const char* message) {
    if (messageBuffer.full) {
        // Remove oldest message
        free(messageBuffer.messages[messageBuffer.tail]);
        messageBuffer.tail = (messageBuffer.tail + 1) % MAX_BUFFER_MESSAGES;
    }
    
    messageBuffer.messages[messageBuffer.head] = strdup(message);
    messageBuffer.head = (messageBuffer.head + 1) % MAX_BUFFER_MESSAGES;
    messageBuffer.full = messageBuffer.head == messageBuffer.tail;
}