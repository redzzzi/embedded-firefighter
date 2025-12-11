#include "stm32f10x_adc.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "misc.h"
#include "motor.h"
#include "bluetooth.h"
#include "fire_control.h"

void Init(void);
void RccInit(void);
void GpioInit(void);
void AdcInit(void);
void NvicInit(void);

void Pol_Delay_us(volatile uint32_t microseconds){	//NOT ACCURATE JUST FOR TESTING
    /* Go to number of cycles for system */
    microseconds *= (SystemCoreClock / 1000000);
    /* Delay till end */
    while (microseconds--);
}

typedef enum {
    FIRE_IDLE = 0,      // 기본 상태(모터 회전)
    FIRE_DETECTED,      // 불꽃 감지됨
    FIRE_ALERT          // 경고 & 블루투스 송신, 워터레벨 탐색 시작
} FireState_t;

FireState_t firestate;

volatile uint16_t Fire_Sensor_Value = 0;
volatile uint16_t Water_Sensor_Value = 0;

#define FIRE_THRESHOLD 3500
#define WATER_THRESHOLD 2000
int main() {
	Init();
	motor_start();
	GPIO_ResetBits(GPIOA, GPIO_Pin_5);
	GPIO_ResetBits(GPIOA, GPIO_Pin_7);
	GPIO_ResetBits(GPIOA, GPIO_Pin_6);

    while (1) {
    	Water_Sensor_Value = ADC_GetConversionValue(ADC2);
    	switch (firestate)
    	    {
    	        case FIRE_IDLE:
    	            motor_task(); // <--- 모터를 계속 왔다 갔다 하게 만듦
    	            break;

    	        case FIRE_DETECTED:
    	        	Water_Sensor_Value = ADC_GetConversionValue(ADC2);
    	        	if(Water_Sensor_Value > WATER_THRESHOLD){
						GPIO_SetBits(GPIOA, GPIO_Pin_5);
						Pol_Delay_us(200000);
						GPIO_ResetBits(GPIOA, GPIO_Pin_5);
						firestate = FIRE_IDLE;
    	        	}
    	        	else{
    	        		GPIO_SetBits(GPIOA, GPIO_Pin_6);
						Pol_Delay_us(200000);
						GPIO_ResetBits(GPIOA, GPIO_Pin_6);
						firestate = FIRE_IDLE;
    	        	}
    	            break;

    	        default:
    	            break;
    	    }
    }
}

void Init(void) {
    SystemInit();
    RccInit();
    GpioInit();
    AdcInit();
    Adc2Init();
    BT_Init(); // BT
    NvicInit();
    motor_init();
}

void RccInit(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
}

void GpioInit(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	//fire sensor pb0
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;	//water level sensor pb1
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

	//water motor module pin pa5
    GPIO_InitTypeDef water_module;
    water_module.GPIO_Pin = GPIO_Pin_5;
    water_module.GPIO_Mode = GPIO_Mode_Out_PP;
    water_module.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &water_module);

	GPIO_InitTypeDef fan_module;
	fan_module.GPIO_Pin = GPIO_Pin_6;
	fan_module.GPIO_Mode = GPIO_Mode_Out_PP;
	fan_module.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &fan_module);


	GPIO_InitTypeDef reset_pin;
	reset_pin.GPIO_Pin = GPIO_Pin_7;
	reset_pin.GPIO_Mode = GPIO_Mode_Out_PP;
	reset_pin.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &reset_pin);
}

void AdcInit(void) {
    ADC_InitTypeDef ADC_InitStructure;

    // Todo: ADC1 Configuration
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;  //adc에서 각 블럭을 독립적으로 사용할때
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;       //단일 채널 스캔 설정
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;  //한 번의 트리거 신호로 하나의 채널에서 ADC 샘플링 지속
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //외부 트리거 없음
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;      //데이터 정렬 왼쪽
    ADC_InitStructure.ADC_NbrOfChannel = 1;                     //사용할 채널 개수
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_239Cycles5);// 채널8 ->PB0
    ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE); // interrupt enable
    ADC_Cmd(ADC1, ENABLE); // ADC1 enable
    ADC_ResetCalibration(ADC1);

    while (ADC_GetResetCalibrationStatus(ADC1));

    ADC_StartCalibration(ADC1);

    while (ADC_GetCalibrationStatus(ADC1));

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void Adc2Init(void) {
	ADC_InitTypeDef ADC_InitStructure;

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;  //adc에서 각 블럭을 독립적으로 사용할때
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;       //단일 채널 스캔 설정
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;  //한 번의 트리거 신호로 하나의 채널에서 ADC 샘플링 지속
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //외부 트리거 없음
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;      //데이터 정렬 왼쪽
	ADC_InitStructure.ADC_NbrOfChannel = 1;                     //사용할 채널 개수
	ADC_Init(ADC2, &ADC_InitStructure);

	ADC_RegularChannelConfig(ADC2, ADC_Channel_9, 1, ADC_SampleTime_239Cycles5);// 채널9 ->PB1
	ADC_Cmd(ADC2, ENABLE); // ADC1 enable
	ADC_ResetCalibration(ADC2);

	while (ADC_GetResetCalibrationStatus(ADC2));

	ADC_StartCalibration(ADC2);

	while (ADC_GetCalibrationStatus(ADC2));

	ADC_SoftwareStartConvCmd(ADC2, ENABLE);

}

void NvicInit(void) {
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); // BT
    NVIC_EnableIRQ(USART1_IRQn); // BT

    NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


/*
 * ISR
 */

/* ISR */
void ADC1_2_IRQHandler() {
    static uint16_t noise_filter_count = 0; // 카운트 변수 (static 필수)

    if (ADC_GetITStatus(ADC1, ADC_IT_EOC) != RESET) {
        Fire_Sensor_Value = ADC_GetConversionValue(ADC1);

        // 1. 값이 튀었을 때 (불꽃 감지 신호)
        if (Fire_Sensor_Value <= FIRE_THRESHOLD) {
            noise_filter_count++; // 카운트를 셉니다.

            // 2. 연속으로 500번 이상 감지되어야 진짜 불로 인정
            // (ADC는 1초에 수십만 번 측정하므로 500번도 아주 짧은 시간입니다)
            if (noise_filter_count > 500) {
                firestate = FIRE_DETECTED;
                // fire_check_count = 0; // 상태가 바뀌었으니 초기화하지 않고 유지해도 됨
            }
        }
        else {
            // 3. 중간에 값이 다시 정상으로 돌아오면(노이즈였다면) 카운트 초기화
            noise_filter_count = 0;
        }

        ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
    }
}

