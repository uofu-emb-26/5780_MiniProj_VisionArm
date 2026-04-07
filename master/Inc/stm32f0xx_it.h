/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F0xx_IT_H
#define __STM32F0xx_IT_H

#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

#define I2C_MAX_MESSAGE_LEN 100

typedef enum {
  BUS_ERROR         = 1,
  ARBITRATION_LOSS,
  OVERRUN_UNDERRUN,
  PEC_ERROR,
  TIMEOUT,
  SMBUS_ALERT,
  NBYTES_INVALID,
  NULL_NEXT_TRANSACTION
} I2C_Errors;

typedef struct {
  uint8_t nbytes;   // The number of bytes transmitted in this transaction
  uint8_t address;  // The device address for this transaction
  bool    read;     // Set to true for a read transaction
  bool    chain;    // Set to true to chain another transaction after the current one
  char*   message;  // A pointer to the message to send or buffer to read into
} I2C_Transaction;

/**
  * @brief Defines the parameters of the next I2C transaction
  *
  * This pointer is set to `NULL` after it is used to define the current I2C
  * transaction. This pointer is safe to assign to when it is `NULL`.
  */
extern I2C_Transaction* I2C_nextTransaction;
extern I2C_Errors I2C_error;
extern bool I2C_ongoingTransaction;   // Whether there is an ongoing transaction

/* Exported functions prototypes ---------------------------------------------*/
void NMI_Handler(void);
void HardFault_Handler(void);
void SVC_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void I2C2_IRQHandler(void);

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
void I2C_Setup(I2C_TypeDef* I2C, I2C_Transaction* transaction);
void I2C_SetNextTransaction(I2C_TypeDef* I2C, I2C_Transaction* transaction);


#ifdef __cplusplus
}
#endif

#endif /* __STM32F0xx_IT_H */
