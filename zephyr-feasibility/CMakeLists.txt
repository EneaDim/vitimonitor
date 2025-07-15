# Specify minimum CMake version
cmake_minimum_required(VERSION 3.20.0)

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

# Aggiungi i pacchetti richiesti da Zephyr
zephyr_include_directories(${ZEPHYR_BASE}/subsys/net/lora)
