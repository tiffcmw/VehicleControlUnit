#ifndef SIM_CONTROL_H
#define SIM_CONTROL_H

#include <stdint.h>

// Time-based sequence definition
typedef struct {
    uint32_t timestamp;
    uint8_t pin;
    float value;
} SimSequenceEvent;

// Event trigger definition
typedef enum {
    TRIGGER_ABOVE,
    TRIGGER_BELOW,
    TRIGGER_EQUALS
} TriggerCondition;

typedef struct {
    uint8_t pin;
    float threshold;
    TriggerCondition condition;
    void (*callback)(void);
} SimTrigger;

// Simulation control functions
void simSetTimeScale(float scale);
void simAddSequenceEvent(SimSequenceEvent event);
void simAddTrigger(SimTrigger trigger);
void simMonitorVariable(const char* name, void* ptr, size_t size);

#endif /* SIM_CONTROL_H */