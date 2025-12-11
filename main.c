#include "stm32f10x_adc.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "misc.h"
#include "motor.h"


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

#define THRESHOLD 3800

int main() {
	Init();
	motor_start();
	GPIO_ResetBits(GPIOD, GPIO_Pin_8);

    while (1) {
    	Water_Sensor_Value = ADC_GetConversionValue(ADC2);
    	switch (firestate)
    	    {
    	        case FIRE_IDLE:
    	            motor_task(); // <--- 모터를 계속 왔다 갔다 하게 만듦
    	            break;

    	        case FIRE_DETECTED:
    	        	Water_Sensor_Value = ADC_GetConversionValue(ADC2);
    	        	GPIO_SetBits(GPIOD, GPIO_Pin_8);
    	        	Pol_Delay_us(200000);
    	        	GPIO_ResetBits(GPIOD, GPIO_Pin_8);
    	        	firestate = FIRE_IDLE;
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
    NvicInit();
    motor_init();
}

void RccInit(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
}

void GpioInit(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	//fire sensor
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;	//water level sensor
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitTypeDef water_motor; //water motor relay module pin pd8
    water_motor.GPIO_Pin = GPIO_Pin_8;
    water_motor.GPIO_Mode = GPIO_Mode_Out_PP;
    water_motor.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &water_motor);

    GPIO_InitTypeDef fan_motor; //fan motor module pin pd9
    fan_motor.GPIO_Pin = GPIO_Pin_9;
    fan_motor.GPIO_Mode = GPIO_Mode_Out_PP;
    fan_motor.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &fan_motor);
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

    NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


/*
 * ISR
 */

void ADC1_2_IRQHandler() {
    if (ADC_GetITStatus(ADC1, ADC_IT_EOC) != RESET) {
        Fire_Sensor_Value = ADC_GetConversionValue(ADC1);
        //--- Clear ADC1 AWD pending interrupt bit
        if(Fire_Sensor_Value <= THRESHOLD){
        	firestate = FIRE_DETECTED;
        }
        //else firestate = FIRE_IDLE;
        ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
    }
}


