#ifndef _PWM_H_
#define _PWM_H_

#include <stdint.h>

void PWM_Init(void);
void PWM_SetColor(uint8_t r, uint8_t g, uint8_t b);
void PWM_SetIntensity(uint8_t intensity);
void PWM_Update(void);

#endif /* _PWM_H_ */
