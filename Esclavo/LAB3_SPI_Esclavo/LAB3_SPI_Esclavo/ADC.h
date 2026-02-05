/*
 * ADC.h
 *
 * Created: 22/01/2026 19:10:35
 *  Author: mario
 */ 


#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>
#include <stdint.h>

void ADC_Init(uint8_t int_en, uint8_t left_adj, uint8_t prescaler, uint8_t auto_trig_en, uint8_t auto_trig_src);
void ADC_SetChannel(uint8_t channel);
void ADC_StartConversion(void);
uint8_t ADC_Prescaler_FromFreq(uint32_t freq_CPU, uint32_t F_ADC_max);
uint16_t ADC_Read(void);
uint8_t ADC_GetChannel(void);

#endif /* ADC_H_ */