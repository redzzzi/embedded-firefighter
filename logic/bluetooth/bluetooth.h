#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include "stm32f10x.h"

typedef struct {
  // 이벤트 발생 시각 (ms 또는 초 단위 카운터)
  uint32_t timestamp;
  // 이벤트 설명
  char event_desc[40];
} EventLog_t;

// 가장 최근의 이벤트만 저장
extern EventLog_t LastEvent; 

void BT_Init(void); 
void BT_SendString(char* str);
// 이벤트 기록 함수
void Log_Event(char* desc);
void Log_SendLastEvent_BT(void);

#endif