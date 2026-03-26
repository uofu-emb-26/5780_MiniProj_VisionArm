#include "main.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx.h"

void SystemClock_Config(void);

  /**
  * @brief  The application entry point.
  * @retval int
  */

volatile uint8_t rx_data = 0;

int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* Configure the system clock */
  SystemClock_Config();


  // Below is adapted from Lab 5 for slave setup

  // Enable clock for GPIO port B
  RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
  // Enable clock for I2C2 peripheral
  RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;

  // PB11 = I2C2 SDA (data line)
  // Clear mode bits for PB11
  GPIOB->MODER &= ~(3 << (11 * 2));

  // Set PB11 to "alternate function mode"
  // this lets the I2C hardware control the pin instead of normal GPIO
  GPIOB->MODER |=  (2 << (11 * 2));

  // Set PB11 to open-drain
  // required for I2C so multiple devices can share the same line safely
  GPIOB->OTYPER |= (1 << 11);

  // Select alternate function for PB11
  // AFR[1] is used for pins 8–15
  // AF1 corresponds to I2C2_SDA
  GPIOB->AFR[1] &= ~(0xF << 12);   // clear previous setting
  GPIOB->AFR[1] |=  (0x1 << 12);   // set AF1 (I2C2 SDA)

  // PB13 = I2C2 SCL (clock line)
  // Clear mode bits for PB13
  GPIOB->MODER &= ~(3 << (13 * 2));

  // Set PB13 to alternate function mode
  GPIOB->MODER |=  (2 << (13 * 2));

  // Set PB13 to open-drain
  GPIOB->OTYPER |= (1 << 13);

  // Select alternate function for PB13
  // AF5 corresponds to I2C2_SCL
  GPIOB->AFR[1] &= ~(0xF << 20);   // clear previous setting
  GPIOB->AFR[1] |=  (0x5 << 20);   // set AF5 (I2C2 SCL)

  // Disable I2C before setup
  I2C2->CR1 &= ~I2C_CR1_PE;

  // Timing from Lab 5
  I2C2->TIMINGR = 0x10420F13;

  // Set slave address = 0x10
  I2C2->OAR1 = (0x10 << 1) | I2C_OAR1_OA1EN;

  // Enable I2C
  I2C2->CR1 |= I2C_CR1_PE;

  while (1)
  {
    rx_data = I2C2_Receive();
  }
  return -1;
}

static void I2C2_Receive(I2C_TypeDef* I2C, uint8_t nbytes, char data[]) 
{
  while(!(I2C2->ISR & I2C_ISR_ADDR)); // Wait till Master sends matching addr
  I2C2->ICR |= I2C_ICR_ADDRCF; // Write to Interrupt Clear Register

  for(int i = 0; i < nbytes; i++) {
    while(!(I2C2->ISR & I2C_ISR_RXNE));
    data[i] = I2C->RXDR & I2C_RXDR_RXDATA;
  }

  while(!(I2C2->ISR & I2C_ISR_STOPF));
  I2C2->ICR |= I2C_ICR_STOPCF;
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
