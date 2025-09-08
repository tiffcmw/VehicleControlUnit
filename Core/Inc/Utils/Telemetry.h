#ifndef RENSSELAERMOTORSPORT_TELEMETRY_H
#define RENSSELAERMOTORSPORT_TELEMETRY_H

#include <stdint.h>
#include <stdbool.h>
#include "Units.h"

typedef struct {
    char name[32];
    UnitId unit_id;               // Use unit ID instead of string
    uint32_t expected_rate_ms;
    uint32_t last_update;
    float custom_min;             // Custom limits (overrides unit defaults)
    float custom_max;
    bool use_custom_limits;       // Whether to use custom or unit default limits
    bool enabled;
} TelemetrySignal;

// Initialize telemetry utility
void initTelemetry(void);

// Register a telemetry signal - returns pointer for O(1) access
TelemetrySignal* registerTelemetrySignal(const char* name, UnitId unit_id, 
                                        uint32_t expected_rate_ms, 
                                        float custom_min, float custom_max);

// Fast telemetry send using pointer (O(1) operation)
void sendTelemetryValue(TelemetrySignal* signal, float value);

// Type-safe telemetry send functions
void sendTelemetryFloat(TelemetrySignal* signal, float value);
void sendTelemetryInt(TelemetrySignal* signal, int32_t value);
void sendTelemetryUInt(TelemetrySignal* signal, uint32_t value);
void sendTelemetryBool(TelemetrySignal* signal, bool value);

// Fallback send by name (avoid if possible)
void sendTelemetryValueByName(const char* name, float value, UnitId unit_id);

// Send configuration and health functions
void sendTelemetryConfiguration(void);
void checkTelemetryHealth(void);
void handleTelemetryConfigRequest(void);
const char* getCategoryName(UnitCategory category);

#endif // RENSSELAERMOTORSPORT_TELEMETRY_H