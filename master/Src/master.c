// #include <string.h>
// #include <stdlib.h>
#include "main.h"
#include "stm32f072xb.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_gpio.h"
#include "stm32f0xx_hal_rcc.h"
#include "stm32f0xx_hal_rcc_ex.h"
#include "stm32f0xx_it.h"
#include "i2c_config.h"
#include "hal_gpio.h"
#include "motor.h"
#include "gyro.h"
// #include "SEGGER_RTT.h"

#define DEVICE_ADDRESS_Y 0x10
#define DEVICE_ADDRESS_Z 0x12

extern volatile int16_t target_position;

void SetupGPIO_USART(void);
void SetupUSART3(void);
void SetupLEDs(void);
void SystemClock_Config(void);

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* Configure the system clock */
  SystemClock_Config();

  SetupLEDs();
  i2c_init();

  motor_init();
  gyro_init();

  // For debugging
  SetupGPIO_USART();
  SetupUSART3();

  NVIC_EnableIRQ(I2C2_IRQn);
  NVIC_SetPriority(I2C2_IRQn, 0);

  uint8_t device_address_y = DEVICE_ADDRESS_Y;  // Y-axis controller address
  uint8_t device_address_z = DEVICE_ADDRESS_Z;  // Z-axis controller address

  int16_t threshold = 250;

  while (1)
  {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_SET);

    // Read raw X-axis velocity from gyroscope

    // Prevent chained blocking I2C transmissions from being interrupted
    __disable_irq();
    int16_t x_val = gyro_readX();
    int16_t y_val = gyro_readY();
    int16_t z_val = gyro_readZ();
    __enable_irq();

    // Velocity to position
    if (x_val > threshold || x_val < -threshold) {
      target_position += (x_val / 16);
    }

    if (x_val > threshold) {
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
    }
    else if (x_val < -threshold) {
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
    }

    char y_target[2] = {(char)(y_val & 0xFF), (char)(y_val >> 8)};
    char z_target[2] = {(char)(z_val & 0xFF), (char)(z_val >> 8)};

    I2C_Write(I2C2, device_address_y, 2, y_target);
    I2C_Write(I2C2, device_address_z, 2, z_target);

    HAL_Delay(40);

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);

    HAL_Delay(10); // Give time for slave's interrupt handler
  }
  return -1;
}

/**
  * @brief Initializes the GPIO pins for USART
  * @retval None
  */
void SetupGPIO_USART(void)
{
  // Initialize USART_TX on pin PC4
  GPIO_InitTypeDef initTxStr = {
    GPIO_PIN_4,
    GPIO_MODE_AF_PP,
    GPIO_NOPULL,
    GPIO_SPEED_FREQ_LOW,
    GPIO_AF1_USART3
  };

  HAL_GPIO_Init(GPIOC, &initTxStr);

  // Initialize USART_RX on pin PC5
  GPIO_InitTypeDef initRxStr = {
    GPIO_PIN_5,
    GPIO_MODE_AF_OD,
    GPIO_NOPULL,
    GPIO_SPEED_FREQ_LOW,
    GPIO_AF1_USART3
  };

  HAL_GPIO_Init(GPIOC, &initRxStr);
}

/**
  * @brief Initializes the USART3 peripheral
  * @retval None
  */
void SetupUSART3(void)
{
  __HAL_RCC_USART3_CLK_ENABLE();

  // Set baud rate
  uint32_t baud_rate = 115200;

  My_HAL_USART_SetBaudRate(USART3, baud_rate);

  // Enable TX and RX hardware
  USART3->CR1 |= USART_CR1_TE;
  USART3->CR1 |= USART_CR1_RE;

  // Enable interrupt when data is received
  // USART3->CR1 |= USART_CR1_RXNEIE;

  // Enable the USART peripheral
  USART3->CR1 |= USART_CR1_UE;
}

void SetupLEDs(void)
{
  __HAL_RCC_GPIOC_CLK_ENABLE();

  GPIO_InitTypeDef initLEDStr = {
    GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9,
    GPIO_MODE_OUTPUT_PP,
    GPIO_NOPULL,
    GPIO_SPEED_FREQ_LOW
  };

  HAL_GPIO_Init(GPIOC, &initLEDStr);
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* User can add their own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add their own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */
