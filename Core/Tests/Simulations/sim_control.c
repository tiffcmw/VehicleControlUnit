#include "sim_control.h"
#include "io_simulation.h"

#define MAX_SEQUENCE_EVENTS 100
#define MAX_TRIGGERS 20
#define MAX_MONITORED_VARS 50

static float timeScale = 1.0f;
static SimSequenceEvent sequences[MAX_SEQUENCE_EVENTS];
static uint32_t sequenceCount = 0;
static SimTrigger triggers[MAX_TRIGGERS];
static uint32_t triggerCount = 0;

typedef struct {
    const char* name;
    void* ptr;
    size_t size;
} MonitoredVar;

static MonitoredVar monitoredVars[MAX_MONITORED_VARS];
static uint32_t monitorCount = 0;

void simSetTimeScale(float scale) {
    timeScale = scale;
}

void simAddSequenceEvent(SimSequenceEvent event) {
    if (sequenceCount < MAX_SEQUENCE_EVENTS) {
        sequences[sequenceCount++] = event;
    }
}

void simAddTrigger(SimTrigger trigger) {
    if (triggerCount < MAX_TRIGGERS) {
        triggers[triggerCount++] = trigger;
    }
}

void simMonitorVariable(const char* name, void* ptr, size_t size) {
    if (monitorCount < MAX_MONITORED_VARS) {
        monitoredVars[monitorCount].name = name;
        monitoredVars[monitorCount].ptr = ptr;
        monitoredVars[monitorCount].size = size;
        monitorCount++;
    }
}

void simUpdateControl(uint32_t timestamp) {
    // Process time-based sequences
    for (uint32_t i = 0; i < sequenceCount; i++) {
        if (sequences[i].timestamp == (uint32_t)(timestamp * timeScale)) {
            if (sequences[i].pin < 16) {
                setDigitalPinState(sequences[i].pin, (uint8_t)sequences[i].value);
            } else {
                setAnalogValue(sequences[i].pin - 16, sequences[i].value);
            }
        }
    }

    // Check triggers
    for (uint32_t i = 0; i < triggerCount; i++) {
        float currentValue = getAnalogValue(triggers[i].pin);
        switch (triggers[i].condition) {
            case TRIGGER_ABOVE:
                if (currentValue > triggers[i].threshold) {
                    triggers[i].callback();
                }
                break;
            case TRIGGER_BELOW:
                if (currentValue < triggers[i].threshold) {
                    triggers[i].callback();
                }
                break;
            case TRIGGER_EQUALS:
                if (currentValue == triggers[i].threshold) {
                    triggers[i].callback();
                }
                break;
        }
    }
}