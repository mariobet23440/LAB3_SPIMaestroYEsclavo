/*-------------------------------------------------------------------
/ ELECTRÓNICA DIGITAL 2 - LABORATORIO 3 (MAESTRO)
/ Creado por: David Carranza y Mario Betancourt
/-------------------------------------------------------------------*/

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include "UART.h"

/*-------------------------------------------------------------------
/ CONFIGURACIÓN Y COMANDOS
/-------------------------------------------------------------------*/
#define CMD_GET1     0x11
#define CMD_GET2     0x22
#define CMD_SET_LEDS 0x33

/*-------------------------------------------------------------------
/ ESTRUCTURAS DE DATOS PARA LEDS (Igual al Esclavo)
/-------------------------------------------------------------------*/
volatile uint8_t *LED_ports[8] = { &PORTD, &PORTD, &PORTD, &PORTD,
&PORTD, &PORTD, &PORTB, &PORTB };

uint8_t LED_pins[8] = { PORTD2, PORTD3, PORTD4, PORTD5,
PORTD6, PORTD7, PORTB0, PORTB1 };

volatile uint8_t *LED_ddrs[8] =  { &DDRD, &DDRD, &DDRD, &DDRD,
&DDRD, &DDRD, &DDRB, &DDRB };

/*-------------------------------------------------------------------
/ VARIABLES GLOBALES
/-------------------------------------------------------------------*/
char rx_buffer[10];
volatile uint8_t rx_index = 0;
volatile uint8_t valor_final = 0;
volatile uint8_t listo_para_enviar = 0;

/*-------------------------------------------------------------------
/ PROTOTIPOS
/-------------------------------------------------------------------*/
void InitPORT(void);
void WritePORT(uint8_t number);
uint8_t SPI_Transfer(uint8_t data);

/*-------------------------------------------------------------------
/ MAINLOOP
/-------------------------------------------------------------------*/
int main(void) {
	// 1. Inicializar Hardware Local
	InitPORT();
	UART_Init(UART_BAUD_9600_16MHZ, UART_INTERRUPTS_ENABLED);
	
	//			  | P1 : 017 | P2 : 034 | Display : 012   |

	UART_sendString("\r\n");
	UART_sendString("| LABORATORIO 3 - ELECTRÓNICA DIGITAL 2 |\r\n");
	
	
	// 2. Configuración SPI Maestro
	DDRB |= (1 << DDB3) | (1 << DDB5) | (1 << DDB2); // MOSI, SCK, SS
	PORTB |= (1 << PORTB2); // SS Idle
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);

	sei();
	char msg[50];

	while (1) {
		// --- TAREA A: Enviar valor a los LEDs (Local y Remoto) ---
		if (listo_para_enviar) {
			// Actualizar LEDs locales en el Maestro
			WritePORT(valor_final);

			// Enviar al Esclavo por SPI
			PORTB &= ~(1 << PORTB2);
			SPI_Transfer(CMD_SET_LEDS);
			_delay_us(20);
			SPI_Transfer(valor_final);
			PORTB |= (1 << PORTB2);
			
			//sprintf(msg, "Enviado: %d\r\n", valor_final);
			//UART_sendString(msg);
			
			listo_para_enviar = 0;
		}

		// --- TAREA B: Pedir Potenciómetros ---
		uint8_t p1, p2;
		
		PORTB &= ~(1 << PORTB2);
		SPI_Transfer(CMD_GET1);
		_delay_us(20);
		p1 = SPI_Transfer(0xFF);
		PORTB |= (1 << PORTB2);

		_delay_ms(10);

		PORTB &= ~(1 << PORTB2);
		SPI_Transfer(CMD_GET2);
		_delay_us(20);
		p2 = SPI_Transfer(0xFF);
		PORTB |= (1 << PORTB2);

		// --- IMPRESIÓN EN FORMATO TABLA ---
		// %03d asegura los 3 dígitos (e.g., 001, 045, 255)
		sprintf(msg, "| P1 : %03d | P2 : %03d | Display : %03d |\r\n", p1, p2, valor_final);
		UART_sendString(msg);

		_delay_ms(250);
	}
}

/*-------------------------------------------------------------------
/ INTERRUPCIÓN UART CON TRUNCADO
/-------------------------------------------------------------------*/
ISR(USART_RX_vect) {
	char c = UDR0;

	if (c >= '0' && c <= '9') {
		if (rx_index < 5) rx_buffer[rx_index++] = c;
	}
	else if (c == '\r' || c == '\n') {
		if (rx_index > 0) {
			rx_buffer[rx_index] = '\0';
			int temp = atoi(rx_buffer); // Atoi convierte caracteres en un número entero
			
			// Truncado inteligente a 8 bits
			if (temp > 255) valor_final = 255;
			else if (temp < 0) valor_final = 0;
			else valor_final = (uint8_t)temp;

			listo_para_enviar = 1;
			rx_index = 0;
		}
	}
}

/*-------------------------------------------------------------------
/ FUNCIONES AUXILIARES (Idénticas al Esclavo)
/-------------------------------------------------------------------*/
uint8_t SPI_Transfer(uint8_t data) {
	SPDR = data;
	while (!(SPSR & (1 << SPIF)));
	return SPDR;
}

void InitPORT(void) {
	for (uint8_t i = 0; i < 8; i++) {
		*LED_ddrs[i] |= (1 << LED_pins[i]);
	}
}

void WritePORT(uint8_t number) {
	for (uint8_t i = 0; i < 8; i++) {
		if (number & (1 << i)) *LED_ports[i] |= (1 << LED_pins[i]);
		else *LED_ports[i] &= ~(1 << LED_pins[i]);
	}
}