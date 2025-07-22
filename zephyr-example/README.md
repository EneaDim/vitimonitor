HVAC emulator
=============

Overview
********

This repository contains a samples Zephyr application which show
how we can use emulator and simulator facilities in Zephyr.

Building and Running
====================

This application can be built and executed as follows:

```
west build -f -p -b native_sim  workspace/hvac/
```

The executable binary is available in 

```
build/zephyr/zephyr.exe
```
To run the application, simply execute it in terminal

Sample Output
=============

```
*** Booting Zephyr OS build v3.7.0-rc2-46-g4dd4746d1e5d ***
EEPROM Size: 256
EEPROM WRITE ret: 0
ret: 0, buf: aa, bb
fake adc ret: 0
adc read ret: 0
ADC READ RET :0
ADC Read val: 1499
Current Temp: 1499, Desired Temp: 2500, motor bias: 250
adc read ret: 0
ADC READ RET :0
ADC Read val: 1748
Current Temp: 1748, Desired Temp: 2500, motor bias: 237
adc read ret: 0
ADC READ RET :0
ADC Read val: 1984

```
