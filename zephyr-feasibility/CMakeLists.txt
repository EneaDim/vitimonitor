# Specify minimum CMake version
cmake_minimum_required(VERSION 3.20.0)

set(ZEPHYR_EXTRA_MODULES "${CMAKE_SOURCE_DIR}/modules/mock_sensor")


# Add Zephyr Package
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Name the project
project(
  vitimonitor
  VERSION 1.0
  DESCRIPTION ""
  LANGUAGES C
)

# Locate source code
target_sources(app PRIVATE src/main.c)

#target_include_directories(app PRIVATE include)

# Aggiungi i pacchetti richiesti da Zephyr
zephyr_include_directories(${ZEPHYR_BASE}/subsys/net/lora)

# Overlay
set(DTC_OVERLAY_FILE ${CMAKE_CURRENT_SOURCE_DIR}/boards/esp32s3.overlay)
