#ifndef WATER_CONTROL_H
#define WATER_CONTROL_H

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_adc.h"
#include "bluetooth.h"

// TODO: 핀 설정 (회로에 맞게 수정필욘)
// 수위센서: PC0 (ADC Channel 10)
// 펌프 릴레이: PB0
// 팬 모터: PB1

void Water_Init(void);              // 센서 및 모터 초기화
void Water_HandleFire(void);        // [핵심] 화재 발생 시 호출하는 함수
void Water_StopAll(void);           // 모든 장치 정지 (화재 진압 후)

#endif