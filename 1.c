#include "stm32f10x.h"

#define WATER_THRESHOLD   2200      // 수위 임계값(12비트 ADC, 실제 물탱크 테스트 후 수정)

uint16_t ADC_Read_WaterLevel(void) {
    ADC1->CR2 |= ADC_CR2_SWSTART;              // 소프트웨어 변환 시작
    while(!(ADC1->SR & ADC_SR_EOC));           // 변환 완료 대기
    return (uint16_t)ADC1->DR;
}

void Pump_ON(void)  { GPIOC->BSRR = (1<<14); }
void Pump_OFF(void) { GPIOC->BRR  = (1<<14); }
void Fan_ON(void)   { GPIOB->BSRR = (1<<1);  }
void Fan_OFF(void)  { GPIOB->BRR  = (1<<1);  }

void Delay_ms(uint32_t ms) {
    volatile uint32_t n = ms * 7200;
    while(n--);
}

void Water_Spray_Logic(void) {
    uint16_t level = ADC_Read_WaterLevel();
    
    if(level > WATER_THRESHOLD) {                // 물 충분
        Pump_ON();
        for(int i = 0; i < 10; i++) {
            Delay_ms(1000);
            if(ADC_Read_WaterLevel() <= WATER_THRESHOLD) {
                Pump_OFF();
                Fan_ON();
                Delay_ms(15000);
                Fan_OFF();
                return;
            }
        }
        Pump_OFF();
    } else {                                     // 물 부족
        Pump_OFF();
        Fan_ON();
        Delay_ms(15000);
        Fan_OFF();
    }
}

int main(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | 
                     RCC_APB2ENR_IOPCEN | RCC_APB2ENR_ADC1EN;
    RCC->AHBENR  |= RCC_AHBENR_DMA1EN;
    RCC->CFGR     |= RCC_CFGR_ADCPRE_DIV6;         // ADC 클럭 12MHz

    GPIOA->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0);
    GPIOA->CRL |= GPIO_CRL_CNF0_0;                  // PA0 아날로그 입력

    GPIOC->CRH &= ~GPIO_CRH_MODE14;
    GPIOC->CRH |= GPIO_CRH_MODE14_1;                // PC14 출력 (펌프)
    GPIOC->BRR = GPIO_PIN_14;

    GPIOB->CRL &= ~GPIO_CRL_MODE1;
    GPIOB->CRL |= GPIO_CRL_MODE1_1;
    GPIOB->BRR = GPIO_PIN_1;

    ADC1->CR1 = 0;
    ADC1->CR2 = ADC_CR2_ADON;
    ADC1->SMPR2 = (5 << 0);                         // 채널0 샘플링 55.5사이클
    ADC1->SQR3 = 0;
    ADC1->CR2 |= ADC_CR2_ADON;
    Delay_ms(1);
    ADC1->CR2 |= ADC_CR2_CAL;
    while(ADC1->CR2 & ADC_CR2_CAL);

    while(1) {
        // 불꽃 감지 EXTI 인터럽트에서 Water_Spray_Logic() 호출
    }
}