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

target_sources(vcu_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/sim_config.c
)

add_executable(sim_config_test
    sim_config_test.c
)
target_link_libraries(sim_config_test vcu_core unity)
add_test(NAME sim_config_test COMMAND sim_config_test)