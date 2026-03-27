#include "main.h"
#include "stm32f0xx_hal.h"
#include "stm32f072xb.h"
#include "i2c_config.h"

void i2c_init(void){


    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();


    //Configure PB11 and PB13 to I2C SDA and SCL
    GPIOB->MODER &= ~((3 << 22) | (3 << 26));
    GPIOB->MODER |= ((1 << 23) | (1 << 27));
    GPIOB->OTYPER |= ((1 << 11) | (1 << 13));

    GPIOB->AFR[1] &= ~((GPIO_AFRH_AFSEL11) | (GPIO_AFRH_AFSEL13));
    GPIOB->AFR[1] |= ((1 << GPIO_AFRH_AFSEL11_Pos) | (5 << GPIO_AFRH_AFSEL13_Pos));


    //Configure PB14 and PC0 to output
    GPIOB->MODER &= ~(3 << 28);
    GPIOB->MODER |= (1 << 28);
    GPIOB->OTYPER &= ~(1 << 14);

    GPIOC->MODER &= ~(3 << 0);
    GPIOC->MODER |= (1 << 0);
    GPIOC->OTYPER &= ~(1 << 0);

    GPIOB->BSRR = (1 << 14);   // set PB14 High
    GPIOC->BSRR = (1 << 0);    // set PC0 High



    //Control Register Setup
    __HAL_RCC_I2C2_CLK_ENABLE();
    I2C2->CR1 &= ~ I2C_CR1_PE;

    I2C2->CR1 |= I2C_CR1_NACKIE;
    I2C2->CR1 |= I2C_CR1_TXIE;
    I2C2->CR1 |= I2C_CR1_RXIE;
    I2C2->CR1 |= I2C_CR1_TCIE;



    //TODO: Update with Macros
    I2C2->TIMINGR =
    (1  << 28) |
    (0x4 << 20) |
    (0x2 << 16) |
    (0xF  << 8) |
    (0x13 << 0);

    //Enable PE
    I2C2->CR1 |= I2C_CR1_PE;


}

void i2c_transaction_init(uint8_t slave_addr, uint8_t nbytes, uint8_t rw){

    //CLEAR the nbyte and SADD and read_write bit fields
    I2C2->CR2 &= ~((0xFF << 16) | (0x3FF << 0) | I2C_CR2_RD_WRN);
    I2C2->CR2 |= (slave_addr << 1);
    I2C2->CR2 |= (nbytes << 16);

    //If 1, then read
    if(rw){
        I2C2->CR2 |= (I2C_CR2_RD_WRN);
    }

    I2C2->CR2 |= (I2C_CR2_START);



}

void i2c_write_reg(uint8_t slave_addr, uint8_t addr, uint8_t value){

    i2c_transaction_init(slave_addr, 2, 0);

    // wait for TXIS or NACK
    while(!(I2C2->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF)));
    if(I2C2->ISR & I2C_ISR_NACKF){
        I2C2->ICR |= I2C_ICR_NACKCF;
        I2C2->CR2 |= I2C_CR2_STOP;
        return;
    }

    I2C2->TXDR = addr;

    // wait again for TXIS or NACK (for second byte)
    while(!(I2C2->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF)));
    if(I2C2->ISR & I2C_ISR_NACKF){
        I2C2->ICR |= I2C_ICR_NACKCF;
        I2C2->CR2 |= I2C_CR2_STOP;
        return;
    }
    I2C2->TXDR = value;

    // wait for complete, then STOP
    while(!(I2C2->ISR & I2C_ISR_TC));
    I2C2->CR2 |= I2C_CR2_STOP;
}



