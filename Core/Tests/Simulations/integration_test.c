#include "unity.h"
#include "io_simulation.h"
#include "sim_control.h"
#include "can_simulation.h"

void setUp(void) {
    initSimulationBuffers();
    initCANSimulation();
}

void test_complete_vehicle_sequence(void) {
    // Test vehicle startup sequence
    SimSequenceEvent startupSequence[] = {
        {.timestamp = 0, .pin = KEY_ON_PIN, .value = 1},
        {.timestamp = 100, .pin = ENGINE_START_PIN, .value = 1},
        {.timestamp = 300, .pin = ENGINE_START_PIN, .value = 0}
    };
    
    for (int i = 0; i < 3; i++) {
        simAddSequenceEvent(startupSequence[i]);
    }
    
    // Run simulation for 1 second
    uint32_t timestamp;
    for (timestamp = 0; timestamp < 1000; timestamp++) {
        simUpdateControl(timestamp);
        updateDigitalBuffers(timestamp);
        updateAnalogBuffers(timestamp);
    }
    
    // Verify engine state
    TEST_ASSERT_EQUAL(1, getDigitalPinState(ENGINE_RUN_PIN));
    TEST_ASSERT_IN_RANGE(getAnalogValue(ENGINE_TEMP_CHANNEL), 20.0f, 30.0f);
}