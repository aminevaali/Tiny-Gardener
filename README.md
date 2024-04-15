# Tiny Gardener Project
Irrigation automation system via arduino. With the ability of controll via sms and automatic watering by measuring soil humidity.

## How to use?
### 1. Hardware
#### Required Hardwares:
- Arduino Uno
- Sim800L
- Twin Relay module
- Twin washing machine valves (for irrigation)
- Adapter (220V AC to 5V DC)
- LM2596S Module (for sim800 3.8V power supplly)
- Dirt humidity sensor

#### 1.1. Grab a CH340 Arduino Uno board
#### 1.2. Connecting the humidity sensor to A0 pin
#### 1.3. Configuring and connecting the Sim800L module to the project.
- Power supply for sim800.
- Sim800 must have same ground with the arduino board.
- rx and tx pins: connect rx pin of sim800 to pin 2 of arduino and tx pin of sim800 to pin 3 of arduino. 
- Connect pin 5 and 6 of arduino to relays. Relays have to switch valves power supply. 
#### 1.4. Connectiong relay modules to Arduino. (valve1 relay to pin 5 and valve2 relay to pin 6)
---


### 2. Compile and run the software
#### 2.1. Development environment: <u>vs code ide</u> and <u>platformio extension</u>
#### 2.2. Inject allowed phone numbers into your arduino (by running phone_number_injector.cpp on the board)
#### 2.3. Comment extra codes by running extra_code_commenter.py
#### 2.4. Compile and upload the program on arduino
