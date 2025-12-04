#ifndef __MOTOR_H__
#define __MOTOR_H__

#include "stm32f10x.h"
void motor_init(void);
void motor_start(void);
void motor_stop(void);
void motor_set_speed(uint16_t pulse);

#endif
