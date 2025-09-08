#ifndef RENNSSELAERMOTORSPORT_MESSAGE_FORMAT_H
#define RENNSSELAERMOTORSPORT_MESSAGE_FORMAT_H

#include <stdio.h>
#include <stdarg.h>

// Message types
typedef enum {
    MSG_SENSOR_VALUE,
    MSG_SYSTEM_STATUS,
    MSG_WARNING,
    MSG_ERROR,
    MSG_DEBUG,
    MSG_CAN_TX,
    MSG_CAN_RX,
    MSG_TIMER_STATS
} MessageType;

// Structured message function
void sendMessage(const char* sender, MessageType type, const char* format, ...);
void sendSensorValue(const char* sensor_name, float value, const char* unit);
void sendSystemStatus(const char* system_name, const char* status);

#endif // RENNSSELAERMOTORSPORT_MESSAGE_FORMAT_H