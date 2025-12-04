#include "motor.h"

void motor_init(void)
{
    /* GPIOA, TIM2 클럭 활성화 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    /* PA0 -> TIM2_CH1 PWM 출력 설정 */
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = GPIO_Pin_0;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    /* PWM 타이머 설정 (50Hz, 20ms 주기) */
    TIM_TimeBaseInitTypeDef tb;
    tb.TIM_Prescaler = 72 - 1;       // 72MHz / 72 = 1MHz = 1us 단위
    tb.TIM_Period = 20000 - 1;       // 20ms 주기 (서보 표준 PWM)
    tb.TIM_ClockDivision = 0;
    tb.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &tb);

    /* PWM 채널 설정 */
    TIM_OCInitTypeDef oc;
    oc.TIM_OCMode = TIM_OCMode_PWM1;
    oc.TIM_OutputState = TIM_OutputState_Enable;
    oc.TIM_OCPolarity = TIM_OCPolarity_High;
    oc.TIM_Pulse = 1500; // 정지 또는 중립값
    TIM_OC1Init(TIM2, &oc);

    TIM_Cmd(TIM2, ENABLE);          // 타이머 동작 시작
}

/* 서보 듀티 조절 */
void motor_set_speed(uint16_t pulse)
{
    TIM_SetCompare1(TIM2, pulse);   // 0~20000 범위 PWM 설정
}

/* 기본 회전 시작 */
void motor_start(void)
{
    motor_set_speed(1200);          // 360도 서보 회전 시작(예시값)
}

/* 모터 정지 */
void motor_stop(void)
{
    motor_set_speed(1500);          // 1500us = 중립(정지)
}
