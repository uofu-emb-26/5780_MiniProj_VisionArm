#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

void Error_Handler(void);
void SetupLEDs(void);
void I2C_SlaveInit(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
