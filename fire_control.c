// #include "bluetooth.h"
#include "fire_control.h"
#include "motor.h"
// #include <stdio.h>
// #include "water_control.h"

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
            // 평상시: 불꽃 감지 체크 + 모터 회전
            if (FireDetect_Check())
            {
                motor_stop(); // stop motor (현재 위치 유지)
                fire_state = FIRE_DETECTED;
            }
            else
            {
                motor_task(); // <--- 모터를 계속 왔다 갔다 하게 만듦
            }
            break;

        case FIRE_DETECTED:
            buzzer_on();
            // pump_start();
            // bluetooth_send("FIRE DETECTED!\r\n");
            // printf("FIRE DETECTED!\r\n");

            fire_state = FIRE_ALERT;
            break;

        case FIRE_ALERT: 
            // 불이 꺼졌는지 확인하여 복귀하는 로직 추가
            if (!FireDetect_Check()) 
            {
                // printf("FIRE CLEARED!\r\n");
                buzzer_off(); 
                motor_start(); // 모터 재가동 준비
                fire_state = FIRE_IDLE;
            }
            break;

        default:
            fire_state = FIRE_IDLE;
            break;
    }
}
