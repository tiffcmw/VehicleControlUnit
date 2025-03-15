#ifndef IO_SIMULATION_H
#define IO_SIMULATION_H

#include <stdint.h>

// Digital I/O simulation (100 Hz)
typedef struct {
    uint32_t timestamp;  // ms since start
    uint8_t pin_state;   // current digital state
} DigitalBuffer;

// Analog I/O simulation (1000 Hz)
typedef struct {
    uint32_t timestamp;  // ms since start
    float value;         // analog value
} AnalogBuffer;

// Buffer management functions
void initSimulationBuffers(void);
void updateDigitalBuffers(uint32_t timestamp);
void updateAnalogBuffers(uint32_t timestamp);

// Simulation control functions
void setDigitalPinState(uint8_t pin, uint8_t state);
void setAnalogValue(uint8_t channel, float value);
float getAnalogValue(uint8_t channel);
uint8_t getDigitalPinState(uint8_t pin);

#endif /* IO_SIMULATION_H */