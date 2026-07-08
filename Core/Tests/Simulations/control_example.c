#include "sim_control.h"

void onOverTemp(void) {
    printf("WARNING: Over temperature detected!\n");
}

int main(void) {
    // Initialize simulation
    initSimulationBuffers();
    
    // Set up a sequence event
    SimSequenceEvent throttleEvent = {
        .timestamp = 1000,  // 1 second
        .pin = THROTTLE_SENSOR_PIN,
        .value = 1.0f
    };
    simAddSequenceEvent(throttleEvent);
    
    // Set up a trigger
    SimTrigger tempTrigger = {
        .pin = ENGINE_TEMP_CHANNEL,
        .threshold = 95.0f,
        .condition = TRIGGER_ABOVE,
        .callback = onOverTemp
    };
    simAddTrigger(tempTrigger);
    
    // Monitor engine temperature
    float engineTemp = 0.0f;
    simMonitorVariable("Engine Temperature", &engineTemp, sizeof(float));
    
    // Run simulation at 2x speed
    simSetTimeScale(2.0f);
    
    uint32_t timestamp = 0;
    while (1) {
        simUpdateControl(timestamp);
        processVehicleInputs();
        timestamp++;
        HAL_Delay(1);
    }
}