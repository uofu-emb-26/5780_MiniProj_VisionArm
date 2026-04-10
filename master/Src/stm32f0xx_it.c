#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_it.h"
#include "stm32f072xb.h"
#include "stm32f0xx_hal_i2c.h"

// ***** External Declarations *****
I2C_Transaction* I2C_nextTransaction = NULL;
I2C_Errors I2C_error = 0;
bool I2C_ongoingTransaction = false;
bool I2C_TransactionQueueEmpty = true;


// ***** Internal Declarations *****
static I2C_Transaction currentTransaction = {0, 0, 0, 0, NULL};
static uint8_t nbytes_left = 0;


// ***** Helper Function Prototypes *****

static void I2C_GetNextTransaction(I2C_TypeDef* I2C);

static void I2C_HandleTXIS(I2C_TypeDef* I2C);
static void I2C_HandleRXNE(I2C_TypeDef* I2C);
static void I2C_HandleNACK(I2C_TypeDef* I2C);
static void I2C_HandleTC(I2C_TypeDef* I2C);

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
  if (!I2C_ongoingTransaction)
    I2C_GetNextTransaction(I2C2);

  if (I2C2->ISR & I2C_ISR_TXIS) {
    I2C_HandleTXIS(I2C2);
  }
  else if (I2C2->ISR & I2C_ISR_RXNE) {
    I2C_HandleRXNE(I2C2);
  }
  else if (I2C2->ISR & I2C_ISR_NACKF) {
    I2C_HandleNACK(I2C2);
    I2C2->ICR |= I2C_ICR_NACKCF;
  }
  else if (I2C2->ISR & I2C_ISR_TC) {
    I2C_HandleTC(I2C2);
  }
}


// ***** Helper Functions *****

void I2C_Setup(I2C_TypeDef* I2C, I2C_Transaction* transaction)
{
  // Set the address of the slave device (7-bit address is the bits SADD[7:1])
  // Set the number of bytes to write or read
  // Set RD_WRN based on the type of transaction
  I2C->CR2 &= ~(I2C_CR2_NBYTES | I2C_CR2_SADD | I2C_CR2_RD_WRN);
  I2C->CR2 |= (transaction->nbytes << I2C_CR2_NBYTES_Pos)
            | (transaction->address << (I2C_CR2_SADD_Pos + 1)
            | (transaction->read << I2C_CR2_RD_WRN_Pos));

  if (!(I2C->ISR & I2C_ISR_BUSY))
    nbytes_left = transaction->nbytes;
}

void I2C_SetNextTransaction(I2C_TypeDef* I2C, I2C_Transaction* transaction)
{
  I2C_nextTransaction = transaction;
}

static void I2C_HandleTXIS(I2C_TypeDef* I2C)
{
  I2C_ongoingTransaction = true;

  nbytes_left--;
  I2C->TXDR = currentTransaction.message[nbytes_left] & 0xFF;
}

// FIXME: Implement
static void I2C_HandleRXNE(I2C_TypeDef* I2C)
{
  // FIXME: Implement
}

// FIXME: Implement
static void I2C_HandleNACK(I2C_TypeDef* I2C)
{
  // FIXME: Implement
}

static void I2C_HandleTC(I2C_TypeDef* I2C)
{
  // FIXME: This function has no way to indicate which transaction in the chain it is handling
  if (!(currentTransaction.chain))
    I2C->CR2 |= I2C_CR2_STOP;

  if (I2C_TransactionQueueEmpty) {
    if ((currentTransaction.chain))
      I2C_error = MISSING_CHAINED_TRANSACTION;

    I2C_ongoingTransaction = false;
    return;
  }

  I2C_ongoingTransaction = true;

  I2C_GetNextTransaction(I2C);
  I2C_Setup(I2C, &currentTransaction);

  I2C->CR2 |= I2C_CR2_START;
}

static void I2C_GetNextTransaction(I2C_TypeDef* I2C)
{
  if (I2C == I2C2) {
    // Copy I2C_nextTransaction struct and reset it
    currentTransaction = (I2C_Transaction){
      I2C_nextTransaction->nbytes,
      I2C_nextTransaction->address,
      I2C_nextTransaction->read,
      I2C_nextTransaction->chain,
      I2C_nextTransaction->message
    };

    I2C_nextTransaction = NULL;
  }
}
