# Matra Fan Controller - Arduino Code Documentation

## Overview

This Arduino code is designed to control a fan system based on environmental data collected from two BME280 sensors (one inside and one outside). The code runs on an ESP8266 microcontroller, which also serves as a Wi-Fi access point with an embedded web server to view logged data. The code includes functionality for reading sensor data, controlling a fan via PWM, logging data to the LittleFS filesystem, and displaying information on an OLED screen.


### Conditions That Turn the Fan On

The fan will be turned on if **all** of the following conditions are met:

1. **Sanity Checks Pass**:
   - **Inside Temperature (tempIn)** is between `-20.0°C` and `50.0°C`.
   - **Outside Temperature (tempOut)** is between `-20.0°C` and `50.0°C`.
   - **Inside Humidity (humIn)** is greater than `1.0%`.
   - **Outside Humidity (humOut)** is greater than `1.0%`.
   - **Inside Pressure (presIn)** is greater than `100.0 hPa`.
   - **Outside Pressure (presOut)** is greater than `100.0 hPa`.

2. **Temperature Conditions**:
   - **Case 1: Warm Outdoors (tempOut > 15.0°C)**:
     - The fan will turn on **if the outside air is drier than the inside air** (i.e., `moistOut < moistIn`).
   
   - **Case 2: Moderate Temperature Outdoors (5.0°C < tempOut <= 15.0°C)**:
     - The fan will turn on **if the outside air is warmer than the inside air** (`tempOut > tempIn`) **and** the outside air is drier than the inside air (`moistOut < moistIn`).

3. **Special Case: Freezing Outdoors (tempOut <= 5.0°C)**:
   - The fan **will not** turn on when the outside temperature is near or below freezing, regardless of humidity or moisture content.

### Summary of Fan Activation Logic

- **Warm Outside (> 15.0°C)**: The fan turns on if the outside air is drier than inside.
- **Moderate Temperature (5.0°C to 15.0°C)**: The fan turns on if the outside air is both warmer and drier than inside.
- **Cold Outside (≤ 5.0°C)**: The fan remains off to prevent bringing in freezing air.

This logic is designed to optimize indoor air quality and comfort by using the fan to exchange air with the outside when it is beneficial (i.e., when the outside air is warmer and/or drier) while avoiding conditions that could bring in excessively cold or humid air.

## Features

- **Environmental Monitoring**: Reads temperature, humidity, and pressure from two BME280 sensors.
- **Fan Control**: Adjusts fan speed based on environmental conditions, with PWM control on GPIO15 (D8).
- **Data Logging**: Logs sensor data to a file (`data.log`) in the LittleFS filesystem.
- **Web Server**: Hosts a web server at `192.168.4.1` to serve the log file contents.
- **OLED Display**: Displays real-time sensor readings and system status on an SSD1306 OLED display.
- **Serial Commands**: Supports `dump` and `delete` commands to view or delete the log file via the Serial Monitor.
- **Daily Reset**: Automatically resets the ESP8266 every 24 hours to ensure consistent operation.

## Hardware Connections

- **BME280 Sensors**: Connected via I2C to the ESP8266.
  - Sensor 1 (Inside): I2C address `0x76`
  - Sensor 2 (Outside): I2C address `0x77`
- **PWM Fan Control**: PWM output on GPIO15 (D8).
- **OLED Display**: Connected via I2C, with the I2C address `0x3D`.
- **Analog Input**: Reads an analog value from pin A0 to control fan speed.

## Code Components

### Libraries

- **Wire.h**: For I2C communication.
- **Adafruit_GFX.h, Adafruit_SSD1306.h**: For OLED display control.
- **Adafruit_Sensor.h, Adafruit_BME280.h**: For BME280 sensor handling.
- **LittleFS.h**: For file system operations on the ESP8266.
- **ESP8266WiFi.h, ESP8266WebServer.h**: For Wi-Fi and web server functionalities.

### Pin Configuration

- `A0`: Reads analog input for fan speed control.
- `D8 (GPIO15)`: Outputs PWM signal to control fan speed.
- `D7`: Digital pin used to turn the fan on/off.

### Setup Function

- **Wi-Fi Initialization**: Sets up the ESP8266 as an access point with SSID `MATRA_FAN`.
- **BME280 Initialization**: Initializes the two BME280 sensors.
- **File System Initialization**: Mounts the LittleFS filesystem.
- **Web Server Initialization**: Starts the web server to serve log data.
- **OLED Initialization**: Initializes the SSD1306 OLED display.

### Loop Function

- **Sensor Reading**: Every 10 seconds, reads data from both BME280 sensors.
- **Fan Control Logic**: Determines whether to turn the fan on or off based on temperature and humidity differences between inside and outside.
- **Data Logging**: Logs sensor readings and fan status every 20 minutes to the `data.log` file.
- **OLED Update**: Updates the OLED display with the latest sensor readings and system status.
- **Web Server Handling**: Continuously handles incoming client requests to the web server.
- **Serial Command Handling**: Checks for `dump` and `delete` commands to manage the log file.
- **Daily Reset**: Resets the ESP8266 after 24 hours of operation.

### Serial Commands

- **dump**: Prints the contents of the `data.log` file to the Serial Monitor.
- **delete**: Deletes the `data.log` file from the LittleFS filesystem.

### Fan Control Logic

The fan is turned on or off based on the following conditions:

1. **Sanity Checks**: Ensure sensor readings are within reasonable ranges (e.g., temperatures between -20°C and 50°C).
2. **Outdoor Conditions**:
   - If the outside temperature is above 15°C and the outside air is drier, the fan turns on.
   - If the outside temperature is between 5°C and 15°C and the outside air is both warmer and drier, the fan turns on.
   - If the outside temperature is below 5°C, the fan remains off.

### Display Output

- **Temperature (Inside/Outside)**
- **Humidity (Inside/Outside)**
- **Moisture Content (Inside/Outside)**
- **Pressure (Inside/Outside)**
- **Fan Status**
- **Analog Input Value**
- **Uptime (Seconds)**

### Logging Format

Logs are saved in the following format:
```
<time_in_seconds>, T1:<temp_in>, H1:<hum_in>, P1:<pres_in>, M1:<moist_in>, T2:<temp_out>, H2:<hum_out>, P2:<pres_out>, M2:<moist_out>, fan=<fan_status>, A=<analog_value>
```

### Reset

The ESP8266 resets itself every 24 hours to prevent potential issues with long-term operation.

The fan in the system is controlled based on the environmental conditions measured by two BME280 sensors, one placed inside and one placed outside. The decision to turn the fan on is based on a set of rules that compare the temperature, humidity, and moisture content between the inside and outside environments.


## Conclusion

This code provides a comprehensive solution for environmental monitoring and fan control using the ESP8266, with added functionality for data logging, web server access, and OLED display output. The system is designed to operate autonomously, with built-in safeguards such as periodic resets and the ability to manually manage log files via serial commands.
