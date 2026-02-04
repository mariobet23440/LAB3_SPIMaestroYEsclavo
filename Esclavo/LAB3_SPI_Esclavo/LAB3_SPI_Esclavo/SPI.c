/*
 * LIBRERÍA SPI (v3.0)
 * SPI.c
 *
 * Creada: 3/02/2026 21:29:29
 * Autor: Mario Alejandro Betancourt Franco
 * Updates : Generalización de función de inicialización
 */ 

#include "SPI.h"

/*********************************************************************************************************
FUNCIÓN DE INICIALIZACIÓN SPI
**********************************************************************************************************/
void SPI_Init(SPI_CONFIG config, SPI_CLOCK_MODE clock_mode, SPI_DORD data_order, SPI_INTERRUPT_CONFIG en_interrupts)
{
	// CONFIGURACIÓN EN REGISTRO DE CONTROL Y ESTATUS
	// Habilitar SPI
	SPCR = (1 << SPE);
	
	// Habilitar interrupciones
	if(en_interrupts == SPI_INTERRUPTS_ENABLED) SPCR |= (1 << SPIE);	
	
	// Aplicar máscara sobre config (Descartar MSB de presets)
	uint8_t temp = config & ((1 << MSTR) | (1 << SPR1) | (1 << SPR0)); 
	SPCR |= (config | clock_mode | data_order);
	
	// Prescalers de velocidad doble
	switch(config)
	{
		case MASTER_PRESCALER_4:	break;
		case MASTER_PRESCALER_16:	break;
		case MASTER_PRESCALER_128:	break;
		case MASTER_PRESCALER_64:	SPSR |= (1 << SPI2X); break;
		case MASTER_PRESCALER_2:	SPSR |= (1 << SPI2X); break;
		case MASTER_PRESCALER_8:	SPSR |= (1 << SPI2X); break;
		case MASTER_PRESCALER_32:	SPSR |= (1 << SPI2X); break;
		case SLAVE_SS:				break;
	}
	
	// INICIALIZACIÓN DE PUERTOS
	// En modo maestro MOSI y SCK son salidas
	if(temp & (1 << MSTR)) DDR_SPI	 =  (1 << DD_MOSI) | (1 << DD_SCK);
	// En modo esclavo MISO es salidas
	else DDR_SPI = (1 << DD_MISO);
}

/*********************************************************************************************************
TRANSMITIR DATO
**********************************************************************************************************/
void SPI_Transmit(uint8_t data)
{
	// Iniciar transmisión
	SPDR = data;
	// Esperar a que la transmisión se complete
	while(!(SPSR & (1 << SPIF)));
}

/*********************************************************************************************************
RECIBIR DATO
**********************************************************************************************************/
uint8_t SPI_Recieve(uint8_t data)
{
	// Esperar a que la transmisión se complete
	while(!(SPSR & (1 << SPIF)));
	// Retornar registro de datos
	return SPDR;	
}

/*********************************************************************************************************
INTERCAMBIO DE BITS (CONSULTA DESDE MAESTRO)
**********************************************************************************************************/
uint8_t SPI_Master_Query(volatile uint8_t *slave_port, uint8_t slave_pin, uint8_t command)
{
	// Encender pin de esclavo
	*slave_port &= ~(1 << slave_pin);
	
	// Esta función se realiza en dos ciclos de reloj
	// 1. Enviar byte de comando al esclavo
	SPI_Transmit(command);
	
	// 2. Intercambiar un byte dummy por el byte solicitado por el comando
	SPI_Transmit(0X00);
	
	// Mostrar la salida obtenida
	return SPDR;
}