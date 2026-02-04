/*
 * Lab3SPI.c
 *
 * Created: 24/01/2026 12:07:07
 * Author : mario
 */ 

#define F_CPU 16000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdint.h>
#include <util/delay.h>
#include "ADC.h"
#include "SPI.h"
#include "UART.h"

/*-------------------------------------------------------------------
/ ESTRUCTURAS DE DATOS Y VARIABLES GLOBALES
/-------------------------------------------------------------------*/
typedef enum
{
	SPI_CMD_GET1	= 0X01,
	SPI_CMD_GET2	= 0x02,
	SPI_SEND_UART	= 0X03,
	SPI_SEND_DUMMY	= 0X04
} COMMAND;

volatile uint8_t *LED_ports[8] = { &PORTD, &PORTD, &PORTD, &PORTD,
								   &PORTD, &PORTD, &PORTB, &PORTB };

uint8_t LED_pins[8] = { PORTD2, PORTD3, PORTD4, PORTD5 ,
						PORTD6, PORTD7, PORTB0, PORTB1 };

volatile uint8_t *LED_ddrs[8] = { &DDRD, &DDRD, &DDRD, &DDRD,
								  &DDRD, &DDRD, &DDRB, &DDRB };
								  
#define CMD_GET_POT1 0X01
#define CMD_GET_POT2 0X02
#define CMD_SET_LEDS 0X03

volatile uint8_t adc_curr_channel = 0;
volatile uint8_t adc_chan1 = 0;
volatile uint8_t adc_chan2 = 1; 
volatile uint8_t pot1_value = 0;
volatile uint8_t pot2_value = 0;

volatile uint8_t uart_value = 5;

/*-------------------------------------------------------------------
/ PROTOTIPOS DE FUNCIONES
/-------------------------------------------------------------------*/
void InitPORT(void);
void WritePORT(uint8_t number);

/*-------------------------------------------------------------------
/ SETUP
/-------------------------------------------------------------------*/
void setup(void){
	cli();
	
	// Inicializar puerto de contadores
	InitPORT();
	
	// Inicializar ADC
	ADC_Init(1,0,0x07,0,0);
	ADC_SetChannel(0);
	ADC_StartConversion();
	
	// Inicializar SPI
	SPI_Init(SLAVE_SS, MODE0_LE_SAMPLE_RISING, LSB_FIRST, SPI_INTERRUPTS_ENABLED);
	
	UART_Init(UART_BAUD_9600_16MHZ, UART_INTERRUPTS_DISABLED);
	sei();
}

/*-------------------------------------------------------------------
/ MAINLOOP
/-------------------------------------------------------------------*/
int main(void){
	setup();
	
	UART_sendString("-------------------------------------------------------------------- \r\n");
	UART_sendString("| LABORATORIO 3 - COMUNICACIÓN SPI - ESCLAVO                       | \r\n");
	UART_sendString("-------------------------------------------------------------------- \r\n");
	
	char mensaje[80]; // Buffer para construir la cadena
	
    /* Replace with your application code */
    while (1) 
    {
		// Desplazamos 4 bits a la derecha para quedarnos con los 4 más significativos (0-15)
		uint8_t high_nibble = (pot1_value >> 4);
		uint8_t low_nibble  = (pot2_value >> 4);

		// Colocamos pot1 en la parte alta y pot2 en la baja
		uint8_t output = (high_nibble << 4) | (low_nibble & 0x0F);

		WritePORT(output);
		
		// Formateamos la cadena:
		// %03u indica un entero sin signo (uint8_t), de 3 dígitos, rellenado con '0'
		sprintf(mensaje, "| POTENCIOMETRO 1 : %03u | POTENCIOMETRO 2 = %03u | UART VALUE = %03u | \r\n", pot1_value, pot2_value, uart_value);
		
		// Enviamos la cadena completa por UART
		UART_sendString(mensaje);
		
		// Un pequeño delay para no saturar la terminal
		_delay_ms(1);
    }
}

/*-------------------------------------------------------------------
/ RUTINAS DE INTERRUPCIÓN
/-------------------------------------------------------------------*/
ISR(SPI_STC_vect)
{
	// Dato recibido
	uint8_t rx = SPDR;
	
	if(rx == SPI_CMD_GET1){
		SPDR = pot1_value;
	}
	else if(rx == SPI_CMD_GET2){
		SPDR = pot2_value;
	}
}

// Interrupción de ADC - Leer ADC y multiplexar canales
ISR(ADC_vect){
	uint16_t temp = ADC; 
	
	if(adc_curr_channel == adc_chan1){
		pot1_value = (uint8_t)(temp >> 2);
		adc_curr_channel = adc_chan2;
		ADC_SetChannel(adc_chan2);
		
	}
	else {
		pot2_value = (uint8_t)(temp >> 2);
		adc_curr_channel = adc_chan1;
		ADC_SetChannel(adc_chan1);
	}
	
	ADC_StartConversion();
}

/*-------------------------------------------------------------------
/ FUNCIONES AUXILIARES
/-------------------------------------------------------------------*/
// Inicializar LEDs de salida (Sin importar el orden)
void InitPORT(void)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		*LED_ddrs[i] |=  (1 << LED_pins[i]);
	}
}

// Mostrar un número en los LEDs
void WritePORT(uint8_t number)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		if (number & (1 << i)) *LED_ports[i] |=  (1 << LED_pins[i]);
		else *LED_ports[i] &= ~(1 << LED_pins[i]);
	}
}

