#include "../../Inc/Utils/MessageFormat.h"
#include <stdio.h>
#include <stdarg.h>

void sendMessage(const char* sender, MessageType type, const char* format, ...) {
    const char* type_strings[] = {
        "SENSOR_VALUE",
        "SYSTEM_STATUS", 
        "WARNING",
        "ERROR",
        "DEBUG",
        "CAN_TX",
        "CAN_RX",
        "TIMER_STATS"
    };
    
    printf("Sender:%s;InfoType:%s;Content:", sender, type_strings[type]);
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    printf("\r\n");
}

void sendSensorValue(const char* sensor_name, float value, const char* unit) {
    printf("Sender:%s;InfoType:SENSOR_VALUE;Value:%.3f;Unit:%s\r\n", 
           sensor_name, value, unit);
}

void sendSystemStatus(const char* system_name, const char* status) {
    printf("Sender:%s;InfoType:SYSTEM_STATUS;Status:%s\r\n", 
           system_name, status);
}