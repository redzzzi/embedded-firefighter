#include "motor.h"

/* PWM 펄스 범위 (0.6ms ~ 2.4ms) */
#define SERVO_MIN  600
#define SERVO_MAX  2400
#define SERVO_STEP 20  // 한 번 호출 시 이동할 펄스 크기

static int16_t current_pulse = 1500;
static int8_t  move_direction = 1; // 1: 증가(시계), -1: 감소(반시계)

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

/* 초기 회전 시작 (중립 위치) */
void motor_start(void)
{
    // motor_set_speed(1500);
    // current_pulse = 1500;
    // move_direction = 1;
    // 현재 위치 유지하며 다시 시작
}

/* 모터 정지 (현재 위치 유지) */
void motor_stop(void)
{
    // PWM을 끄거나, 현재 각도 유지
    // 서보는 펄스를 계속 줘야 힘을 유지하므로 별도 조치 없음
    // 다만 움직이는 task를 멈추면 그 자리에서 멈춤
}

/* 주기적으로 호출되어 모터를 좌우로 흔드는 함수 */
void motor_task(void)
{
    // 속도 조절용 카운터 (main 루프 속도에 따라 값 조절 필요)
    static volatile uint32_t delay_cnt = 0;
    
    if (delay_cnt++ < 20000) return; // 적절한 딜레이 (너무 빠르면 모터가 못 따라감)
    delay_cnt = 0;

    // 각도 변경
    current_pulse += (move_direction * SERVO_STEP);

    // 범위 체크 및 방향 전환
    if (current_pulse >= SERVO_MAX)
    {
        current_pulse = SERVO_MAX;
        move_direction = -1; // 감소 방향으로 전환
    }
    else if (current_pulse <= SERVO_MIN)
    {
        current_pulse = SERVO_MIN;
        move_direction = 1;  // 증가 방향으로 전환
    }

    motor_set_speed(current_pulse);
}
