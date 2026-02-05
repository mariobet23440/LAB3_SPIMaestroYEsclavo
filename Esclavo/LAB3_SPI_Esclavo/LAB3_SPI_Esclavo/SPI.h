/*
 * LIBRERÍA SPI (v3.1)
 * SPI.h
 *
 * Creada: 3/02/2026
 * Autor: Mario Alejandro Betancourt Franco
 * Modificada: Ajuste de máscaras y prescalers
 */ 

#ifndef SPI_H_
#define SPI_H_

/*********************************************************************************************************
LIBRERÍAS
**********************************************************************************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

/*********************************************************************************************************
ENUMERACIONES
**********************************************************************************************************/
typedef enum
{
    // Formato de bits: [MSTR | X | X | X | X | SPI2X_FLAG | SPR1 | SPR0]
    MASTER_PRESCALER_4    = (1 << MSTR),
    MASTER_PRESCALER_16   = (1 << MSTR) | (1 << SPR0),
    MASTER_PRESCALER_64   = (1 << MSTR) | (1 << SPR1),
    MASTER_PRESCALER_128  = (1 << MSTR) | (1 << SPR1) | (1 << SPR0),
    MASTER_PRESCALER_2    = (1 << MSTR) | 0x04, // Flag manual para SPI2X
    MASTER_PRESCALER_8    = (1 << MSTR) | (1 << SPR0) | 0x04,
    MASTER_PRESCALER_32   = (1 << MSTR) | (1 << SPR1) | 0x04,
    SLAVE_SS              = 0x00
    
} SPI_CONFIG;

typedef enum
{
    MSB_FIRST = 0,             // Estándar AVR: DORD = 0
    LSB_FIRST = (1 << DORD)    // Estándar AVR: DORD = 1
    
} SPI_DORD;

typedef enum
{
    MODE0_LE_SAMPLE_RISING  = 0,
    MODE1_LE_SETUP_RISING   = (1 << CPHA),
    MODE2_LE_SAMPLE_FALLING = (1 << CPOL),
    MODE3_LE_SETUP_FALLING  = (1 << CPOL) | (1 << CPHA),
    
} SPI_CLOCK_MODE;

typedef enum
{
    SPI_INTERRUPTS_DISABLED = 0,
    SPI_INTERRUPTS_ENABLED  = 1 
} SPI_INTERRUPT_CONFIG;

/*********************************************************************************************************
DEFINICIONES DE PINES (ATMega328P - PORTB)
**********************************************************************************************************/
#define DDR_SPI  DDRB
#define DD_SS    DDB2  // Pin 10
#define DD_MOSI  DDB3  // Pin 11
#define DD_MISO  DDB4  // Pin 12
#define DD_SCK   DDB5  // Pin 13

/*********************************************************************************************************
DECLARACIONES DE FUNCIONES
**********************************************************************************************************/
void SPI_Init(SPI_CONFIG config, SPI_CLOCK_MODE clock_mode, SPI_DORD data_order, SPI_INTERRUPT_CONFIG en_interrupts);
void SPI_Transmit(uint8_t data);
uint8_t SPI_Recieve(uint8_t data);
uint8_t SPI_Master_Query(volatile uint8_t *slave_port, uint8_t slave_pin, uint8_t command);

#endif /* SPI_H_ */