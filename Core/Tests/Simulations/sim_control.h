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

// Advanced simulation control
typedef struct {
    uint32_t startTime;
    uint32_t endTime;
    uint32_t interval;
    void (*callback)(uint32_t timestamp);
} SimInterval;

void simAddInterval(SimInterval interval);
void simPause(void);
void simResume(void);
void simReset(void);
void simJumpToTime(uint32_t timestamp);
void simSetAutoRepeat(bool enabled);

// Advanced simulation scenarios
typedef struct {
    const char* name;
    SimSequenceEvent* events;
    uint32_t eventCount;
    bool repeatable;
    uint32_t repeatInterval;
} SimScenario;

// Simulation states
typedef enum {
    SIM_STATE_STOPPED,
    SIM_STATE_RUNNING,
    SIM_STATE_PAUSED,
    SIM_STATE_ERROR
} SimState;

// New simulation control functions
void simLoadScenario(SimScenario* scenario);
void simSaveState(const char* filename);
void simLoadState(const char* filename);
SimState simGetState(void);
void simSetSpeed(float speedMultiplier);
uint32_t simGetElapsedTime(void);

#endif /* SIM_CONTROL_H */