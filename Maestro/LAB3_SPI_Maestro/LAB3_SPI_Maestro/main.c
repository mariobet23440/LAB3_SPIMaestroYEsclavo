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

// Variable para máquina de estados
volatile COMMAND spi_curr_cmd = SPI_SEND_DUMMY;	// Comando que se acaba de ejecutar
volatile COMMAND pending_cmd = SPI_SEND_DUMMY;
volatile uint8_t spi_tx_byte = SPI_CMD_GET1;


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

typedef enum
{
	POLLING,
	SEND_UART_COMMAND,
	SEND_UART_DATA	
} UART_SPI_STATE;
	
volatile UART_SPI_STATE uart_pending = 0;

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
	SPI_Init(MASTER_PRESCALER_2, MODE0_LE_SAMPLE_RISING, LSB_FIRST, SPI_INTERRUPTS_ENABLED);
	SLAVE1_DDR	|= (1 << SLAVE1_DD);	// Activar salida en pin de esclavo 1
	SLAVE1_PORT |= (1 << SLAVE1_PIN);	// Poner esclavo en idle
	
	// UART Init
	UART_Init(UART_BAUD_9600_16MHZ, UART_INTERRUPTS_ENABLED);
	
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
	
	UART_sendString("-------------------------------------------------------------------- \r\n");
	UART_sendString("|LABORATORIO 3 - COMUNICACIÓN SPI                                  | \r\n");
	UART_sendString("-------------------------------------------------------------------- \r\n");
	
	char mensaje[80]; // Buffer para construir la cadena
   
    /* Replace with your application code */
    while (1) 
    {
		// Formateamos la cadena:
		// %03u indica un entero sin signo (uint8_t), de 3 dígitos, rellenado con '0'
		sprintf(mensaje, "| POTENCIOMETRO 1 : %03u | POTENCIOMETRO 2 = %03u | UART VALUE = %03u | \r\n", v1, v2, uart_value);
		
		// Enviamos la cadena completa por UART
		UART_sendString(mensaje);
		
		// Un pequeño delay para no saturar la terminal
		_delay_ms(500);
    }
}

/*********************************************************************************************************
RUTINAS DE INTERRUPCIÓN
**********************************************************************************************************/
/*
OBSERVACIONES IMPORTANTES SOBRE SPI
Por cada comando enviado se recibe un byte SPDR. El intercambio de bytes ocurre en simultáneo.
Por lo tanto, es necesario tener información sobre el comando anterior (pending_cmd) y el comando actual.
*/
ISR(SPI_STC_vect)
{
	// =========================================================
	// 1. INPUT LOGIC  - Procesar byte recibido
	// =========================================================
	uint8_t rx = SPDR;

	if (pending_cmd == SPI_CMD_GET1)
	{
		v1 = rx;
		pending_cmd = SPI_SEND_DUMMY;
	}
	else if (pending_cmd == SPI_CMD_GET2)
	{
		v2 = rx;
		pending_cmd = SPI_SEND_DUMMY;
	}

	// =========================================================
	// 2. NEXT STATE LOGIC - Decidir siguiente estado
	// =========================================================

	// Variables locales para la salida
	uint8_t  next_tx_byte = DUMMY;
	COMMAND  next_spi_cmd = spi_curr_cmd;

	// --- Prioridad UART ---
	if (uart_pending == SEND_UART_COMMAND)
	{
		next_tx_byte = SPI_SEND_UART;
		uart_pending = SEND_UART_DATA;
		next_spi_cmd = SPI_SEND_UART;   // estado lógico
	}
	else if (uart_pending == SEND_UART_DATA)
	{
		next_tx_byte = uart_value;
		uart_pending = POLLING;
		next_spi_cmd = SPI_CMD_GET1;    // volver a polling
		spi_tx_byte  = SPI_CMD_GET1;
	}
	else
	{
		// --- Polling normal SPI ---
		switch (spi_curr_cmd)
		{
			case SPI_CMD_GET1:
			next_tx_byte = SPI_CMD_GET1;
			next_spi_cmd = SPI_SEND_DUMMY;
			break;

			case SPI_CMD_GET2:
			next_tx_byte = SPI_CMD_GET2;
			next_spi_cmd = SPI_SEND_DUMMY;
			break;

			case SPI_SEND_DUMMY:
			next_tx_byte = DUMMY;

			if (spi_tx_byte == SPI_CMD_GET1)
			{
				pending_cmd  = SPI_CMD_GET1;
				next_spi_cmd = SPI_CMD_GET2;
				spi_tx_byte  = SPI_CMD_GET2;
			}
			else
			{
				pending_cmd  = SPI_CMD_GET2;
				next_spi_cmd = SPI_CMD_GET1;
				spi_tx_byte  = SPI_CMD_GET1;
			}
			break;

			default:
			next_spi_cmd = SPI_CMD_GET1;
			next_tx_byte = SPI_CMD_GET1;
			break;
		}
	}

	// =========================================================
	// 3. OUTPUT LOGIC - Configurar salida
	// =========================================================
	spi_curr_cmd = next_spi_cmd;
	SPDR = next_tx_byte;
}


// Recibir dato en UART
ISR(USART_RX_vect)
{
	uart_value = UDR0;
	uart_pending = SEND_UART_COMMAND;   // ?? Arrancar secuencia UART por SPI
}