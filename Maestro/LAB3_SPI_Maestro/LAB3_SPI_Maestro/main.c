/*
 * LAB 3 SPI - CÓDIGO DE MAESTRO
 * Created: 3/02/2026 21:29:10
 * Author : David Carranza y Mario Betancourt
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h> // Necesaria para atoi()
#include "UART.h"

#define CMD_GET1     0x11
#define CMD_GET2     0x22
#define CMD_SET_LEDS 0x33

// --- Variables para el procesamiento de UART ---
char rx_buffer[10];            // Buffer para guardar los dígitos (ej: "255")
volatile uint8_t rx_index = 0; // Índice del buffer
volatile uint8_t valor_final = 0;
volatile uint8_t listo_para_enviar = 0;

uint8_t SPI_Transfer(uint8_t data) {
	SPDR = data;
	while (!(SPSR & (1 << SPIF)));
	return SPDR;
}

int main(void) {
	UART_Init(UART_BAUD_9600_16MHZ, UART_INTERRUPTS_ENABLED);
	
	// SPI Maestro
	DDRB |= (1 << DDB3) | (1 << DDB5) | (1 << DDB2);
	PORTB |= (1 << PORTB2);
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);

	sei();
	char msg[50];

	while (1) {
		// --- TAREA NUEVA: ¿El usuario mandó un número por UART? ---
		if (listo_para_enviar) {
			PORTB &= ~(1 << PORTB2);
			SPI_Transfer(CMD_SET_LEDS); // Aviso al esclavo
			_delay_us(20);
			SPI_Transfer(valor_final);  // Enviamos el número convertido (0-255)
			PORTB |= (1 << PORTB2);
			
			listo_para_enviar = 0;      // Reset bandera
			
			// Confirmación opcional a la PC
			sprintf(msg, "Enviado al Esclavo: %d\r\n", valor_final);
			UART_sendString(msg);
		}

		// --- TAREA ANTERIOR: Pedir Potenciómetros (Se mantiene igual) ---
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

		sprintf(msg, "Pot1:%d Pot2:%d\r\n", p1, p2);
		UART_sendString(msg);

		_delay_ms(250);
	}
}

// --- Interrupción de UART: Captura y construye el número ---
ISR(USART_RX_vect) {
	char c = UDR0; // Leer carácter

	// Si es un número, guardarlo en el buffer
	if (c >= '0' && c <= '9') {
		if (rx_index < 5) { // Evitar desbordamiento
			rx_buffer[rx_index++] = c;
		}
	}
	// Si es fin de línea (Enter), procesar
	else if (c == '\r' || c == '\n') {
		if (rx_index > 0) {
			rx_buffer[rx_index] = '\0';      // Terminar la cadena de texto
			valor_final = (uint8_t)atoi(rx_buffer); // Convertir "123" -> 123
			listo_para_enviar = 1;           // Levantar bandera para el main
			rx_index = 0;                    // Reset buffer para el próximo número
		}
	}
}



/*********************************************************************************************************
FRECUENCIA DE RELOJ
**********************************************************************************************************/
// #ifndef F_CPU
// #define F_CPU 16000000UL // 16 MHz
// #endif
// 
// /*********************************************************************************************************
// LIBRERÍAS
// **********************************************************************************************************/
// #include <avr/io.h>
// #include <avr/interrupt.h>
// #include <stdio.h>
// #include <stdint.h>
// #include <util/delay.h>
// #include "SPI.h"
// #include "UART.h"
// 
// /*********************************************************************************************************
// ESTRUCTURAS DE DATOS Y DEFINICIONES
// **********************************************************************************************************/
// // Enumeración de comandos
// typedef enum
// {
// 	SPI_CMD_GET1	= 0X01,
// 	SPI_CMD_GET2	= 0x02,
// 	SPI_SEND_UART	= 0X03,
// 	SPI_SEND_DUMMY	= 0X04
// } COMMAND;
// 
// // Puertos y pines de esclavo
// #define SLAVE1_DDR	DDRB
// #define SLAVE1_DD	DDB0
// #define SLAVE1_PORT	PORTB
// #define SLAVE1_PIN	PINB0
// 
// // Byte dummy
// #define DUMMY 0x00
// 
// // Variables globales
// volatile uint8_t v1 = 0;
// volatile uint8_t v2 = 0;
// 
// // Dígitos de número enviado por UART
// volatile uint8_t uart_d3		= 0;
// volatile uint8_t uart_d2		= 0;
// volatile uint8_t uart_d1		= 0;
// volatile uint8_t uart_number	= 0;
// volatile uint8_t uart_value		= 0;
// 
// 
// /*********************************************************************************************************
//  SETUP
// **********************************************************************************************************/
// void setup()
// {
// 	// Deshabilitar interrupciones globales
// 	cli();
// 	
// 	// SPI INinit
// 	SPI_Init(MASTER_PRESCALER_2, MODE0_LE_SAMPLE_RISING, LSB_FIRST, SPI_INTERRUPTS_DISABLED);
// 	DDRB	|= (1 << PORTB0);	// Activar salida en pin de esclavo 1
// 	PORTB	|= (1 << PORTB0);	// Poner esclavo en idle
// 	
// 	// UART Init
// 	UART_Init(UART_BAUD_9600_16MHZ, UART_INTERRUPTS_ENABLED);
// 	
// 	// Inicialización de Timer 1 - 500 ms
// 	
// 	// Habilitar interrupciones globales
// 	sei();
// 	
// 	// Arrancar el proceso SPI
// 	SLAVE1_PORT &= ~(1 << SLAVE1_PIN);
// 	SPDR = SPI_CMD_GET1;
// }
// 
// /*********************************************************************************************************
//  MAINLOOP
// **********************************************************************************************************/
// int main(void)
// {
//     setup();
// 
// 	UART_sendString("\r\n");
// 	UART_sendString("----------------------------------------------------------------------------------------- \r\n");
// 	UART_sendString("| LABORATORIO 3 - COMUNICACIÓN SPI - MAESTRO                                             | \r\n");
// 	UART_sendString("----------------------------------------------------------------------------------------- \r\n");
// 	
// 	char mensaje[80]; // Buffer para construir la cadena
//    
//     /* Replace with your application code */
//     while (1) 
//     {
// 		// ENVÍO DE CADENA POR UART
// 		// Formateamos la cadena:
// 		// %03u indica un entero sin signo (uint8_t), de 3 dígitos, rellenado con '0'
// 		sprintf(mensaje, "| POTENCIOMETRO 1 : %03u | POTENCIOMETRO 2 = %03u | UART VALUE = %03u | NEXT_TX_BYTE = %03u | \r\n", v1, v2, uart_value);
// 		
// 		// Enviamos la cadena completa por UART
// 		UART_sendString(mensaje);
// 		
// 		// ENVÍO DE DATOS A ESCLAVO
// 		// Secuencia cíclica de queries
// 		v1 = SPI_Master_Query(&PORTB, PORTB0, SPI_CMD_GET1);
// 		v2 = SPI_Master_Query(&PORTB, PORTB0, SPI_CMD_GET2);
// 		SPI_Transmit(SPI_SEND_UART);
// 		SPI_Transmit(uart_number);
// 		
// 		// Un pequeño delay para no saturar la terminal
// 		_delay_ms(500);
//     }
// }
// 
// /*********************************************************************************************************
// RUTINAS DE INTERRUPCIÓN
// **********************************************************************************************************/
// // Recibir dato en UART
// ISR(USART_RX_vect)
// {
// 	uart_value = UDR0;
// }