# 5780_MiniProj_VisionArm

## Purpose

This project serves as a proof of concept for a robot arm that mimics a human's movements. By utilizing a gyroscope attached to an STM32F072 Discovery board, we rotate the board in any direction to receive an angular velocity. We use this angular velocity to set the position of three brushed DC motors with quadrature encoders by utilizing a PI feedback loop. Each axis of angular velocity measured by the gyroscope is sent through I2C to a corresponding STM32 board to actuate its dedicated motor.

This approach of using multiple STM32 boards allows the system to control any number of motors (within the 1024 address limit of I2C). While this distributed approach has an increased cost compared to other solutions due to the extra STM32 boards required, it allows for increased customization for the functionality of each motor, such as different PI feedback tunings for two motors that are controlled by x-axis rotations. Additionally, each STM32 board can easily implement functionality that is related to that motor, such as cutting power to its motor due to overcurrent or overtemperature signals.

![alt text](<documentation/images/STM32 and Motor Diagram.png>)

## Usage

<!--FIXME: Write usage notes-->

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
