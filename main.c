#include "stm32f10x.h"
#include "fire_control.h"
#include "water_control.h"
#include "motor.h"
#include "bluetooth.h"
#include <stdio.h>

int main(void)
{
    motor_init();
    FireDetect_Init(); 
    Water_Init();
    BT_Init();

    BT_SendString("System Ready.\r\n");
    motor_start();

    while(1)
    {
        FireDetect_Task();
    }
}
