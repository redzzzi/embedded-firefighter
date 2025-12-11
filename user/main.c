// main.c (통합 모듈)

#include "stm32f10x.h"
#include "bluetooth.h"
#include "water_control.h"
#include "fire_control.h"
#include "motor.h"
#include "misc.h" // NVIC 관련 함수 사용을 위해 포함

// 블루투스 로깅에 사용될 시스템 시간 (ms 단위)
// Log_Event 함수가 extern으로 참조하고 있음
volatile uint32_t sys_time_ms = 0; 
#define PERIODIC_REPORT_MS 5000 // 5초마다 수위 및 로그 전송

// 1ms SysTick 인터럽트 핸들러
void SysTick_Handler(void)
{
    sys_time_ms++;
}

void System_Init(void)
{
    // 기본 시스템 클럭 설정 (72MHz)
    SystemInit(); 

    // 1ms SysTick 초기화 (72MHz 클럭 기준)
    SysTick_Config(SystemCoreClock / 1000); 

    // 1. 모듈별 초기화
    BT_Init();          // 블루투스 (USART1) 초기화
    motor_init();       // 서보 모터 (TIM2) 초기화
    Water_Init();       // 수위 센서 (ADC/DMA), 펌프/팬 초기화
    FireDetect_Init();  // 불꽃 센서 (GPIO/EXTI 필요 시) 및 LED 초기화
    
    // 2. 초기 상태 설정
    motor_start(); // 감시 모드 시작 (서보모터 회전)

    // 3. 로깅 초기화
    Log_Event("System Booted Up. Monitoring Mode Active.");
    BT_SendString("System Ready. Waiting for Fire Detection...\r\n");
}


int main(void)
{
    uint32_t last_report_time = 0;

    System_Init();
    
    while (1)
    {
        // 1. 화재 감지 및 진압 로직 (FSM) 실행
        FireDetect_Task(); 
        
        // 2. 주기적인 상태 보고 (5초마다)
        if (sys_time_ms - last_report_time >= PERIODIC_REPORT_MS)
        {
            // 수위 정보 전송
            Water_SendLevel_BT();
            
            // 마지막 기록된 로그 재전송 (앱에서 로깅 확인용)
            Log_SendLastEvent_BT(); 
            
            last_report_time = sys_time_ms;
        }
        
        // 여기에 delay나 yield 함수를 추가하여 busy-waiting을 줄일 수 있습니다.
    }
}