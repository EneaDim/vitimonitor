import os
import argparse

def generate_env(c_make_version, project_name, language, output_folder):
    cmake_version = c_make_version if c_make_version else "3.20.0"
    language = language if language else "C"
    # CMakeLists.txt
    content = f'''# Specify minimum CMake version
cmake_minimum_required(VERSION {cmake_version})

# Add Zephyr Package
find_package(Zephyr REQUIRED HINTS $ENV{{ZEPHYR_BASE}})

# Name the project
project(
  {project_name}
  VERSION 1.0
  DESCRIPTION ""
  LANGUAGES {language}
)

# Locate source code
target_sources(app PRIVATE src/main.c)
'''
    os.makedirs(output_folder, exist_ok=True)
    output_path = os.path.join(output_folder, "CMakeLists.txt")

    if not os.path.exists(output_path):
        with open(output_path, "w") as file:
            file.write(content)
        print("CMakeLists.txt generated successfully.")

    # Makefile
    content = f'''BOARD   ?= qemu_riscv64
OVERLAY ?= app\n
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
'''
    os.makedirs(output_folder, exist_ok=True)
    output_path = os.path.join(output_folder, "Makefile")

    if not os.path.exists(output_path):
        with open(output_path, "w") as file:
            file.write(content)
        print("Makefile generated successfully.")

    content = ''
    output_path = os.path.join(output_folder, "prj.conf")
    if not os.path.exists(output_path):
        with open(output_path, "w") as file:
            file.write(content)
        print("prj.conf generated successfully.")
    dir_name = output_folder + '/boards'
    if not os.path.exists(dir_name):
        # Create app.overlay
        os.makedirs(dir_name, exist_ok=True)
    boards_files = ['overlay']
    for i in range(2):
        content = ''
        output_path = os.path.join(dir_name, "board."+boards_files[i]+"") 
        if not os.path.exists(output_path):
            with open(output_path, "w") as file:
                file.write(content)
            print("app."+boards_files[i]+" generated successfully.")
    # Create src/main.c if it doesn't exist
    content = "//#include <stdio.h>\n"
    content += "#include <zephyr/kernel.h>\n"
    content += "//#include <zephyr/drivers/gpio.h>\n\n"
    content += "void main(void) {\n\n"
    content += "    // TODO: Add application code here\n\n"
    content += "    return 0;\n"
    content += "}\n"
    src_dir = os.path.join(output_folder, "src")
    os.makedirs(src_dir, exist_ok=True)
    main_c_path = os.path.join(src_dir, "main.c")
    if not os.path.exists(main_c_path):
        with open(main_c_path, "w") as main_file:
            main_file.write(content)
        print(f"{main_c_path} generated successfully.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate CMakeLists.txt for a Zephyr project.")
    parser.add_argument("-p", "--project_name", required=True, help="The name of the project")
    parser.add_argument("-v", "--c_make_version", default=None, help="Minimum required CMake version (default: 3.20.0)")
    parser.add_argument("-l", "--languages", default=None, help="Languages used in the project")
    parser.add_argument("-o", "--output_folder", default=".", help="Folder where CMakeLists.txt will be generated")

    args = parser.parse_args()
    generate_env(args.c_make_version, args.project_name, args.languages, args.output_folder)

