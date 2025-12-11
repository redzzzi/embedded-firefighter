#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "bluetooth.h"
#include <string.h>
#include <stdio.h>


EventLog_t LastEvent;
extern uint32_t sys_time_ms;

void Log_Event(char* desc) {
    LastEvent.timestamp = sys_time_ms;
    strncpy(LastEvent.event_desc, desc, sizeof(LastEvent.event_desc) - 1);
    LastEvent.event_desc[sizeof(LastEvent.event_desc) - 1] = '\0';
}

void Log_SendLastEvent_BT(void) {
    char tx_buf[80];
    // 밀리초를 초 단위로 변환
    uint32_t seconds = LastEvent.timestamp / 1000;

    snprintf(tx_buf, sizeof(tx_buf), "[LOG] Time: %lu s, Event: %s\r\n", seconds, LastEvent.event_desc);
    BT_SendString(tx_buf);
}

void BT_Init(void)
{
    // 1. 클럭 활성화 (GPIOA, USART1)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    // 2. GPIO 설정 (Tx: PA9, Rx: PA10)
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // Tx (PA9)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Rx (PA10)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 3. USART1 설정 (9600-8-N-1)
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}

void BT_SendString(char* str)
{
    while (*str)
    {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, *str++);
    }
}