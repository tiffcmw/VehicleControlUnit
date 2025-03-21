cmake_minimum_required(VERSION 3.10)
project(VehicleControlUnit)

# Enable testing
enable_testing()

# Add core library
add_library(vcu_core
    Core/Src/main.c
    Core/Src/io_simulation.c
)

target_include_directories(vcu_core PUBLIC
    Core/Inc
)

# Add test directory
add_subdirectory(Tests)