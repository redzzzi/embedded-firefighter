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
    BT_Init(); 

    // 2. 시작 메시지 전송
    BT_SendString("\r\n[TEST] Bluetooth Connected Successfully!\r\n");
    BT_SendString("[TEST] Type something on your phone...\r\n");

    while(1)
    {
        // --- 테스트 1: 스마트폰에서 데이터가 오면 그대로 다시 보냄 (Echo) ---
        if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)
        {
            // 1바이트 받기
            uint16_t receivedData = USART_ReceiveData(USART1);
            
            // 받은 데이터 다시 보내기 (Echo)
            USART_SendData(USART1, receivedData);
            
            // 보내는 동안 기다리기
            while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        }

        // --- 테스트 2: 2초마다 생존 신고 메시지 보내기 (옵션) ---
        /*
        static int counter = 0;
        counter++;
        if (counter > 2000000) { // 대략적인 딜레이
            BT_SendString("[ALIVE] System is running...\r\n");
            counter = 0;
        }
        */
    }
}