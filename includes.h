#ifndef INCLUDES_H
#define INCLUDES_H

// 이 주석을 풀면 로컬 테스트 모드, 주석 처리하면 STM32 모드
#define LOCAL_SIMULATION_MODE 

#ifdef LOCAL_SIMULATION_MODE
    // [로컬 PC 환경]
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "mock/driver_mock.h" // 지난번에 만든 가짜 드라이버
    
    // PC에는 없는 임베디드 타입 정의 (필요 시)
    #include <stdint.h>
    #include <stdbool.h>

#else
    // [STM32 하드웨어 환경]
    #include "stm32f10x.h"
    #include "stm32f10x_gpio.h"
    #include "stm32f10x_rcc.h"
    #include "stm32f10x_adc.h"
    #include "stm32f10x_dma.h"
    #include "stm32f10x_usart.h"
    
    // printf 사용을 막거나 UART로 리다이렉션 하기 위한 매크로 처리
    // 예: #define printf(...) 
#endif

// 공통으로 사용할 상수 정의 (제안서 기반)
#define WATER_FULL_LEVEL 3000
#define WATER_LOW_LEVEL  1000

#endif