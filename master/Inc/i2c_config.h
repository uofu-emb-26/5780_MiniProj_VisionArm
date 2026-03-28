#include <stdint.h>
#include <stm32f0xx_hal.h>


void i2c_init(void);
void i2c_transaction_init(uint8_t slave_addr, uint8_t nbytes, uint8_t rw);
void i2c_write_reg(uint8_t slave_addr, uint8_t addr, uint8_t value);

