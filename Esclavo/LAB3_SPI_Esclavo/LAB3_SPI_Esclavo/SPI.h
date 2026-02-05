/*
 * LIBRERÍA SPI (v3.0)
 * SPI.h
 *
 * Creada: 3/02/2026 21:29:29
 * Autor: Mario Alejandro Betancourt Franco
 */ 


#ifndef SPI_H_
#define SPI_H_

/*********************************************************************************************************
LIBRERÍAS
**********************************************************************************************************/
#include "avr/io.h"
#include "avr/interrupt.h"
#include <stdint.h>

/*********************************************************************************************************
ENUMERACIONES
**********************************************************************************************************/
typedef enum {
	// Usamos los bits SPR1 y SPR0, y el bit 2 como bandera para SPI2X
	MASTER_PRESCALER_4   = (1 << MSTR),
	MASTER_PRESCALER_16  = (1 << MSTR) | (1 << SPR0),
	MASTER_PRESCALER_64  = (1 << MSTR) | (1 << SPR1),
	MASTER_PRESCALER_128 = (1 << MSTR) | (1 << SPR1) | (1 << SPR0),
	MASTER_PRESCALER_2   = (1 << MSTR) | 0x04, // Flag manual para SPI2X
	MASTER_PRESCALER_8   = (1 << MSTR) | (1 << SPR0) | 0x04,
	MASTER_PRESCALER_32  = (1 << MSTR) | (1 << SPR1) | 0x04,
	SLAVE_SS             = 0x00
} SPI_CONFIG;

typedef enum
{
	LSB_FIRST = (1 << DORD),
	MSB_FIRST = 0
	
} SPI_DORD;

typedef enum
{
	MODE0_LE_SAMPLE_RISING	= 0,
	MODE1_LE_SETUP_RISING	= (1 << CPHA),
	MODE2_LE_SAMPLE_FALLING	= (1 << CPOL),
	MODE3_LE_SETUP_FALLING	= (1 << CPOL) | (1 << CPHA),
	
} SPI_CLOCK_MODE;

typedef enum
{
	SPI_INTERRUPTS_DISABLED = 0,
	SPI_INTERRUPTS_ENABLED	= 1	
} SPI_INTERRUPT_CONFIG;

/*********************************************************************************************************
DEFINICIONES
**********************************************************************************************************/
#define DDR_SPI  DDRB
#define DD_SS	 DDB2
#define DD_MOSI  DDB3
#define DD_MISO  DDB4
#define DD_SCK	 DDB5

/*********************************************************************************************************
DECLARACIONES DE FUNCIONES
**********************************************************************************************************/
void SPI_Init(SPI_CONFIG config, SPI_CLOCK_MODE clock_mode, SPI_DORD data_order, SPI_INTERRUPT_CONFIG en_interrupts);
void SPI_Transmit(uint8_t data);
uint8_t SPI_Recieve(uint8_t data);
uint8_t SPI_Master_Query(volatile uint8_t *slave_port, uint8_t slave_pin, uint8_t command);

#endif /* SPI_H_ */