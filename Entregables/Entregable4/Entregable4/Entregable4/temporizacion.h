#ifndef _TEMPORIZACION_H_
#define _TEMPORIZACION_H_

#include <stdint.h>

void Temporizacion_Init(void);
uint32_t Temporizacion_GetMillis(void);
void Temporizacion_AnimacionUpdate(uint32_t periodo_T);

#endif /* _TEMPORIZACION_H_ */
