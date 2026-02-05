/*
 * LAB 3 SPI - CÓDIGO DE MAESTRO
 * Created: 3/02/2026 21:29:10
 * Author : David Carranza y Mario Betancourt
 */ 

/*********************************************************************************************************
FRECUENCIA DE RELOJ
**********************************************************************************************************/
#ifndef F_CPU
#define F_CPU 16000000UL // 16 MHz
#endif

/*********************************************************************************************************
LIBRERÍAS
**********************************************************************************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdint.h>
#include <util/delay.h>
#include "SPI.h"
#include "UART.h"

/*********************************************************************************************************
ESTRUCTURAS DE DATOS Y DEFINICIONES
**********************************************************************************************************/
// Enumeración de comandos
typedef enum
{
	SPI_CMD_GET1	= 0X01,
	SPI_CMD_GET2	= 0x02,
	SPI_SEND_UART	= 0X03,
	SPI_SEND_DUMMY	= 0X04
} COMMAND;

// Puertos y pines de esclavo
#define SLAVE1_DDR	DDRB
#define SLAVE1_DD	DDB0
#define SLAVE1_PORT	PORTB
#define SLAVE1_PIN	PINB0

// Byte dummy
#define DUMMY 0x00

// Variables globales
volatile uint8_t v1 = 0;
volatile uint8_t v2 = 0;

// Dígitos de número enviado por UART
volatile uint8_t uart_d3		= 0;
volatile uint8_t uart_d2		= 0;
volatile uint8_t uart_d1		= 0;
volatile uint8_t uart_number	= 0;
volatile uint8_t uart_value		= 0;


/*********************************************************************************************************
 SETUP
**********************************************************************************************************/
void setup()
{
	// Deshabilitar interrupciones globales
	cli();
	
	// SPI INinit
	SPI_Init(MASTER_PRESCALER_2, MODE0_LE_SAMPLE_RISING, LSB_FIRST, SPI_INTERRUPTS_DISABLED);
	DDRB	|= (1 << PORTB0);	// Activar salida en pin de esclavo 1
	PORTB	|= (1 << PORTB0);	// Poner esclavo en idle
	
	// UART Init
	UART_Init(UART_BAUD_9600_16MHZ, UART_INTERRUPTS_ENABLED);
	
	// Inicialización de Timer 1 - 500 ms
	
	// Habilitar interrupciones globales
	sei();
	
	// Arrancar el proceso SPI
	SLAVE1_PORT &= ~(1 << SLAVE1_PIN);
	SPDR = SPI_CMD_GET1;
}

/*********************************************************************************************************
 MAINLOOP
**********************************************************************************************************/
int main(void)
{
    setup();

	UART_sendString("\r\n");
	UART_sendString("----------------------------------------------------------------------------------------- \r\n");
	UART_sendString("| LABORATORIO 3 - COMUNICACIÓN SPI - MAESTRO                                             | \r\n");
	UART_sendString("----------------------------------------------------------------------------------------- \r\n");
	
	char mensaje[80]; // Buffer para construir la cadena
   
    /* Replace with your application code */
    while (1) 
    {
		// ENVÍO DE CADENA POR UART
		// Formateamos la cadena:
		// %03u indica un entero sin signo (uint8_t), de 3 dígitos, rellenado con '0'
		sprintf(mensaje, "| POTENCIOMETRO 1 : %03u | POTENCIOMETRO 2 = %03u | UART VALUE = %03u | NEXT_TX_BYTE = %03u | \r\n", v1, v2, uart_value);
		
		// Enviamos la cadena completa por UART
		UART_sendString(mensaje);
		
		// ENVÍO DE DATOS A ESCLAVO
		// Secuencia cíclica de queries
		v1 = SPI_Master_Query(&PORTB, PORTB0, SPI_CMD_GET1);
		v2 = SPI_Master_Query(&PORTB, PORTB0, SPI_CMD_GET2);
		SPI_Transmit(SPI_SEND_UART);
		SPI_Transmit(uart_number);
		
		// Un pequeño delay para no saturar la terminal
		_delay_ms(500);
    }
}

/*********************************************************************************************************
RUTINAS DE INTERRUPCIÓN
**********************************************************************************************************/
// Recibir dato en UART
ISR(USART_RX_vect)
{
	uart_value = UDR0;
}