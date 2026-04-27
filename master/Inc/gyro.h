#ifndef __GYRO_H
#define __GYRO_H

#include "stm32f0xx_hal.h"
#include "stm32f072xb.h"

void gyro_init(void);
int16_t gyro_readX(void);
int16_t gyro_readY(void);

#endif