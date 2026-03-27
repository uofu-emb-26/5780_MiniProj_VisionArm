#include <stdint.h>
#include <stdbool.h>
#include <stm32f0xx_hal.h>
#include <stm32f0xx_hal_gpio.h>
#include "stm32f0xx_it.h"

/*
void My_HAL_GPIO_Init(GPIO_TypeDef  *GPIOx, GPIO_InitTypeDef *GPIO_Init)
{
}
*/

/*
void My_HAL_GPIO_DeInit(GPIO_TypeDef  *GPIOx, uint32_t GPIO_Pin)
{
}
*/

/*
GPIO_PinState My_HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
return -1;
}
*/

/*
void My_HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
}
*/

/*
void My_HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
}
*/

void I2C_Write(I2C_TypeDef* I2C, uint8_t device_address, uint8_t nbytes, char data[])
{
  I2C_Transaction nextTransaction = {
    nbytes,
    device_address,
    false,
    false,
    data
  };

  while (I2C_nextTransaction == NULL) {
    // Spin loop
  }

  if (I2C->ISR & I2C_ISR_BUSY) {
    I2C_SetNextTransaction(I2C, &nextTransaction);
  }
  else {

    // Set transmission parameters in CR2 register
    I2C_Setup(I2C2, &nextTransaction);

    // Start the transmission
    I2C2->CR2 |= I2C_CR2_START;
  }
}
