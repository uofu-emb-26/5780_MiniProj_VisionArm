#include <stdint.h>
#include <stm32f0xx_hal.h>
#include <stm32f0xx_hal_gpio.h>

void My_HAL_GPIO_Init(GPIO_TypeDef  *GPIOx, GPIO_InitTypeDef *GPIO_Init);
void My_HAL_GPIO_DeInit(GPIO_TypeDef  *GPIOx, uint32_t GPIO_Pin);
GPIO_PinState My_HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void My_HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
void My_HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

void I2C_Write(I2C_TypeDef* I2C, uint8_t device_address, uint8_t nbytes, char data[]);

void My_HAL_USART_SetBaudRate(USART_TypeDef* USART, uint32_t baud_rate);
void My_HAL_USART_WriteChar(USART_TypeDef* USART, char character);
void My_HAL_USART_WriteString(USART_TypeDef* USART, char* string);
char My_HAL_USART_ReadChar(USART_TypeDef* USART);
