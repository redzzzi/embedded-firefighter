#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include "bluetooth.h"
#include <stdio.h>

// 간단한 딜레이 함수
void Delay_Test(volatile uint32_t count) {
    while (count--);
}

int main(void)
{
    // 1. 시스템 및 블루투스 초기화
    SystemInit();
    BT_Init(); // 이 함수 안에서 USART2가 초기화됩니다.

    // 2. 시작 메시지 전송
    BT_SendString("\r\n[SYSTEM] Firefighter Robot Ready.\r\n");
    BT_SendString("[SYSTEM] Bluetooth: USART2 (PD5/Tx, PD6/Rx)\r\n");
    
    while(1)
    {
        // --- 테스트 1: 스마트폰에서 데이터가 오면 그대로 다시 보냄 (Echo) ---
        // ★ 수정: USART1 -> USART2 로 변경해야 함!
        if (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) != RESET)
        {
            // 1바이트 받기 (USART2에서)
            uint16_t receivedData = USART_ReceiveData(USART2);
            
            // 받은 데이터 다시 보내기 (Echo) (USART2로)
            USART_SendData(USART2, receivedData);
            
            // 보내는 동안 기다리기 (USART2 완료 대기)
            while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
        }
    }
}