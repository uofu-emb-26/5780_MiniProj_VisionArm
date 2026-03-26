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
  NBYTES_INVALID
} I2C_ERRORS;

extern uint8_t I2C_read;
extern uint8_t I2C_address;
extern char I2C_message[100];
extern uint8_t I2C_nbytes;
extern uint8_t I2C_chain;
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
