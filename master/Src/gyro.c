#include "gyro.h"

#define I3G4250D_ADDR_WRITE 0xD6
#define I3G4250D_ADDR_READ 0xD7

#define I3G4250D_WHO_AM_I 0x0F
#define I3G4250D_CTRL_REG1 0x20
#define I3G4250D_OUT_X_L 0x28
#define I3G4250D_OUT_X_H 0x29

// I2C1: PB8 = SCL (AF1), PB9 = SDA (AF1)

static void i2c_wait_idle(void) {
    while (I2C1->ISR & I2C_ISR_BUSY);
}

static void i2c_write_reg(uint8_t reg, uint8_t value) {
    i2c_wait_idle();

    I2C1->CR2 = (I3G4250D_ADDR_WRITE) |
                (2 << I2C_CR2_NBYTES_Pos) |
                I2C_CR2_START |
                I2C_CR2_AUTOEND;

    while (!(I2C1->ISR & I2C_ISR_TXIS));
    I2C1->TXDR = reg;

    while (!(I2C1->ISR & I2C_ISR_TXIS));
    I2C1->TXDR = value;

    while (!(I2C1->ISR & I2C_ISR_STOPF));
    I2C1->ICR |= I2C_ICR_STOPCF;
}

static uint8_t i2c_read_reg(uint8_t reg) {
    i2c_wait_idle();

    I2C1->CR2 = (I3G4250D_ADDR_WRITE) |
                (1 << I2C_CR2_NBYTES_Pos) |
                I2C_CR2_START;

    while (!(I2C1->ISR & I2C_ISR_TXIS));
    I2C1->TXDR = reg;
    while (!(I2C1->ISR & I2C_ISR_TC));

    I2C1->CR2 = (I3G4250D_ADDR_READ) |
                (1 << I2C_CR2_NBYTES_Pos) |
                I2C_CR2_START |
                I2C_CR2_AUTOEND |
                I2C_CR2_RD_WRN;

    while (!(I2C1->ISR & I2C_ISR_RXNE));
    uint8_t data = I2C1->RXDR;

    while (!(I2C1->ISR & I2C_ISR_STOPF));
    I2C1->ICR |= I2C_ICR_STOPCF;

    return data;
}

void gyro_init(void) {
    RCC->AHBENR  |= RCC_AHBENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    // PB8 = SCL, PB9 = SDA, AF1, open-drain
    GPIOB->MODER  &= ~((3 << 16) | (3 << 18));
    GPIOB->MODER  |=  ((2 << 16) | (2 << 18));
    GPIOB->OTYPER |=  (1 << 8) | (1 << 9);
    // Enable I2C pull-up resistors
    GPIOB->PUPDR &= ~((3 << 16) | (3 << 18));
    GPIOB->PUPDR |= ((1 << 16) | (1 << 18));
    
    GPIOB->AFR[1] &= ~((0xF << 0) | (0xF << 4));
    GPIOB->AFR[1] |=  ((1 << 0) | (1 << 4));     // AF1 = I2C1

    I2C1->CR1 = 0;
    I2C1->TIMINGR = 0x10805E89;
    I2C1->CR1 |= I2C_CR1_PE;

    for(volatile int i = 0; i < 100000; i++);

    i2c_write_reg(I3G4250D_CTRL_REG1, 0x0F);
}

int16_t gyro_readX(void) {
    uint8_t lo = i2c_read_reg(I3G4250D_OUT_X_L);
    uint8_t hi = i2c_read_reg(I3G4250D_OUT_X_H);
    return (int16_t)((hi << 8) | lo);
}