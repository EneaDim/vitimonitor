BOARD   ?=esp32s3_devkitc/esp32s3/procpu
OVERLAY ?=esp32s3

ORANGE  :=\033[38;5;214m
RESET   :=\033[0m

all:: config build run

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

help:
	@echo "$(ORANGE)"
	@echo "Makefile targets:"
	@echo ""
	@echo "config     Configure the build system"
	@echo "menuconfig Configure with menuconfig"
	@echo "build      Build the project"
	@echo "run        Run the project"
	@echo "clean      Clean the build directory"
	@echo "help       Show help message"
	@echo "$(RESET)"
