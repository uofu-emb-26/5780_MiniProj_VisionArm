#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "motor.h"
#include "stm32f072xb.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_gpio.h"
#include "stm32f0xx_hal_rcc.h"
#include "stm32f0xx_hal_rcc_ex.h"
#include "stm32f0xx_it.h"
#include "i2c_config.h"
#include "hal_gpio.h"

void SetupGPIO_USART(void);
void SetupUSART3(void);
void SetupLEDs(void);
void SystemClock_Config(void);
void Gyro_Init(uint8_t gyro_addr);

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

  SetupGPIO_USART();
  SetupUSART3();
  i2c_init();
  motor_init();

  NVIC_EnableIRQ(I2C2_IRQn);
  NVIC_SetPriority(I2C2_IRQn, 0);

  //i2c_setup function shifts adress already
  uint8_t device_address = (0x10);   // STM32 slave device address
  uint8_t device_address2 = 0x12;
  char* data = "Hello from master device";

  uint8_t gyro_addr = 0x69;
  Gyro_Init(gyro_addr);


  int32_t accumulated_tilt = 0;
  int16_t threshold = 800;
  while (1)
  {
    My_HAL_USART_WriteString(USART3, "main: Start of main loop\n");
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_SET);

    // 1. Send to Slave 1
    My_HAL_USART_WriteString(USART3, "main: Sending to Slave 1 (0x10)\n");
    I2C_Write(I2C2, device_address, strlen(data) + 1, data);
    
    // Give Slave 1 a tiny bit of breathing room (optional but good practice)
    HAL_Delay(10); 

    // 2. Send to Slave 2
    My_HAL_USART_WriteString(USART3, "main: Sending to Slave 2 (0x12)\n");
    I2C_Write(I2C2, device_address2, strlen(data) + 1, data);

    // 3. Wait before next big cycle
    My_HAL_USART_WriteString(USART3, "main: Cycle complete\n");
    HAL_Delay(500); 


    //---Read x axis from gyroscope
    I2C2->CR2 &= ~((0xFF << 16) | (0x3FF << 0));
    I2C2->CR2 |= (1 << 16) | (gyro_addr << 1);
    I2C2->CR2 &= ~I2C_CR2_RD_WRN;
    I2C2->CR2 |= I2C_CR2_START;

    while (!(I2C2->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF))) {}
    I2C2->TXDR = 0xA8; // x axis low byte reg
    while (!(I2C2->ISR & I2C_ISR_TC)) {}

    I2C2->CR2 &= ~((0xFF << 16) | (0x3FF << 0));
    I2C2->CR2 |= (2 << 16) | (gyro_addr << 1);
    I2C2->CR2 &= ~I2C_CR2_RD_WRN;
    I2C2->CR2 |= I2C_CR2_START;

    uint8_t gyro_data[2];
    for (int i = 0; i < 2; i++) {
      while (!(I2C2->ISR & (I2C_ISR_RXNE | I2C_ISR_NACKF))) {}
      gydro_data[i] = I2C2->RXDR;
    }
    while (!(I2C2->ISR & I2C_ISR_TC)) {}
    I2C2->CR2 |= I2C_CR2_STOP;

    // Combine bytes into signed 16-bit
    int16_t x_velocity = (int16_t) ((gyro_data[1] << 8) | gyro_data[0]);

    // Integrate velocity into angle
    if (abs(x_velocity) > threshold) {
      // Scale velocity to slow down motor
      accumulated_tilt += (x_velocity / 200);
    }

    // Update target position for motor.c PI loop
    target_position = (int16_t) accumulated_tilt;

    HAL_Delay(20); // Run loop at ~50Hz
  }

  return -1;
}


void Gyro_Init(uint8_t gyro_addr)
{
  // Clears & Sets NBYTES & SADD
  I2C2->CR2 &= ~((0xFF << 16) | (0x3FF << 0));
  I2C2->CR2 |= (2 << 16) | (gyro_addr << 1); // NBYTES = 2
  I2C2->CR2 &= ~I2C_CR2_RD_WRN; // Set to WRITE (0)
  I2C2->CR2 |= I2C_CR2_START;

  while (!(I2C2->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF))) {}
  I2C2->TXDR = 0x20; // CTRL_REG1 reg addr

  while (!(I2C2->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF))) {}
  I2C2->TXDR = 0x09; // Enable only x axis

  while (!(I2C2->ISR & I2C_ISR_TC)) {}
  I2C2->TXDR |= I2C_CR2_STOP;

  HAL_Delay(50);
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
    GPIO_MODE_AF_PP,
    GPIO_PULLUP,
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
