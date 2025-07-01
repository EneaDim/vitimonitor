# Makefile for Zephyr Project (mock_sensor_zbus_mqtt)

# Default board target for QEMU RISC-V 32-bit; can be overridden by command line, e.g.:
#   make build BOARD=qemu_cortex_m3
BOARD ?= qemu_riscv32

APP_NAME := mock_sensor_zbus_mqtt
BUILD_DIR := build

.PHONY: all build run flash clean help

all: build

setup:
	@echo "Run this in your shell BEFORE building or running:"
	@echo "  source ~/zephyrproject/zephyr/zephyr-env.sh"

build:
	@echo "==> Building Zephyr application for board: $(BOARD)"
	west build -b $(BOARD) -d $(BUILD_DIR)

run: build
	@echo "==> Running Zephyr application in QEMU for board: $(BOARD)"
	west build -d $(BUILD_DIR) -t run

flash:
	@echo "==> Flashing application to device (BOARD=$(BOARD))"
	west flash -b $(BOARD)

clean:
	@echo "==> Cleaning build directory"
	west build -d $(BUILD_DIR) -t clean

help:
	@echo "Available targets:"
	@echo "  make setup                 - Setup zephyr environment"
	@echo "  make build [BOARD=<board>] - Build the application for the specified board (default: $(BOARD))"
	@echo "  make run   [BOARD=<board>] - Build and run the app in QEMU emulator for the specified board"
	@echo "  make flash [BOARD=<board>] - Flash the application to real hardware (device connected)"
	@echo "  make clean                 - Clean build files"
	@echo "  make help                  - Show this help message"
	@echo ""
	@echo "To change the target board, specify BOARD variable. Example:"
	@echo "  make build BOARD=qemu_cortex_m3"
	@echo "  make run BOARD=qemu_cortex_m3"

