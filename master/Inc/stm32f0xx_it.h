/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F0xx_IT_H
#define __STM32F0xx_IT_H

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
  uint8_t read;     // Set to true for a read transaction
  uint8_t chain;    // Set to true to chain another transaction after the current one
  char*   message;  // A pointer to the message to send or buffer to read into
} I2C_Transaction;

/**
  * @brief Defines the parameters of the next I2C transaction
  *
  * This pointer is set to `NULL` after it is used to define the current I2C
  * transaction. This pointer is safe to assign to when it is `NULL`.
  */
extern I2C_Transaction* I2C_nextTransaction;
extern uint8_t I2C_error;

/* Exported functions prototypes ---------------------------------------------*/
void NMI_Handler(void);
void HardFault_Handler(void);
void SVC_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void I2C2_IRQHandler(void);


#ifdef __cplusplus
}
#endif

#endif /* __STM32F0xx_IT_H */
