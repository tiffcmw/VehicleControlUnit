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
Implements `updateDigitalBuffers` to update digital buffers at a 100 Hz refresh rate.
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
