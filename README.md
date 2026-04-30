# 5780_MiniProj_VisionArm

## Purpose

This project serves as a proof of concept for a robot arm that mimics a human's movements. By utilizing a gyroscope attached to an STM32F072 Discovery board, we rotate the board in any direction to receive an angular velocity. We use this angular velocity to set the position of three brushed DC motors with quadrature encoders by utilizing a PI feedback loop. Each axis of angular velocity measured by the gyroscope is sent through I2C to a corresponding STM32 board to actuate its dedicated motor.

This approach of using multiple STM32 boards allows the system to control any number of motors (within the 1024 address limit of I2C). While this distributed approach has an increased cost compared to other solutions due to the extra STM32 boards required, it allows for increased customization for the functionality of each motor, such as different PI feedback tunings for two motors that are controlled by x-axis rotations. Additionally, each STM32 board can easily implement functionality that is related to that motor, such as cutting power to its motor due to overcurrent or overtemperature signals.

![alt text](<documentation/images/STM32 and Motor Diagram.png>)

## Usage

To set up this project, first look at the [BOM](<BoM.csv>) section to gather all necessary components for the motor driver boards. Begin by downloading the KiCad project [HERE](<Motor Driver PCB.zip>), ordering 3 copies of it, and then soldering them with the parts in the [BOM](<BoM.csv>) so that they can drive the DC motors that will be used with the STM32 Discovery Boards.

After aquiring all STM32 boards and motor driver boards, a master device can be chosen and the boards can be connected together. See the Pinouts section for an exmaple of how each board's pins are configured. Below is an example of how to wire the boards together using I2C with two 10k pull-up resistors and the pins they are connected to on the master and slave STM32 boards. A breadboard may be helpful for connecting all three of the boards together using I2C.

![alt text](<documentation/images/I2C Wiring Schematic.png>)

Once the boards are wired together, the STM32 boards can be flashed with the master device code or the slave device code. Designate one STM32 as the master device to read gyroscope data. Flash the master device with the following CMake build commands:

```bash
# Create build object directory
cmake -B build -S ./

# Build and flash master code
cmake --build build --target flash_master
```

After the master device is flashed, each of the slave devices can be flashed. Before flashing a slave device, its I2C address macro must be changed within the [slave/Src/slave.c](<slave/Src/slave.c>) file on line 9 shown below.

```C
#define SLAVE_ADDRESS 0x10    // *** 0x10 is for Y-axis, 0x12 is for Z-axis
```

After changing the slave address, a slave STM32 can be flashed with the following CMake build command:

```bash
# Build and flash slave code
cmake --build build --target flash_slave
```

Once each device is flashed with the appropriate code, the master device's gyroscope will control the position of all three motors.

## Pinouts

### Master Device

The following table gives the pinouts for connecting the STM32 board acting as the master device to a motor driver board:

| Function      | Pin |
| ------------- | --- |
| Current Sense | PA1 |
| Motor PWM     | PA4 |
| Motor In 1    | PA8 |
| Motor In 2    | PA5 |
| Encoder A     | PB5 |
| Encoder B     | PB4 |

The following table gives the pinouts for connecting the STM32 board acting as the master device to the I2C bus between STM32s:

| Function       | Pin  |
| -------------- | ---- |
| STM32 I2C2 SCL | PB13 |
| STM32 I2C2 SDA | PB11 |
| Gyroscope SDA  | PB15 |

### Slave Device

The following table gives the pinouts for connecting the STM32 board acting as a slave device to a motor driver board:

| Function      | Pin |
| ------------- | --- |
| Current Sense | PA1 |
| Motor PWM     | PA4 |
| Motor In 1    | PA8 |
| Motor In 2    | PA5 |
| Encoder A     | PB5 |
| Encoder B     | PB4 |

The following table gives the pinouts for connecting the STM32 board acting as a slave device to the I2C bus between STM32s:

| Function       | Pin  |
| -------------- | ---- |
| STM32 I2C2 SCL | PB13 |
| STM32 I2C2 SDA | PB11 |
