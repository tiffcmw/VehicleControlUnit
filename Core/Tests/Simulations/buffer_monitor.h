#ifndef BUFFER_MONITOR_H
#define BUFFER_MONITOR_H

#include <stdint.h>

typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR
} LogLevel;

// Buffer monitoring functions
void startBufferLogging(const char* filename);
void stopBufferLogging(void);
void setLogLevel(LogLevel level);
void exportBufferData(const char* filename, uint32_t startTime, uint32_t endTime);
void registerBufferOverflowCallback(void (*callback)(void));
void initBufferMonitor(void);
void cleanupBufferMonitor(void);
void setMaxBufferSize(uint32_t size);
float getBufferUtilization(void);
void checkBufferUtilization(void);
void updateBufferStatus(uint32_t currentTime);

#endif /* BUFFER_MONITOR_H */