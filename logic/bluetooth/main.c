// main.c (블루투스 최소 동작 테스트 버전)

#include "stm32f10x.h"
#include "bluetooth.h"
#include "motor.h" 
#include "water_control.h" 
#include "fire_control.h" 
#include "misc.h" 

// 블루투스 로깅에 사용될 시스템 시간 변수는 주석 처리하거나 0으로 유지합니다.
// volatile uint32_t sys_time_ms = 0; 

// SysTick_Handler는 테스트를 위해 제외합니다.

void NVIC_Configure_BT(void) 
{
    NVIC_InitTypeDef NVIC_InitStructure;

    // 1. 우선순위 그룹 설정 (예: Group 4)
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); 

    // 2. USART1 인터럽트 활성화
    NVIC_EnableIRQ(USART1_IRQn); // 인터럽트 채널 활성화 [cite: 114, 256]
    
    // 3. NVIC 구조체 설정
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 예시 우선순위
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; // 예시 우선순위
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void System_Init(void)
{
    // 기본 시스템 클럭 설정 (72MHz)
    SystemInit(); 

    // --- [블루투스 통신 최소 초기화] ---
    BT_Init(); // 블루투스 (USART1) 초기화
    NVIC_Configure_BT();
    
    // 이외의 모든 프로젝트 모듈 초기화는 제외합니다.
    /*
    motor_init();       
    Water_Init();       
    FireDetect_Init();  
    */
    
    // --- [테스트 메시지 송신] ---
    // 시스템 시작과 동시에 이 메시지가 송신되어야 합니다.
    BT_SendString("<<< BLUETOOTH TX TEST SUCCESSFUL (NO FIRE LOGIC) >>>\r\n");
}


int main(void)
{
    // uint32_t last_report_time = 0; // 사용하지 않음

    System_Init();
    
    while (1)
    {
        // 무한 루프 내에서 어떠한 프로젝트 로직도 실행하지 않습니다.
        // BT_SendString이 정상 작동했다면, 이 시점에서 스마트폰에는 메시지가 출력되었어야 합니다.
    }
}