#ifndef GYRO_H
#define GYRO_H
#include "stm32f0xx_hal.h"
#include "stm32f072xb.h"

#define GYRO_DEADBAND 200 //Used to ignore noise thats within +- 200
#define GYRO_SCALE 16 //Scales the raw readings down for motor

void gyro_init(void);
int16_t gyro_readX(void);

#endif