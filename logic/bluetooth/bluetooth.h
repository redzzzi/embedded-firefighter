#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include "stm32f10x.h"

void BT_Init(void); 
void BT_SendString(char* str);

#endif