#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "misc.h"
#include <stdio.h>

void BT_Init(void) {
    // 1. 클럭 활성화 (GPIOD, AFIO, USART2)
    // 주의: PD5, PD6을 쓰려면 GPIOD와 AFIO 클럭이 필수입니다.
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    // 2. 핀 재매핑 (Remap)
    GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);

    // 3. GPIO 설정 (PD5: Tx, PD6: Rx)
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // Tx (PD5) - Alternate Function Push-Pull
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // Rx (PD6) - Input Floating
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // 4. USART2 설정 (9600bps, 8bit, No Parity, 1 Stop bit)
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART2, &USART_InitStructure);
    
    // 5. 수신 인터럽트 설정 (데이터 수신 시 필요할 경우 대비)
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    
    // 6. NVIC 설정 (인터럽트 우선순위)
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 7. USART2 활성화
    USART_Cmd(USART2, ENABLE);
}

// 문자열 전송 함수
void BT_SendString(char* str) {
    while (*str) {
        // USART2 전송 레지스터가 빌 때까지 대기
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
        USART_SendData(USART2, *str++);
    }
}

// (선택사항) 수신 인터럽트 핸들러
// stm32f10x_it.c 에 넣거나 여기서 처리
void USART2_IRQHandler(void) {
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        // 데이터를 받으면 단순히 읽어버림 (Echo 기능을 원하면 여기서 처리)
        uint16_t data = USART_ReceiveData(USART2);
        // 필요시 여기에 수신 처리 로직 추가
    }
}