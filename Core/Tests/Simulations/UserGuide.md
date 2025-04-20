# Vehicle Control Unit Simulation User Guide

## Getting Started
### 1. Setting Up the Environment

Clone the repository and navigate to the simulation directory:

```sh
cd VehicleControlUnit/Core/Tests/Simulations
```

Build the project using CMake:

```sh
mkdir build
cd build
cmake ..
make
```

### 2. Basic Simulation Setup
    
Here's a minimal example to give a brief example of the core functionalities:

```c
#include "io_simulation.h"
#include "sim_control.h"
#include "buffer_monitor.h"

int main(void) {
    // Initialize systems
    initSimulationBuffers();
    initBufferMonitor();
    
    // Start logging
    startBufferLogging("simulation.csv");
    setLogLevel(LOG_LEVEL_INFO);
    
    // Run simulation
    uint32_t timestamp = 0;
    while (timestamp < 10000) { // Run for 10 seconds
        updateDigitalBuffers(timestamp);
        updateAnalogBuffers(timestamp);
        updateBufferStatus(timestamp);
        timestamp++;
    }
    
    // Cleanup
    cleanupBufferMonitor();
    return 0;
}
```
####  Features Guide
1. Digital and Analog I/O
    ```c
    // Set digital pin states
    setDigitalPinState(THROTTLE_SENSOR_PIN, 1);
    uint8_t state = getDigitalPinState(BRAKE_SENSOR_PIN);

    // Set analog values
    setAnalogValue(ENGINE_TEMP_CHANNEL, 85.5f);
    float temp = getAnalogValue(ENGINE_TEMP_CHANNEL);
    ```

2. Time-Based Sequences
    ```c
    // Define a sequence of events
    SimSequenceEvent sequence = {
        .timestamp = 1000,  // 1 second
        .pin = THROTTLE_SENSOR_PIN,
        .value = 1.0f
    };
    simAddSequenceEvent(sequence);
    ```
3. Event Triggers
    ```c
    void onOverTemp(void) {
        printf("Warning: Engine temperature too high!\n");
    }

    // Set up temperature monitoring
    SimTrigger trigger = {
        .pin = ENGINE_TEMP_CHANNEL,
        .threshold = 90.0f,
        .condition = TRIGGER_ABOVE,
        .callback = onOverTemp
    };
    simAddTrigger(trigger);
    ```
4. Buffer Monitoring
    ```c
    // Monitor buffer utilization
    void onBufferOverflow(void) {
        printf("Warning: Buffer overflow detected!\n");
    }

    registerBufferOverflowCallback(onBufferOverflow);
    setMaxBufferSize(1000);
    checkBufferUtilization();
    ```
5. Configuration
Create a ```config.yaml``` file:
    ```yaml
    simulation:
    digital_io:
        refresh_rate: 100
        buffer_size: 1000
    analog_io:
        refresh_rate: 1000
        channel_count: 8
    monitoring:
        log_level: INFO
        buffer_warning_threshold: 80
    ```

## Best Practices
1. Initialization Order:
    - Always initialize buffers before starting simulation
    - Set up monitoring and logging early
    - Register callbacks before running simulation

2. Resource Management:
    - Clean up resources when simulation ends
    - Check buffer utilization regularly
    - Handle overflow conditions

3. Testing:
    - Write unit tests for critical components
    - Use the provided test infrastructure
    - Validate timing requirements

## Troubleshooting
Common issues and solutions:

1. Buffer Overflows:
    - Increase buffer size in config
    - Reduce data generation rate
    - Implement overflow handling

2. Timing Issues:
    - Check refresh rates match requirements
    - Verify sequence timestamps
    - Monitor system performance

3. Memory Leaks:
    - Ensure proper cleanup
    - Monitor resource usage
    - Check file handle closure


## Example Projects
See `control_example.c` for a complete working example demonstrating:
- Sequence definition
- Trigger setup
- Variable monitoring
- Buffer management

For more examples and detailed API documentation, refer to the individual header files in the simulation directory.