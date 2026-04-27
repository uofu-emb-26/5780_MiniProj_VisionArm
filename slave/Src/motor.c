#include <stdint.h>
#include "motor.h"
#include "stm32f072xb.h"
#include "SEGGER_RTT.h"

/* -------------------------------------------------------------------------------------------------------------
 *  Global Variable and Type Declarations
 *  -------------------------------------------------------------------------------------------------------------
 */

#define MAX(a, b)         (a > b ? a : b)
#define MIN(a, b)         (a > b ? b : a)
#define ABSOLUTE_VALUE(x) (x >= 0 ? x : -1 * x)

// Added named constants to replace hardcoded values for readability and easier tuning
#define POSITION_TOLERANCE 5
#define MAX_DUTY_CYCLE     90
#define OUTPUT_SHIFT       8
#define INTEGRAL_CLAMP     (100 * (1 << OUTPUT_SHIFT))
#define MOTOR_TOLERANCE     25        // The minimum tolerable angle difference from the target position

#define GYRO_DEADBAND 200   // Used to ignore noise thats within +- 200
#define GYRO_SCALE    16       // Scales the raw readings down for motor   // FIXME: Tune how fast tilt drives motor position

volatile int16_t error_integral = 0;    // Integrated error signal
volatile uint8_t duty_cycle = 0;    	// Output PWM duty cycle
volatile int16_t target_position = 0;    // Desired encoder position
volatile int16_t motor_position = 0;     // Current encoder position
volatile int16_t error = 0;              // Position error
volatile int8_t adc_value = 0;      	// ADC measured motor current
volatile uint8_t Kp = 5;            	// Proportional gain        // FIXME: Tune the feedback controller
volatile uint8_t Ki = 7;            	// Integral gain            // FIXME: Tune the feedback controller
volatile MOTOR_DIRECTION motor_dir = MOTOR_FORWARD;

static uint8_t buf0[1024];
static uint8_t buf1[1024];
static uint8_t buf2[1024];

union byte_split {
    uint32_t uword;
    int32_t word;
    uint8_t bytes[4];
};

void log_init(void) {
    SEGGER_RTT_ConfigUpBuffer(0, "", buf0, 1024, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
    SEGGER_RTT_ConfigUpBuffer(1, "", buf1, 1024, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
    SEGGER_RTT_ConfigUpBuffer(2, "", buf2, 1024, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
}

void log_data(void) {
    // Begin critical section
    __disable_irq();
    uint32_t duty_cycle_copy = duty_cycle;
    int32_t target_position_copy = target_position;
    int32_t motor_position_copy = motor_position;
    // End critical section
    __enable_irq();

    union byte_split data;
    data.uword = duty_cycle_copy;
    SEGGER_RTT_Write (0, &data.bytes, 4);
    data.word = target_position_copy;
    SEGGER_RTT_Write (1, &data.bytes, 4);
    data.word = motor_position_copy;
    SEGGER_RTT_Write (2, &data.bytes, 4);
}

// Sets up the entire motor drive system
void motor_init(void) {
    pwm_init();
    encoder_init();
    ADC_init();
    log_init();
}

// Sets up the PWM and direction signals to drive the H-Bridge
void pwm_init(void) {

    // Set up pin PA4 for H-bridge PWM output (TIMER 14 CH1)
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    GPIOA->MODER |= (1 << 9);
    GPIOA->MODER &= ~(1 << 8);

    // Set PA4 to AF4,
    GPIOA->AFR[0] &= 0xFFF0FFFF; // clear PA4 bits,
    GPIOA->AFR[0] |= (1 << 18);

    // Set up a PA5, PA6 as GPIO output pins for motor direction control
    GPIOA->MODER &= ~(GPIO_MODER_MODER5) | ~(GPIO_MODER_MODER8); // clear PA5, PA8 bits,
    GPIOA->MODER |= (GPIO_MODER_MODER5_0) | (GPIO_MODER_MODER8_0);

    //Initialize one direction pin to high, the other low
    motor_setDirection(MOTOR_FORWARD);

    // Set up PWM timer
    RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
    TIM14->CR1 = 0;                         // Clear control registers
    TIM14->CCMR1 = 0;                       // (prevents having to manually clear bits)
    TIM14->CCER = 0;

    // Set output-compare CH1 to PWM1 mode and enable CCR1 preload buffer
    TIM14->CCMR1 |= (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE);
    TIM14->CCER |= TIM_CCER_CC1E;           // Enable capture-compare channel 1
    TIM14->PSC = 1;                         // Run timer on 24Mhz
    TIM14->ARR = 1200;                      // PWM at 20kHz
    TIM14->CCR1 = 0;                        // Start PWM at 0% duty cycle

    TIM14->CR1 |= TIM_CR1_CEN;              // Enable timer
}

// Set the duty cycle of the PWM, accepts (0-100)
void pwm_setDutyCycle(uint8_t duty) {
    if(duty <= MAX_DUTY_CYCLE) {
        TIM14->CCR1 = ((uint32_t)duty*TIM14->ARR)/MAX_DUTY_CYCLE;  // Use linear transform to produce CCR1 value
        // (CCR1 == "pulse" parameter in PWM struct used by peripheral library)
    }

    duty_cycle = duty;

    // ADC debug read
    if(ADC1->ISR & ADC_ISR_EOC) {
        adc_value = ADC1->DR;
    }
}

// Sets up encoder interface to read motor speed
void encoder_init(void) {

    // Set up encoder input pins (TIMER 3 CH1 and CH2)
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

    GPIOB->MODER &= ~(GPIO_MODER_MODER4_0 | GPIO_MODER_MODER5_0);
    GPIOB->MODER |= (GPIO_MODER_MODER4_1 | GPIO_MODER_MODER5_1);
    GPIOB->AFR[0] |= ( (1 << 16) | (1 << 20) );

    // Set up encoder interface (TIM3 encoder input mode)
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    TIM3->CCMR1 = 0;
    TIM3->CCER = 0;
    TIM3->SMCR = 0;
    TIM3->CR1 = 0;

    TIM3->CCMR1 |= (TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0);   // TI1FP1 and TI2FP2 signals connected to CH1 and CH2
    TIM3->SMCR |= (TIM_SMCR_SMS_1 | TIM_SMCR_SMS_0);        // Capture encoder on both rising and falling edges
    TIM3->ARR = 0xFFFF;
    TIM3->CNT = 0x7FFF;
    // (Could also cast unsigned register to signed number to get negative numbers if it rotates backwards past zero
    //  just another option, the mid-bias is a bit simpler to understand though.)
    TIM3->CR1 |= TIM_CR1_CEN;                               // Enable timer

    // Configure a second timer (TIM6) to fire an ISR on update event
    // Used to periodically check and update speed variable
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;

    // Select PSC and ARR values that give an appropriate interrupt rate
    TIM6->PSC = 11;
    TIM6->ARR = 30000;

    TIM6->DIER |= TIM_DIER_UIE;             // Enable update event interrupt
    TIM6->CR1 |= TIM_CR1_CEN;               // Enable Timer

    NVIC_EnableIRQ(TIM6_DAC_IRQn);          // Enable interrupt in NVIC
    NVIC_SetPriority(TIM6_DAC_IRQn,2);
}

// Encoder interrupt to calculate motor position, also manages PI controller
void TIM6_DAC_IRQHandler(void) {
    // FIXME: This might be better as:
    // motor_position = (motor_position + (int16_t)(TIM3->CNT - 0x7FFF)) % 360;
    // to prevent spinning to an angle above 360 degrees

    motor_position += (int16_t)(TIM3->CNT - 0x7FFF) % 360;
    TIM3->CNT = 0x7FFF;

    PI_update();
    log_data();

    TIM6->SR &= ~TIM_SR_UIF;        // Acknowledge the interrupt
}

void ADC_init(void) {

    // Configure PA1 for ADC input (used for current monitoring)
    GPIOA->MODER |= (GPIO_MODER_MODER1_0 | GPIO_MODER_MODER1_1);

    // Configure ADC to 8-bit continuous-run mode, (asynchronous clock mode)
    RCC->APB2ENR |= RCC_APB2ENR_ADCEN;

    ADC1->CFGR1 = 0;                        // Default resolution is 12-bit (RES[1:0] = 00 --> 12-bit)
    ADC1->CFGR1 |= ADC_CFGR1_CONT;          // Set to continuous mode
    ADC1->CHSELR |= ADC_CHSELR_CHSEL1;      // Enable channel 1

    ADC1->CR = 0;
    ADC1->CR |= ADC_CR_ADCAL;               // Perform self calibration
    while(ADC1->CR & ADC_CR_ADCAL);         // Delay until calibration is complete

    ADC1->CR |= ADC_CR_ADEN;                // Enable ADC
    while(!(ADC1->ISR & ADC_ISR_ADRDY));    // Wait until ADC ready
    ADC1->CR |= ADC_CR_ADSTART;             // Signal conversion start
}

/*
 * Set motor rotation direction
 */
void motor_setDirection(MOTOR_DIRECTION dir) {
    switch (dir) {
        case MOTOR_FORWARD:
        GPIOA->ODR |= (1 << 5);
        GPIOA->ODR &= ~(1 << 8);

        motor_dir = MOTOR_FORWARD;
        break;
    default:
    case MOTOR_REVERSE:
        GPIOA->ODR &= ~(1 << 5);
        GPIOA->ODR |= (1 << 8);

        motor_dir = MOTOR_REVERSE;
        break;
    }
}

/* Run PI control loop
 *
 * Make sure to use the indicated variable names. This allows STMStudio to monitor
 * the condition of the system!
 *
 * target_rpm -> target motor speed in RPM
 * motor_speed -> raw motor speed in encoder counts
 * error -> error signal (difference between measured speed and target)
 * error_integral -> integrated error signal
 * Kp -> Proportional Gain
 * Ki -> Integral Gain
 * output -> raw output signal from PI controller
 * duty_cycle -> used to report the duty cycle of the system
 * adc_value -> raw /disaADC counts to report current
 *
 */
void PI_update(void) {
    __disable_irq();
    //calculate error signal and write to "error" variable

    int16_t output;

    // Calculate error between target and actual motor angle
    error = target_position - motor_position;

    if (target_position > motor_position)
        motor_setDirection(MOTOR_FORWARD);
    else
        motor_setDirection(MOTOR_REVERSE);

    // Check if the motor position is within tolerance
    if (ABSOLUTE_VALUE(error) < MOTOR_TOLERANCE) {
        pwm_setDutyCycle(0);

        __enable_irq();
        return;
    }
    else if (ABSOLUTE_VALUE(error) < 3 * MOTOR_TOLERANCE) {
        error_integral = 0;
    }

    // Prevent error_integral overflow
    if (error_integral >= 0) {
        error_integral = MIN(
                            (int32_t)(error_integral + Ki * error),
                            (int32_t)INTEGRAL_CLAMP
                        );
    }
    else {
        error_integral = MAX(
                            (int32_t)(error_integral + Ki * error),
                            -1 * (int32_t)INTEGRAL_CLAMP
                        );
    }

    // FIXME: Probably need to check for negative overflow here too
    // Calculate output PWM value and prevent overflow
    output = MIN((int32_t)(Kp * error + error_integral), (int32_t)INT16_MAX);
    output = ABSOLUTE_VALUE(output);

    // Scale down
    output = output >> OUTPUT_SHIFT;

    // Clamp to valid PWM range
    if(output > MAX_DUTY_CYCLE)
        output = MAX_DUTY_CYCLE;
    else if (output < 0)
        output = 0;

    pwm_setDutyCycle(output);
    __enable_irq();
}