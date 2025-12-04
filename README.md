# Embedded Firefighter 🔥
## How to Test in VS Code


## Folder Structure Schema Example
```plaintext
/embedded-firefighter/
├── 1. Core_Application  (최상위 동작 제어 및 메인 로직)
│   ├── main.c           // 메인 루프, 전체 시스템 상태 천이(State Transition) 관리 (ex: 감시 -> 화재 대응 -> 복귀)
│   └── project_config.h // 전체 시스템에서 공유되는 상태 플래그, 전역 변수 (ex: DMA 수위값, 임계값), 매크로 정의
│
├── 2. Drivers           (MCU 주변 장치 저수준 초기화 및 드라이버)
│   ├── Rcc              // 클럭 제어 (RCC_AHBENR, RCC_APB2ENR 레지스터 등) [1, 2]
│   │   └── rcc.c, rcc.h
│   ├── Gpio             // 모든 GPIO 핀 설정 (센서, 모터, UART 핀 등) [3]
│   │   └── gpio.c, gpio.h
│   ├── Nvic_Exti        // 인터럽트 제어 및 불꽃 감지 센서 EXTI 초기화 [4, 5]
│   │   └── nvic_exti.c, nvic_exti.h
│   ├── Timer            // 타이머 모듈 초기화 (서보 PWM용 TIMx 설정) [6, 7]
│   │   └── timer.c, timer.h
│   ├── Adc_Dma          // 수위 센서를 위한 ADC1 및 DMA1 채널 1 설정 (12주차 미션 기반) [8, 9]
│   │   └── adc_dma.c, adc_dma.h
│   └── Uart             // 블루투스 통신을 위한 UART 모듈 초기화 (예: USART2/PA2, PA3 등) [10, 11]
│       └── uart.c, uart.h
│
├── 3. Modules           (고수준 기능 및 팀원 역할 분담 영역)
│   ├── M_Actuator       // 구동 장치 제어
│   │   ├── m_servo.c, m_servo.h           // 서보모터 동작 로직 및 PWM 듀티 설정
│   │   └── m_pump_fan.c, m_pump_fan.h     // 펌프/팬 ON/OFF 제어 및 릴레이 관리
│   ├── M_Sensor         // 센서 값 해석 및 상태 판단
│   │   ├── m_flame.c, m_flame.h           // 불꽃 감지 이벤트 처리 (인터럽트 응답)
│   │   └── m_water_level.c, m_water_level.h // DMA 수위 값 해석, 물 부족 임계값 판단 (사용자 역할)
│   └── M_Bluetooth      // 통신 및 보고 로직 (사용자 역할)
│       ├── m_bluetooth_comm.c, m_bluetooth_comm.h // 블루투스 모듈 초기 AT 명령 설정, 통신 프로토콜 구현
│       └── m_log.c, m_log.h               // 화재 기록 및 수위 부족 경고 로그를 MCU 메모리에 저장하고 블루투스로 전송하는 기능 구현 [12, 13]
│
├── 4. Utils_System      (디버깅 및 시스템 파일)
│   ├── Startup          // IAR/컴파일러 관련 시작 파일 (startup_stm32f10x.s)
│   ├── Linker           // 메모리 매핑 파일 (예: .icf 파일)
│   └── Debug_Print      // printf 재정의를 위한 파일 (디버깅 시에만 사용) [14]
│
└── 5. Docs_Assets       // 문서 및 결과물
    ├── Final_Report.pdf
    └── Demo_Video.mp4
```