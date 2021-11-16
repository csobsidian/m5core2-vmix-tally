# M5Stack Core2 vMix Tally Indicator

A tally light indicator for vMix which runs on the M5Stack Core2.

![Live image](/img/live.jpg)

## Requirements

- [Arduino IDE](https://www.arduino.cc/en/software)
- [M5Stack Core2](https://github.com/m5stack/M5Core2)

## Dependencies

- [IniFile 1.3.0](https://www.arduino.cc/reference/en/libraries/inifile/)

## Usage

1. Install the Arduino IDE and setup the environment for the M5Stack Core 2 by following the instructions [here](https://docs.m5stack.com/en/quick_start/core2/arduino).
2. Install the IniFile library via the arduino library manager.
3. Modify the vMix.ini file to match your requirements (ssid, password, vmix ip address).
4. Copy your modified vMix.ini file to a micro SD card and insert into the Core2 device.
5. Compile the program using the Arduino IDE and transfer it to the Core2.