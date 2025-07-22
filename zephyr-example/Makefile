#BOARD   ?= esp32s3_devkitc/esp32s3/procpu
#OVERLAY ?= esp32s3_devkitc
BOARD   ?= native_sim
OVERLAY ?= native_sim

ORANGE  :=\033[38;5;214m
RESET   :=\033[0m

all: config build run

config:
	cmake -S . -B build -DBOARD=$(BOARD) -DDTC_OVERLAY_FILE=boards/$(OVERLAY).overlay

menuconfig: config
	cmake --build build --target menuconfig

build:
	cmake --build build

run:
	cmake --build build --target run

clean:
	rm -rf build

west-build:
	west build -p always -b $(BOARD) -- -DDTC_OVERLAY_FILE=boards/$(OVERLAY).overlay

west-run:
	west build -t run

help:
	@echo "$(ORANGE)"
	@echo "Makefile targets:"
	@echo ""
	@echo "all         Run config, build, and run"
	@echo "config      Configure the build with CMake"
	@echo "menuconfig  Run menuconfig (interactive config)"
	@echo "build       Build using CMake"
	@echo "run         Run using CMake"
	@echo "west-build  Build using west (recommended)"
	@echo "west-run    Run using west (if supported)"
	@echo "clean       Remove build directory"
	@echo "help        Show this help message"
	@echo "$(RESET)"
