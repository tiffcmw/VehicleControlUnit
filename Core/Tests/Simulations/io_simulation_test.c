v#include "unity.h"
#include "io_simulation.h"

void setUp(void) {
    initSimulationBuffers();
}

void tearDown(void) {
    // Nothing to clean up
}

void test_digital_buffer_initialization(void) {
    uint8_t pin = 0;
    TEST_ASSERT_EQUAL(0, getDigitalPinState(pin));
}

void test_analog_buffer_initialization(void) {
    uint8_t channel = 0;
    TEST_ASSERT_EQUAL_FLOAT(0.0f, getAnalogValue(channel));
}

void test_digital_buffer_update(void) {
    uint8_t pin = 1;
    uint8_t state = 1;
    
    setDigitalPinState(pin, state);
    TEST_ASSERT_EQUAL(state, getDigitalPinState(pin));
}

void test_analog_buffer_update(void) {
    uint8_t channel = 1;
    float value = 3.3f;
    
    setAnalogValue(channel, value);
    TEST_ASSERT_EQUAL_FLOAT(value, getAnalogValue(channel));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_digital_buffer_initialization);
    RUN_TEST(test_analog_buffer_initialization);
    RUN_TEST(test_digital_buffer_update);
    RUN_TEST(test_analog_buffer_update);
    return UNITY_END();
}