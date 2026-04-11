#include <stdint.h>
#include <stdbool.h>
#include <stm32f0xx_hal.h>
#include <stm32f0xx_hal_gpio.h>
#include "stm32f072xb.h"
#include "stm32f0xx_it.h"
#include "hal_gpio.h"

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

  while (!I2C_TransactionQueueEmpty) {
    // Spin loop
  }

  I2C_SetNextTransaction(I2C, &nextTransaction);

  if (!I2C_ongoingTransaction) {
    // Set transmission parameters in CR2 register
    I2C_Setup(I2C2, &nextTransaction);

    // Start the transmission
    I2C2->CR2 |= I2C_CR2_START;
  }
}

/**
  * @param USART: The USART peripheral to set the baud rate of.
  * @param baud_rate: The desired baud rate. The actual baud rate will be
  *                   approximated by setting the BRR register to the nearest
  *                   integer.
  */
void My_HAL_USART_SetBaudRate(USART_TypeDef* USART, uint32_t baud_rate)
{
  uint16_t BRR = HAL_RCC_GetHCLKFreq() / baud_rate;
  USART->BRR = BRR;
}

/**
  * @param USART: The USART peripheral to write a character to.
  * @param character: The character to write.
  */
void My_HAL_USART_WriteChar(USART_TypeDef *USART, char character)
{
  // Wait until the Transfer Data Register is empty
  while (!(USART->ISR & USART_ISR_TXE)) {
    // Spin loop
  }

  USART->TDR = character;
}

/**
  * @param USART: The USART peripheral to write a string to.
  * @param string: A char array that represents the string to write.
  */
void My_HAL_USART_WriteString(USART_TypeDef* USART, char* string)
{
  while (*string != '\0') {
    My_HAL_USART_WriteChar(USART, *string);
    string++;
  }
}

/**
  * @brief Blocks until a character is read from the USART peripheral.
  * @param USART: The USART peripheral to read a character from.
  * @retval The character that was read from the USART peripheral.
  */
char My_HAL_USART_ReadChar(USART_TypeDef* USART)
{
  // Block until data is received
  while (!(USART3->ISR & USART_ISR_RXNE)) {
    // Spin loop
  }

  return USART3->RDR & 0xFF;
}
