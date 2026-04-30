# 5780_MiniProj_VisionArm

## Purpose

This project serves as a proof of concept for a robot arm that mimics a human's movements. By utilizing a gyroscope attached to an STM32F072 Discovery board, we rotate the board in any direction to receive an angular velocity. We use this angular velocity to set the position of three brushed DC motors with quadrature encoders by utilizing a PI feedback loop. Each axis of angular velocity measured by the gyroscope is sent through I2C to a corresponding STM32 board to actuate its dedicated motor.

This approach of using multiple STM32 boards allows the system to control any number of motors (within the 1024 address limit of I2C). While this distributed approach has an increased cost compared to other solutions due to the extra STM32 boards required, it allows for increased customization for the functionality of each motor, such as different PI feedback tunings for two motors that are controlled by x-axis rotations. Additionally, each STM32 board can easily implement functionality that is related to that motor, such as cutting power to its motor due to overcurrent or overtemperature signals.

![alt text](<documentation/images/STM32 and Motor Diagram.png>)

## Usage

To set up this project, first look at the [BOM](<BoM.csv>) section to gather all necessary components. Begin by dwonaloading the KiCad project [HERE](<Motor Driver PCB.zip>), ordering 3 copies of it, and then soldering them with the parts in the  [BOM](<BoM.csv>) so that they can drive the DC motors that will be used with the STM32 Discovery Boards. After the motor drivers are correctly able to drive the DC motors, integrating basic position control using PID will be the next step in the process. Simply verifying position control works with the buttons on the STM32 Board is fine for this step. The gyroscopes on the STM32 Discovery Boards aren't stellar, so extremly accurate position control may be tough. Once position control of a DC motor using just one STM32 Discovery board is working, correctly using the gyroscope on the board to control the position on the DC motor will be next. The amount of responsiveness is up to whoever is building it. Integrating I2C and delegating two slave STM32 Boards and one master STM32 Boards will be next. See the Pinouts section for an exmaple of how we configured our pins, but different pins can be configured and this project will have the same functionality if configured correctly. Below will be an example of how to wire the boards together using I2C with the two 10k pullup resistors and the pins they are connected to. Some sort of breadboard may be helpful for connecting all three of the boards together using I2C. Once the I2C is working and verified through some simple data over it, send the Master Board's gyroscope readings over. Once that is complete and working, and each of the individual components work with one another, driving the 3 serpeate motors using the Master Boards gyroscope will be complete, just make sure to have one repsective axis driving one respective motor, but even this is up to you as well.

![alt text](<documentation/images/I2C Wiring Schematic.png>) 


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
