#include "io_simulation.h"

#define MAX_DIGITAL_PINS 16
#define MAX_ANALOG_CHANNELS 8

// Pin definitions
#define THROTTLE_SENSOR_PIN     0
#define BRAKE_SENSOR_PIN        1
#define ENGINE_TEMP_CHANNEL     0
#define BATTERY_VOLTAGE_CHANNEL 1

static DigitalBuffer digital_buffers[MAX_DIGITAL_PINS];
static AnalogBuffer analog_buffers[MAX_ANALOG_CHANNELS];

void initSimulationBuffers(void) {
    for (int i = 0; i < MAX_DIGITAL_PINS; i++) {
        digital_buffers[i].timestamp = 0;
        digital_buffers[i].pin_state = 0;
    }
    
    for (int i = 0; i < MAX_ANALOG_CHANNELS; i++) {
        analog_buffers[i].timestamp = 0;
        analog_buffers[i].value = 0.0f;
    }
}

void updateDigitalBuffers(uint32_t timestamp) {
    // Update digital buffers at 100Hz
    for (int i = 0; i < MAX_DIGITAL_PINS; i++) {
        digital_buffers[i].timestamp = timestamp;
    }
}

void updateAnalogBuffers(uint32_t timestamp) {
    // Update analog buffers at 1000Hz
    for (int i = 0; i < MAX_ANALOG_CHANNELS; i++) {
        analog_buffers[i].timestamp = timestamp;
    }
}

void setDigitalPinState(uint8_t pin, uint8_t state) {
    if (pin < MAX_DIGITAL_PINS) {
        digital_buffers[pin].pin_state = state;
    }
}

void setAnalogValue(uint8_t channel, float value) {
    if (channel < MAX_ANALOG_CHANNELS) {
        analog_buffers[channel].value = value;
    }
}

float getAnalogValue(uint8_t channel) {
    if (channel < MAX_ANALOG_CHANNELS) {
        return analog_buffers[channel].value;
    }
    return 0.0f;
}

uint8_t getDigitalPinState(uint8_t pin) {
    if (pin < MAX_DIGITAL_PINS) {
        return digital_buffers[pin].pin_state;
    }
    return 0;
}

void processVehicleInputs(void) {
    // Read simulated throttle position (digital)
    uint8_t throttlePressed = getDigitalPinState(THROTTLE_SENSOR_PIN);
    
    // Read simulated brake position (digital)
    uint8_t brakePressed = getDigitalPinState(BRAKE_SENSOR_PIN);
    
    // Read simulated engine temperature (analog)
    float engineTemp = getAnalogValue(ENGINE_TEMP_CHANNEL);
    
    // Read simulated battery voltage (analog)
    float batteryVoltage = getAnalogValue(BATTERY_VOLTAGE_CHANNEL);
    
    // Vehicle control logic
    if (throttlePressed && !brakePressed) {
        // Accelerate
        setMotorPower(calculateThrottleResponse(engineTemp));
    } else if (brakePressed) {
        // Brake
        setMotorPower(0);
        activateBrakeSystem();
    }
    
    // Battery management
    if (batteryVoltage < 10.0f) {
        setFaultStatus(BATTERY_LOW_FAULT);
    }
    
    // Temperature management
    if (engineTemp > 95.0f) {
        setFaultStatus(OVERTEMP_FAULT);
        activateCoolingSystems();
    }
}

int main(void) {
    // Initialize simulation buffers
    initSimulationBuffers();
    
    uint32_t timestamp = 0;
    
    while (1) {
        // Update buffers at appropriate rates
        if (timestamp % 10 == 0) {  // 100 Hz for digital
            updateDigitalBuffers(timestamp);
        }
        if (timestamp % 1 == 0) {   // 1000 Hz for analog
            updateAnalogBuffers(timestamp);
        }
        
        // Process vehicle control logic
        processVehicleInputs();
        
        HAL_Delay(1);  // 1ms delay for timing
        timestamp++;
    }
}