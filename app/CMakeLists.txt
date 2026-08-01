cmake_minimum_required(VERSION 3.20.0)

# Include Zephyr
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(biomedical_sensor_node)

# Add source files
target_sources(app PRIVATE 
    src/main.c
    src/sensor_service.c
    src/processing.c
    src/state_machine.c
    src/telemetry.c
    src/watchdog.c
)

# Include directories
target_include_directories(app PRIVATE include)
