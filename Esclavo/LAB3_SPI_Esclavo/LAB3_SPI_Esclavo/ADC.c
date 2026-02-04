/*
 * ADC.c
 *
 * Created: 22/01/2026 19:10:46
 *  Author: mario
 */ 

#include "ADC.h"

/*
* PARA INICIALIZAR ADC
* IMPORTANTE: Las interrupciones globales deben estar deshabilitadas y habilitarse después
*/

/*-------------------------------------------------------------
 Funciones Principales
--------------------------------------------------------------*/

// INICIALIZAR ADC
void ADC_Init(uint8_t int_en, uint8_t left_adj, uint8_t prescaler, uint8_t auto_trig_en, uint8_t auto_trig_src)
{
	// ADMUX - Voltaje de referencia y ajuste
	// Voltaje de referencia igual a VCC(AVCC)
	ADMUX &= ~((1 << REFS1) | (1 << REFS0));
	ADMUX |= (1 << REFS0);   // AVCC
	
	// left_adj : Ajustar bits de salida a la izquierda
	if(left_adj & 1) ADMUX |= (1 << ADLAR);
	else ADMUX &= ~(1 << ADLAR);
	
	// ADCSRA - Habilitación, prescaler, interrupciones
	// Habilitar ADC y Prescalers
	ADCSRA &= ~((1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0));
	ADCSRA |= (prescaler & 0x07);
	ADCSRA |= (1 << ADEN);
	
	// int_en : Habilitar interrupciones
	if(int_en & 1) ADCSRA |= (1 << ADIE);
	else ADCSRA &= ~(1 << ADIE);
	
	// ADCSRB - Autotrigger
	if(auto_trig_en & 1) {
		ADCSRA |= (1 << ADATE);
		ADCSRB &= ~0x07;
		ADCSRB |= (auto_trig_src & 0x07);
	}
	
	ADC_StartConversion();
}

// ESTABLECER CANAL EN ADC
void ADC_SetChannel(uint8_t channel){
	ADMUX &= 0xF0;				// Apagar últimos cuatro bits 
	ADMUX |= channel & 0x0F;	// Establecer canal
}

uint8_t ADC_GetChannel(void){
	uint8_t channel = ADMUX & 0X0F;
	return channel; 
}

// INICIAR CONVERSIÓN
void ADC_StartConversion(void){
	ADCSRA |= (1 << ADSC);
}


uint16_t ADC_Read(void){
	return ADC;
}

/*-------------------------------------------------------------
 Funciones Auxiliares
--------------------------------------------------------------*/
// Calculadora del prescaler (Valores de ADPS)
uint8_t ADC_Prescaler_FromFreq(uint32_t freq_CPU, uint32_t F_ADC_max)
{
	uint32_t div = freq_CPU / F_ADC_max;

	if(div <= 2)    return 0x01; // /2
	if(div <= 4)    return 0x02; // /4
	if(div <= 8)    return 0x03; // /8
	if(div <= 16)   return 0x04; // /16
	if(div <= 32)   return 0x05; // /32
	if(div <= 64)   return 0x06; // /64
	return 0x07;                // /128
}