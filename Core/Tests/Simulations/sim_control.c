#include "sim_control.h"
#include "buffer_monitor.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

// Static variables for simulation control
static struct {
    SimState state;
    float timeScale;
    float speedMultiplier;
    uint32_t elapsedTime;
    bool autoRepeat;
    SimScenario* activeScenario;
    uint32_t scenarioStartTime;
} simControl = {
    .state = SIM_STATE_STOPPED,
    .timeScale = 1.0f,
    .speedMultiplier = 1.0f,
    .elapsedTime = 0,
    .autoRepeat = false,
    .activeScenario = NULL,
    .scenarioStartTime = 0
};

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

void simLoadScenario(SimScenario* scenario) {
    if (scenario == NULL) {
        simControl.state = SIM_STATE_ERROR;
        logBufferMessage(LOG_LEVEL_ERROR, "Invalid scenario pointer");
        return;
    }
    
    simControl.activeScenario = scenario;
    simControl.scenarioStartTime = simControl.elapsedTime;
    simControl.state = SIM_STATE_RUNNING;
    
    char logMsg[128];
    snprintf(logMsg, sizeof(logMsg), "Loaded scenario: %s", scenario->name);
    logBufferMessage(LOG_LEVEL_INFO, logMsg);
}

void simSaveState(const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        logBufferMessage(LOG_LEVEL_ERROR, "Failed to open state file for writing");
        return;
    }
    
    fwrite(&simControl, sizeof(simControl), 1, file);
    fclose(file);
    
    logBufferMessage(LOG_LEVEL_INFO, "Simulation state saved");
}

void simLoadState(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        logBufferMessage(LOG_LEVEL_ERROR, "Failed to open state file for reading");
        return;
    }
    
    fread(&simControl, sizeof(simControl), 1, file);
    fclose(file);
    
    logBufferMessage(LOG_LEVEL_INFO, "Simulation state loaded");
}

SimState simGetState(void) {
    return simControl.state;
}

void simSetSpeed(float speedMultiplier) {
    if (speedMultiplier <= 0.0f || speedMultiplier > 10.0f) {
        logBufferMessage(LOG_LEVEL_WARNING, "Invalid speed multiplier");
        return;
    }
    
    simControl.speedMultiplier = speedMultiplier;
    char logMsg[64];
    snprintf(logMsg, sizeof(logMsg), "Simulation speed set to %.2fx", speedMultiplier);
    logBufferMessage(LOG_LEVEL_INFO, logMsg);
}

uint32_t simGetElapsedTime(void) {
    return simControl.elapsedTime;
}

void simAddInterval(SimInterval interval) {
    if (interval.startTime >= interval.endTime) {
        logBufferMessage(LOG_LEVEL_ERROR, "Invalid interval times");
        return;
    }
    
    // Store interval for processing
    // Implementation depends on interval management strategy
}

void simPause(void) {
    if (simControl.state == SIM_STATE_RUNNING) {
        simControl.state = SIM_STATE_PAUSED;
        logBufferMessage(LOG_LEVEL_INFO, "Simulation paused");
    }
}

void simResume(void) {
    if (simControl.state == SIM_STATE_PAUSED) {
        simControl.state = SIM_STATE_RUNNING;
        logBufferMessage(LOG_LEVEL_INFO, "Simulation resumed");
    }
}

void simReset(void) {
    simControl.elapsedTime = 0;
    simControl.state = SIM_STATE_STOPPED;
    simControl.activeScenario = NULL;
    logBufferMessage(LOG_LEVEL_INFO, "Simulation reset");
}

void simJumpToTime(uint32_t timestamp) {
    simControl.elapsedTime = timestamp;
    char logMsg[64];
    snprintf(logMsg, sizeof(logMsg), "Jumped to time: %lu ms", timestamp);
    logBufferMessage(LOG_LEVEL_INFO, logMsg);
}

void simSetAutoRepeat(bool enabled) {
    simControl.autoRepeat = enabled;
    logBufferMessage(LOG_LEVEL_INFO, enabled ? 
        "Auto-repeat enabled" : "Auto-repeat disabled");
}

// Update function to be called in the simulation loop
void simUpdate(void) {
    if (simControl.state != SIM_STATE_RUNNING) {
        return;
    }
    
    simControl.elapsedTime += (uint32_t)(simControl.timeScale * simControl.speedMultiplier);
    
    // Process active scenario if exists
    if (simControl.activeScenario) {
        processScenarioEvents();
    }
}