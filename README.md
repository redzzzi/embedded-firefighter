화재 감지 센서 pb0 \
수위 센서 pb1 \
양수 모터 pa5 \
팬 모듈 pa6 \
리셋 핀 pa7(팬 모듈이 인풋 제어 2개 받음, 두 개가 같을 때 안 돌아감) \
양수 모터, 팬 모듈은 핀 셋 하면 켜지고 리셋하면 꺼짐 GPIO_SetBits GPIO_ResetBits \ 
센서 값은 ADC_GetConversionValue(ADCx) ADC1은 화재, ADC2는 수위 센서 \
