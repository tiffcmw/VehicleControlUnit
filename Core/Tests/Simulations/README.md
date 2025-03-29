# Documentation for Each File in Simulation

## 1. CMakeLists.txt
### Purpose: Build and Test Integration
The CMakeLists.txt file defines the build system for the Vehicle Control Unit (VCU) project. It integrates the core simulation infrastructure and test suite into the build process.

#### Key Contributions to the Simulation 
- Core Library Integration:
Adds the vcu_core library, which includes the `io_simulation.c` file. This ensures that the core simulation logic is compiled and linked properly.
- Test Integration:
Adds the Tests directory, enabling the inclusion of unit tests for validating the simulation infrastructure.
- Encapsulation:
Ensures that only the necessary headers (Inc) are exposed to other components, preserving encapsulation.

#### Relation to PR
- Supports the unified simulation framework by integrating all components into a single build system.
- Facilitates the development of a test suite for validating buffer behavior, as outlined in Section 4.1 of the original documentation.

## 2. io_simulation.h
### Purpose: API for Digital and Analog I/O Simulation
The `io_simulation.h` file defines the public API for managing digital and analog buffers. It provides controlled interfaces for interacting with the simulation infrastructure.

#### Key Contributions to the Simulation 
- Buffer Management:
Defines `DigitalBuffer` and `AnalogBuffer` structures for managing digital and analog I/O states.
- Declares functions for initializing, updating, and accessing buffers (`initSimulationBuffers`, `updateDigitalBuffers`, `updateAnalogBuffers`).
- Encapsulation:
Exposes only the necessary functions for buffer manipulation, preventing direct access to private variables.
- Standardized Interfaces:
Provides consistent APIs for setting and retrieving digital and analog values (`setDigitalPinState`, `getDigitalPinState`, `setAnalogValue`, `getAnalogValue`).

#### Relaion to PR
- Implements buffer wrapper classes to expose controlled simulation interfaces (Section 4.1).
- Addresses the limitation of direct manipulation of private variables by providing encapsulated APIs.

## 3. io_simulation.c

### Purpose: Core Simulation Logic
The `io_simulation.c` file implements the core functionality for simulating digital and analog I/O behavior. It manages buffers, enforces timing control, and processes vehicle control logic.

#### Key Contributions to the Simulation 
- Digital I/O Simulation (100 Hz):
Implements cupdateDigitalBuffers` to update digital buffers at a 100 Hz refresh rate.
Provides spoofing functions (`setDigitalPinState`) for programmatic manipulation of digital inputs.
- Analog I/O Simulation (1000 Hz):
Implements `updateAnalogBuffers` to update analog buffers at a 1000 Hz refresh rate.
Provides controlled data injection through `setAnalogValue` and `getAnalogValue`.
- Timing Control:
Ensures timing accuracy in the main function by updating buffers at the required rates (100 Hz for digital, 1000 Hz for analog).
- Vehicle Control Logic:
Simulates vehicle behavior by processing inputs (e.g., throttle, brake, engine temperature, battery voltage) and triggering appropriate actions.

#### Relation to PR
- Implements timing-accurate simulation capabilities for digital and analog I/O (Section 3.3).
- Provides a realistic environment for validating control algorithms without modifying private variables (Section 1).
- Addresses the need for manual and programmatic buffer manipulation (Section 4.1).


## 4. sim_control.h

### Purpose: Simulation Control Interface
The `sim_control.h` file defines the interface for controlling the simulation environment. It provides mechanisms for time-based input sequences, event triggers, and runtime variable monitoring.

#### Key Contributions to the Simulation Infrastructure
- Time-Based Input Sequences:
Defines the `SimSequenceEvent` structure for scheduling input changes at specific timestamps.
- Event Triggers:
Defines the `SimTrigger` structure for monitoring conditions (e.g., thresholds) and triggering callbacks.
- Runtime Variable Monitoring:
Declares `simMonitorVariable` for tracking and logging runtime variables during simulation.

#### Relation to PR
- Implements the simulation control interface described in Section 4.2.
- Supports time-based input sequence definition and event triggers for specific conditions.


## 5. sim_control.c
### Purpose: Implementation of Simulation Control
The `sim_control.c` file implements the simulation control interface defined in sim_control.h. It manages time-based sequences, triggers, and runtime monitoring.

#### Key Contributions to the Simulation Infrastructure
- Time-Based Input Sequences:
Implements `simAddSequenceEvent` to schedule input changes at specific timestamps.
Processes sequences in `simUpdateControl` to update digital and analog buffers based on the simulation timeline.
- Event Triggers:
Implements `simAddTrigger` to define conditions (e.g., thresholds) and callbacks.
Monitors conditions in simUpdateControl and invokes callbacks when conditions are met.
- Runtime Variable Monitoring:
Implements `simMonitorVariable` to track and log runtime variables for debugging and analysis.

#### Relation to PR
- Provides controlled execution speed through `simSetTimeScalev (Section 4.2).
- Enables runtime variable monitoring and event triggers for specific conditions (Section 4.2).

## 6. io_simulation_test.c
### Purpose: Unit Testing for Simulation Infrastructure
The `io_simulation_test.c` file contains unit tests for validating the behavior and integrity of the simulation infrastructure.

#### Key Contributions to the Simulation Infrastructure
- Buffer Initialization:
Tests the initialization of digital and analog buffers to ensure they start in a clean state.
- Buffer Updates:
Validates the behavior of `updateDigitalBuffers` and `updateAnalogBuffers` for timing accuracy.
- Controlled Access:
Tests the functionality of `setDigitalPinState`, `getDigitalPinState`, `setAnalogValue`, and `getAnalogValue`.

#### Relation to PR
- Supports the development of a test suite for validating buffer behavior (Section 4.1).
- Ensures the robustness of the simulation infrastructure by testing edge cases and expected behavior.


## 7. control_example.c
### Purpose: Example Usage of Simulation Control
The `control_example.c` file demonstrates how to use the simulation infrastructure to define sequences, triggers, and runtime monitoring.

#### Key Contributions to the Simulation Infrastructure
- Time-Based Input Sequences:
Demonstrates the use of simAddSequenceEvent to schedule a throttle input at a specific timestamp.
- Event Triggers:
Shows how to define a trigger for over-temperature conditions using simAddTrigger.
- Runtime Monitoring:
Demonstrates the use of simMonitorVariable to track engine temperature during simulation.

#### Relation to PR
- Provides a usage example for the simulation control interface (Section 4.2).
- Demonstrates how to create a time-based simulation control flow (Section 5).



## Summary:
- CMakeLists.txt: Build and test integration. Supports unified framework and test suite development.
- io_simulation.h: API for digital and analog I/O simulation. Provides encapsulated, standardized interfaces for buffer management.
- io_simulation.c: Core simulation logic. Implements timing-accurate digital and analog I/O simulation and vehicle control logic.
- sim_control.h: Simulation control interface. Defines interfaces for sequences, triggers, and runtime monitoring.
- sim_control.c: Implementation of simulation control. Implements time-based sequences, event triggers, and runtime monitoring.
- io_simulation_test.c: Unit testing for simulation infrastructure. Validates buffer behavior and ensures robustness of the simulation infrastructure.
- control_example.c: Example usage of simulation control. Demonstrates how to use the simulation infrastructure for sequences, triggers, and runtime monitoring.