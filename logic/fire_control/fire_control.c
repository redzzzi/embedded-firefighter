#include "fire_control.h"


static FireState_t fire_state = FIRE_IDLE;

// 부저 함수가 없다면 임시 정의 (나중에 buzzer.c 등으로 분리 필요)
void buzzer_on(void) {
    // GPIO_SetBits(GPIOB, GPIO_Pin_5); // 예시
    printf("buzzer_on\r\n");
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

void FireDetect_Init(void)
{
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
                Water_HandleFire();
            }
            break;

        case FIRE_DETECTED:
            buzzer_on();
            // pump_start();
            // bluetooth_send("FIRE DETECTED!\r\n");
            printf("FIRE DETECTED!\r\n");

            fire_state = FIRE_ALERT;
            break;

        case FIRE_ALERT: 
            Water_HandleFire();
            // 불이 꺼졌는지 확인하여 복귀하는 로직 추가
            if (!FireDetect_Check()) 
            {
                printf("FIRE CLEARED!\r\n");
                // buzzer_off(); 
                fire_state = FIRE_IDLE;
                motor_start(); // 모터 재가동
                Water_StopAll();
            }
            break;

        default:
            fire_state = FIRE_IDLE;
            break;
    }
}
