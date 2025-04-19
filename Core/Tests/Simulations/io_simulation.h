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

// Buffer monitoring functions
typedef struct {
    uint32_t overflows;
    uint32_t updates;
    float utilization;
} BufferStats;

void getDigitalBufferStats(BufferStats* stats);
void getAnalogBufferStats(BufferStats* stats);
void resetBufferStats(void);
void enableBufferWarnings(uint8_t threshold);

typedef enum {
    IO_SUCCESS = 0,
    IO_INVALID_PIN,
    IO_INVALID_CHANNEL,
    IO_BUFFER_FULL,
    IO_BUFFER_EMPTY,
    IO_NOT_INITIALIZED
} IOStatus;

// Update function signatures to return status
IOStatus setDigitalPinState(uint8_t pin, uint8_t state);
IOStatus setAnalogValue(uint8_t channel, float value);
IOStatus getAnalogValue(uint8_t channel, float* value);
IOStatus getDigitalPinState(uint8_t pin, uint8_t* state);

#endif /* IO_SIMULATION_H */