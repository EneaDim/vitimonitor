# Specify minimum CMake version
cmake_minimum_required(VERSION 3.20.0)

# Define the Zephyr base directory, which should be set by the environment variable ZEPHYR_BASE
# The ZEPHYR_BASE variable is usually provided by the Zephyr SDK or your development environment setup
# In case you want to explicitly set it, you can uncomment the next line:
# set(ZEPHYR_BASE "/path/to/zephyr")

# Overlay file for the device tree configuration
set(DTC_OVERLAY_FILE ${CMAKE_CURRENT_SOURCE_DIR}/boards/esp32s3.overlay)

# Add Zephyr package and setup for the project
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Define the project with a version and description
project(
  vitimonitor
  VERSION 1.0
  DESCRIPTION "Temperature monitoring system with sensor reading and LoRa transmission"
  LANGUAGES C
)

# Include the source code file for the application
target_sources(app PRIVATE src/main.c)

# Optionally, if you have an include directory for headers, uncomment and adjust as necessary
# target_include_directories(app PRIVATE include)

# Add the required Zephyr packages for LoRa support (or other needed packages)
# Ensure that the LoRa package is available in your Zephyr environment
zephyr_include_directories(${ZEPHYR_BASE}/subsys/net/lora)

# If your project requires any additional modules, like mock sensors, uncomment the line below:
# set(ZEPHYR_EXTRA_MODULES "${CMAKE_SOURCE_DIR}/modules/mock_sensor")


