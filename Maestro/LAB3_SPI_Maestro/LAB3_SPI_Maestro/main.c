/*
 * LAB 3 SPI - CÓDIGO DE MAESTRO
 * Created: 3/02/2026 21:29:10
 * Author : David Carranza y Mario Betancourt
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>    // Para sprintf
#include "UART.h"

// Definición de comandos
#define CMD_GET1 0x11
#define CMD_GET2 0x22

// Prototipo de intercambio SPI
uint8_t SPI_Transfer(uint8_t data);

int main(void) {
	// 1. INICIALIZAR UART (9600 bps)
	UART_Init(UART_BAUD_9600_16MHZ, UART_INTERRUPTS_DISABLED);
	
	// 2. INICIALIZAR SPI (Registros Puros)
	// MOSI (PB3), SCK (PB5) y SS (PB2) como salidas
	DDRB |= (1 << DDB3) | (1 << DDB5) | (1 << DDB2);
	// SS en alto (IDLE)
	PORTB |= (1 << PORTB2);
	
	// SPCR: Habilitar, Maestro, Fosc/128 (Seguridad máxima)
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);

	char msg_buffer[50];
	uint8_t pot1_val, pot2_val;

	UART_sendString("--- Sistema de Monitoreo Iniciado ---\r\n");

	while (1) {
		// --- OBTENER POT 1 ---
		PORTB &= ~(1 << PORTB2);     // SS LOW
		SPI_Transfer(CMD_GET1);      // Paso 1: Enviar comando
		_delay_us(10);               // Delay táctico para que el esclavo procese
		pot1_val = SPI_Transfer(0xFF); // Paso 2: Dummy byte para recibir pot1
		PORTB |= (1 << PORTB2);      // SS HIGH

		_delay_ms(5);                // Respiro para el esclavo

		// --- OBTENER POT 2 ---
		PORTB &= ~(1 << PORTB2);     // SS LOW
		SPI_Transfer(CMD_GET2);      // Paso 1: Enviar comando
		_delay_us(10);
		pot2_val = SPI_Transfer(0xFF); // Paso 2: Recibir pot2
		PORTB |= (1 << PORTB2);      // SS HIGH

		// --- ENVIAR A LA PC ---
		// Formateamos los valores en una cadena
		sprintf(msg_buffer, "Pot1: %d | Pot2: %d \r\n", pot1_val, pot2_val);
		UART_sendString(msg_buffer);

		_delay_ms(250); // Muestreo cada 250ms
	}
}

// Función auxiliar para simplificar el flujo
uint8_t SPI_Transfer(uint8_t data) {
	SPDR = data;
	while (!(SPSR & (1 << SPIF)));
	return SPDR;
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