#include "gyro.h"

#define I3G4250D_ADDR_WRITE 0xD6
#define I3G4250D_ADDR_READ 0xD7

#define I3G4250D_WHO_AM_I 0x0F
#define I3G4250D_CTRL_REG1 0x20
#define I3G4250D_OUT_X_L 0x28
#define I3G4250D_OUT_X_H 0x29

#define I3G4250D_OUT_Y_L 0x2A
#define I3G4250D_OUT_Y_H 0x2B


static void i2c_wait_idle(void) {
    while (I2C2->ISR & I2C_ISR_BUSY);
}

static void i2c_write_reg(uint8_t reg, uint8_t value) {
    i2c_wait_idle();

    I2C2->CR2 = (I3G4250D_ADDR_WRITE) |
                (2 << I2C_CR2_NBYTES_Pos) |
                I2C_CR2_START |
                I2C_CR2_AUTOEND;

    while (!(I2C2->ISR & I2C_ISR_TXIS));
    I2C2->TXDR = reg;

    while (!(I2C2->ISR & I2C_ISR_TXIS));
    I2C2->TXDR = value;

    while (!(I2C2->ISR & I2C_ISR_STOPF));
    I2C2->ICR |= I2C_ICR_STOPCF;
}

static uint8_t i2c_read_reg(uint8_t reg) {
    i2c_wait_idle();

    I2C2->CR2 = (I3G4250D_ADDR_WRITE) |
                (1 << I2C_CR2_NBYTES_Pos) |
                I2C_CR2_START;

    while (!(I2C2->ISR & I2C_ISR_TXIS));
    I2C2->TXDR = reg;
    while (!(I2C2->ISR & I2C_ISR_TC));

    I2C2->CR2 = (I3G4250D_ADDR_READ) |
                (1 << I2C_CR2_NBYTES_Pos) |
                I2C_CR2_START |
                I2C_CR2_AUTOEND |
                I2C_CR2_RD_WRN;

    while (!(I2C2->ISR & I2C_ISR_RXNE));
    uint8_t data = I2C2->RXDR;

    while (!(I2C2->ISR & I2C_ISR_STOPF));
    I2C2->ICR |= I2C_ICR_STOPCF;

    return data;
}

void gyro_init(void) {
    RCC->AHBENR  |= RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
    // PC0 general & High: Enable I2C mode
    GPIOC->MODER &= ~(3 << 0);
    GPIOC->MODER |= (1 << 0);
    GPIOC->OTYPER &= ~(1 << 0);
    GPIOC->ODR |= (1 << 0);
    // PB14 general & High: addr = 0xD6
    GPIOB->MODER &= ~(3 << 28);
    GPIOB->MODER |= (1 << 28);
    GPIOB->OTYPER &= ~(1 << 14);
    GPIOB->ODR |= (1 << 14);

    // PB13 = SCL, PB11 = SDA, open-drain
    GPIOB->MODER  &= ~((3 << 22) | (3 << 26));
    GPIOB->MODER  |=  ((2 << 22) | (2 << 26));
    GPIOB->OTYPER |=  (1 << 11) | (1 << 13);
    // PB11 to AF1, PB13 to AF5
    GPIOB->AFR[1] &= ~((0xF << 12) | (0xF << 20));
    GPIOB->AFR[1] |=  ((1 << 12) | (5 << 20));     // AF1 = I2C2

    I2C2->CR1 &= ~I2C_CR1_PE;
    I2C2->TIMINGR = 0x10420F13;
    I2C2->CR1 |= I2C_CR1_PE;

    for(volatile int i = 0; i < 100000; i++);

    i2c_write_reg(0x20, 0x0F);
}

int16_t gyro_readX(void) {
    uint8_t lo = i2c_read_reg(I3G4250D_OUT_X_L);
    uint8_t hi = i2c_read_reg(I3G4250D_OUT_X_H);
    return (int16_t)((hi << 8) | lo);
}

int16_t gyro_readY(void) {
    uint8_t lo = i2c_read_reg(I3G4250D_OUT_Y_L);
    uint8_t hi = i2c_read_reg(I3G4250D_OUT_Y_H);
    return (int16_t)((hi << 8) | lo);
}
