#include "stm32f10x.h"
#include "fire_control.h"
#include "motor.h"
#include <stdio.h>

int main(void)
{
    motor_init();
    FireDetect_Init(); 

    motor_start();

    while(1)
    {
        FireDetect_Task();
    }
}
