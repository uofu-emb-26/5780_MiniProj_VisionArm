#include <stdbool.h>
#include "main.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_it.h"
#include "stm32f072xb.h"
#include "stm32f0xx_hal_i2c.h"

// ***** External Declarations *****
uint8_t I2C_read = true;    // Set to true for a read transaction
uint8_t I2C_address = 0;
char I2C_message[I2C_MAX_MESSAGE_LEN];
uint8_t I2C_nbytes = 0;
uint8_t I2C_chain = false;    // Set to true to chain another transaction after the current one
uint8_t I2C_error = 0;


// ***** Internal Declarations *****
static uint8_t nbytes_left = 0;


// ***** Helper Function Prototypes *****

/**
  * @brief A helper function for setting the CR2 registers of an I2C peripheral
  *        for writing.
  * This function does not set the START bit.
  *
  * See the `My_HAL_I2C_Write` function for additional parameter details.
  * @param rd_wrn: A boolean that is true if the transaction is a read;
  *                otherwise, false.
  * @retval None
  */
static void I2C_Setup(I2C_TypeDef* I2C, uint8_t device_address, uint8_t nbytes, uint8_t rd_wrn);

static void I2C_HandleTXIS(void);
static void I2C_HandleRXNE(void);
static void I2C_HandleNACK(void);
static void I2C_HandleTC(void);

/******************************************************************************/
/*           Cortex-M0 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
   while (1)
  {
  }
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
}

/******************************************************************************/
/* STM32F0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f0xx.s).                    */
/******************************************************************************/


void I2C2_IRQHandler(void)
{
  if (I2C2->ISR & I2C_ISR_TXIS) {
    I2C_HandleTXIS();
  }
  else if (I2C2->ISR & I2C_ISR_RXNE) {
    I2C_HandleRXNE();
  }
  else if (I2C2->ISR & I2C_ISR_NACKF) {
    I2C_HandleNACK();
  }
  else if (I2C2->ISR & I2C_ISR_TC) {
    I2C_HandleTC();
  }
  else {
    // Set transmission parameters in CR2 register
    I2C_Setup(I2C2, I2C_address, I2C_nbytes, I2C_read);

    if (I2C_nbytes > I2C_MAX_MESSAGE_LEN) {
      I2C_error = NBYTES_INVALID;
      return;
    }

    nbytes_left = I2C_nbytes;

    // Start the transmission
    I2C2->CR2 |= I2C_CR2_START;
  }
}


// ***** Helper Functions *****

static void I2C_HandleTXIS(void)
{
  nbytes_left--;
  I2C->TXDR = I2C_message[nbytes_left] & 0xFF;
}

static void I2C_HandleTC(void)
{
  // FIXME: This function has no way to indicate which transaction in the chain it is handling
  if (I2C_chain) {
    I2C_Setup(I2C2, I2C_address, I2C_nbytes, I2C_read);

    I2C2->CR2 |= I2C_CR2_START;
  }
  else {
    I2C->CR2 |= I2C_CR2_STOP;
  }
}

static void I2C_Setup(I2C_TypeDef* I2C, uint8_t device_address, uint8_t nbytes, uint8_t rd_wrn)
{
  // Set the address of the slave device (7-bit address is the bits SADD[7:1])
  // Set the number of bytes to write or read
  // Set RD_WRN based on the type of transaction
  I2C->CR2 &= ~(I2C_CR2_NBYTES | I2C_CR2_SADD | I2C_CR2_RD_WRN);
  I2C->CR2 |= (nbytes << I2C_CR2_NBYTES_Pos) | (device_address << (I2C_CR2_SADD_Pos + 1) | (rd_wrn << I2C_CR2_RD_WRN_Pos));
}
