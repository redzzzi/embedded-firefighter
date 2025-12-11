#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_adc.h"
#include "bluetooth.h"
#include "water_control.h"

#define WATER_THRESHOLD 1000  // 물 부족 판단 기준값 (0~4095)
#define PUMP_PIN GPIO_Pin_0   // PB0
#define FAN_PIN  GPIO_Pin_1   // PB1

volatile uint16_t adc_value[1]; // DMA 값 저장용

void Water_Init(void)
{
    // --- 1. GPIO & ADC & DMA 클럭 활성화 ---
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_ADC1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    // --- 2. 펌프/팬 GPIO 설정 (PB0, PB1 출력) ---
    GPIO_InitTypeDef GPIO_InitAct;
    GPIO_InitAct.GPIO_Pin = PUMP_PIN | FAN_PIN;
    GPIO_InitAct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitAct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitAct);
    Water_StopAll(); // 초기 상태 OFF

    // --- 3. 수위센서 GPIO 설정 (PC0 아날로그 입력) ---
    GPIO_InitTypeDef GPIO_InitSens;
    GPIO_InitSens.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitSens.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &GPIO_InitSens);

    // --- 4. DMA 설정 (ADC -> 메모리 자동전송) ---
    DMA_InitTypeDef DMA_InitStructure;
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)adc_value;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 1;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    DMA_Cmd(DMA1_Channel1, ENABLE);

    // --- 5. ADC 설정 ---
    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; // 연속 변환
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    // PC0 = ADC Channel 10 (데이터시트 확인 필요, 보통 PC0는 Ch10)
    ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_239Cycles5);
    ADC_DMACmd(ADC1, ENABLE);
    ADC_Cmd(ADC1, ENABLE);

    // 캘리브레이션
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));

    ADC_SoftwareStartConvCmd(ADC1, ENABLE); // 측정 시작
}

// [핵심 로직] 화재 감지 시 팀원이 이 함수만 호출하면 됨
void Water_HandleFire(void)
{
    // 물이 부족한가? (임계값 미만)
    if (adc_value[0] < WATER_THRESHOLD)
    {
        // [물 부족] -> 펌프 OFF, 팬 ON
        GPIO_ResetBits(GPIOB, PUMP_PIN);
        GPIO_SetBits(GPIOB, FAN_PIN);
        
        // 반복 전송 방지를 위해 상태 변경 시에만 보내는 로직을 추가하면 더 좋음
        BT_SendString("[WARN] No Water! Fan Mode ON.\r\n");
    }
    else
    {
        // [물 충분] -> 펌프 ON, 팬 OFF
        GPIO_SetBits(GPIOB, PUMP_PIN);
        GPIO_ResetBits(GPIOB, FAN_PIN);
    }
}

// 수위 센서 값(ADC)을 블루투스로 전송하는 함수
void Water_SendLevel_BT(void) {
    char tx_buf[50];
    uint16_t current_adc = adc_value[0]; // volatile 변수 복사
    
    // snprintf를 사용하여 문자열 형식화 (printf 사용 가능하도록 설정 필요)
    snprintf(tx_buf, sizeof(tx_buf), "[STATUS] Water Level: %u (ADC)\r\n", current_adc);
    BT_SendString(tx_buf);
    
    // 물 부족 시 추가 경고를 여기서도 보낼 수 있습니다.
    /*
? ? if (current_adc < WATER_THRESHOLD)
? ? {
? ? ? ? BT_SendString("[STATUS] WARNING: WATER LOW\r\n");
? ? }
    */
}

void Water_StopAll(void)
{
    GPIO_ResetBits(GPIOB, PUMP_PIN | FAN_PIN);
}