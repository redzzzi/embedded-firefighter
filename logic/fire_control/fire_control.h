#ifndef __FIRE_DETECT_H__
#define __FIRE_DETECT_H__

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "water_control.h"
#include "bluetooth.h"
#include <stdbool.h>
#include <stdio.h>
#include "motor.h"

/* 불꽃 센서 핀 정의 (CubeMX에서 설정한 GPIO에 맞게 수정) */
#define FLAME_GPIO_Port      GPIOA
#define FLAME_Pin            GPIO_Pin_1

typedef enum {
    FIRE_IDLE = 0,      // 기본 상태(모터 회전)
    FIRE_DETECTED,      // 불꽃 감지됨
    FIRE_ALERT          // 경고 & 블루투스 송신, 워터레벨 탐색 시작
} FireState_t;

void FireDetect_Init(void);      // GPIO 초기 설정이 필요할 경우
void FireDetect_Task(void);      // 주기적으로 호출할 FSM 처리

#endif