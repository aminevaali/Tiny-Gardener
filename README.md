# Tiny Gardener Project
Garden watering automation system via arduino.

## How to use?
### 1. Implement the hardware
#### 1.1. Grab a CH340 Arduino Uno board
#### 1.2. Connecting the humidity sensor to A0 pin
#### 1.3. Configuring and connecting the Sim800L module to the project.
- Power supply
- Same ground with the arduino board
- rx and tx pins (connecting <u>rx of sim800</u> to <u>tx of arduino</u> and <u>vise versa</u>)
#### 1.4. Connectiong relay modules to Arduino. (valve1 relay to pin 5 and valve2 relay to pin 6)

### 2. Compile and run the software
#### 2.1. Development environment: <u>vs code ide</u> and <u>platformio extension</u>
#### 2.2. Inject allowed phone numbers into your arduino
#### 2.3. Comment extra codes by running extra_code_commenter.py
#### 2.4. Compile and upload the program on arduino