#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "water_control.h"
#include "fire_control.h"
#include "bluetooth.h"
#include "motor.h"
#include <stdbool.h>
#include <stdio.h>


// PC13 LED 제어 매크로 (Blue Pill 보드 기준: 0=ON, 1=OFF)
#define LED_ON()  GPIO_ResetBits(GPIOC, GPIO_Pin_13)
#define LED_OFF() GPIO_SetBits(GPIOC, GPIO_Pin_13)

static FireState_t fire_state = FIRE_IDLE;

// 부저 대신 LED를 켜는 함수로 활용
void buzzer_on(void) {
    // GPIO_SetBits(GPIOB, GPIO_Pin_5); // 실제 부저 핀
    LED_ON();
}

void buzzer_off(void) {
    // GPIO_ResetBits(GPIOB, GPIO_Pin_5);
    LED_OFF();
}

void FlameSensor_GPIO_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = FLAME_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;   // input + pull-up
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(FLAME_GPIO_Port, &GPIO_InitStructure);
}

void LED_GPIO_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    LED_OFF(); // 초기화 시 끔
}

void FireDetect_Init(void)
{
    LED_GPIO_Init();        // LED 초기화 추가
    FlameSensor_GPIO_Init();
    fire_state = FIRE_IDLE;
}

static int FireDetect_Check(void)
{
    return (GPIO_ReadInputDataBit(FLAME_GPIO_Port, FLAME_Pin) == Bit_RESET);
}


// detect fire
void FireDetect_Task(void)
{   // if detect: warning & stop motor + bluetooth alert + start detect water depth
    switch (fire_state)
    {
        case FIRE_IDLE:
            if (FireDetect_Check())
            {
                fire_state = FIRE_DETECTED;
                motor_stop(); 
                // Water_HandleFire(); // FIRE_DETECTED에서 ALART로 넘어갈 때 처리
            }
            break;

        case FIRE_DETECTED:
            buzzer_on();
            // pump_start();
            BT_SendString("[ALERT] FIRE DETECTED! \r\n");  
            Log_Event("FIRE DETECTED"); // 로깅
            Water_HandleFire(); 
            fire_state = FIRE_ALERT;
            break;

        case FIRE_ALERT: 
            Water_HandleFire();
            // 불이 꺼졌는지 확인하여 복귀하는 로직 추가
            if (!FireDetect_Check()) 
            {
                BT_SendString("[INFO] Fire Cleared. Returning to Monitoring Mode.\r\n");
                Log_Event("FIRE CLEARED");
                // printf("FIRE CLEARED!\r\n");
                buzzer_off(); 
                motor_start(); // 모터 재가동
                Water_StopAll();
                fire_state = FIRE_IDLE;
            }
            break;

        default:
            fire_state = FIRE_IDLE;
            break;
    }
}
