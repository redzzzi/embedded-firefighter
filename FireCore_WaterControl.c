#include "stm32f10x.h"

#define WATER_THRESHOLD 2200

void GPIO_Config(void);
void ADC1_Config(void);
void TIM3_Config(void);
void NVIC_Config(void);
void USART1_Config(void);

uint16_t ADC_Read_WaterLevel(void) {
    ADC1->CR2 |= ADC_CR2_SWSTART;
    while(!(ADC1->SR & ADC_SR_EOC));
    return ADC1->DR;
}

void Pump_ON(void)  { GPIOC->BSRR = (1<<14); }
void Pump_OFF(void) { GPIOC->BRR  = (1<<14); }
void Fan_ON(void)   { GPIOB->BSRR = (1<<1);  }
void Fan_OFF(void)  { GPIOB->BRR  = (1<<1);  }

void Send_Bluetooth_Warning(char *msg) {
    while(*msg) {
        while(!(USART1->SR & USART_SR_TXE));
        USART1->DR = (*msg++);
    }
}

void Delay_ms(uint32_t ms) {
    for(uint32_t i=0; i<ms*7200; i++);
}

void TIM3_IRQHandler(void) {
    if(TIM3->SR & TIM_SR_UIF) {
        TIM3->SR &= ~TIM_SR_UIF;
        uint16_t level = ADC_Read_WaterLevel();
        if(level <= WATER_THRESHOLD) {
            Pump_OFF();
            Fan_ON();
            Send_Bluetooth_Warning("물 부족! 물을 채워주세요");
            Delay_ms(15000);
            Fan_OFF();
        }
        TIM3->CR1 &= ~TIM_CR1_CEN;
    }
}

void EXTI0_IRQHandler(void) {
    if(EXTI->PR & EXTI_PR_PR0) {
        EXTI->PR = EXTI_PR_PR0;
        uint16_t level = ADC_Read_WaterLevel();
        if(level > WATER_THRESHOLD) {
            Pump_ON();
            TIM3->CR1 |= TIM_CR1_CEN;
        } else {
            Pump_OFF();
            Fan_ON();
            Send_Bluetooth_Warning("물 부족! 물을 채워주세요");
            Delay_ms(15000);
            Fan_OFF();
        }
    }
}

int main(void) {
    GPIO_Config();
    ADC1_Config();
    TIM3_Config();
    NVIC_Config();
    USART1_Config();

    while(1) {
        // 
    }
}

void GPIO_Config(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN;
    GPIOA->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0);
    GPIOA->CRL |= GPIO_CRL_CNF0_0;
    GPIOC->CRH &= ~GPIO_CRH_MODE14;
    GPIOC->CRH |= GPIO_CRH_MODE14_1;
    GPIOC->BRR = GPIO_PIN_14;
    GPIOB->CRL &= ~GPIO_CRL_MODE1;
    GPIOB->CRL |= GPIO_CRL_MODE1_1;
    GPIOB->BRR = GPIO_PIN_1;
    GPIOB->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0);
    GPIOB->CRL |= GPIO_CRL_CNF0_1; // PB0 
}

void ADC1_Config(void) {
    RCC->CFGR |= RCC_CFGR_ADCPRE_DIV6;
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    ADC1->CR1 = 0;
    ADC1->CR2 = ADC_CR2_ADON;
    ADC1->SMPR2 = (5 << 0);
    ADC1->SQR3 = 0;
    ADC1->CR2 |= ADC_CR2_ADON;
    Delay_ms(1);
    ADC1->CR2 |= ADC_CR2_CAL;
    while(ADC1->CR2 & ADC_CR2_CAL);
}

void TIM3_Config(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    TIM3->PSC = 7199; // 72MHz / 7200 = 10kHz
    TIM3->ARR = 9999; // 10kHz * 10000 = 10s
    TIM3->DIER |= TIM_DIER_UIE;
}

void NVIC_Config(void) {
    NVIC->ISER[0] |= (1 << 14); // TIM3 IRQ
    NVIC->ISER[0] |= (1 << 6);  // EXTI0 IRQ
    EXTI->IMR |= EXTI_IMR_MR0;
    EXTI->RTSR |= EXTI_RTSR_TR0;
    AFIO->EXTICR[0] = 0x0001; // 
}

void USART1_Config(void) {
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN | RCC_APB2ENR_IOPAEN;
    GPIOA->CRH &= ~(GPIO_CRH_MODE9 | GPIO_CRH_CNF9);
    GPIOA->CRH |= GPIO_CRH_MODE9_1 | GPIO_CRH_CNF9_1; // PA9 TX 
    USART1->BRR = 0x271; // 9600bps @ 72MHz
    USART1->CR1 |= USART_CR1_TE | USART_CR1_UE;
}