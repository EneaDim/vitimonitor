# Zephyr Project Setup Guide

This guide walks you through setting up a Python virtual environment, installing West, building a sample Zephyr application, and setting environment variables.

---

## 1. Create and Activate Python Virtual Environment

Navigate to your Zephyr project directory and set up a virtual environment:

# PYTHON VENV
cd ~/zephyrproject
python3 -m venv ~/zephyrproject/.venv
source ~/zephyrproject/.venv/bin/activate


---

## 2. Install and Set Up West

Install West and initialize your workspace:

# WEST
pip install west
west init
west update
west zephyr-export
west packages pip --install


---

## 3. Install Zephyr SDK and Build Sample

Change to the Zephyr directory and install the SDK:


# SDK
cd ~/zephyrproject/zephyr
west sdk install


Build and run the `hello_world` sample for the `qemu_x86` board:


west build -b qemu_x86 samples/hello_world
west build -t run


---

## 4. Export Environment Variables

Set the required environment variables for the Zephyr toolchain:



# EXPORT
export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
export ZEPHYR_BASE=/home/eneadim/zephyrproject/zephyr
