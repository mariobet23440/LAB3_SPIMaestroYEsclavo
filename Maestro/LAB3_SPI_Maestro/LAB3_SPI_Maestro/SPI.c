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
void SPI_Init(SPI_CONFIG config, SPI_CLOCK_MODE clock_mode, SPI_DORD data_order, SPI_INTERRUPT_CONFIG en_interrupts) {
	// 1. Limpiar y configurar SPCR (Usamos la máscara 0x03 para SPR1:0)
	SPCR = (1 << SPE) | (config & 0x53) | clock_mode | data_order;
	
	if(en_interrupts == SPI_INTERRUPTS_ENABLED) SPCR |= (1 << SPIE);

	// 2. Configurar SPI2X en SPSR si la bandera 0x04 está presente
	if (config & 0x04) SPSR |= (1 << SPI2X);
	else SPSR &= ~(1 << SPI2X);

	// 3. Puertos (Usando |= para no destruir otras configs)
	if(config & (1 << MSTR)) {
		DDR_SPI |= (1 << DD_MOSI) | (1 << DD_SCK) | (1 << DD_SS); // SS como salida por seguridad
		} else {
		DDR_SPI |= (1 << DD_MISO);
	}
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
	// Encender apagar pin de esclavo
	*slave_port &= ~(1 << slave_pin);
	
	// Esta función se realiza en dos ciclos de reloj
	// 1. Enviar byte de comando al esclavo
	SPI_Transmit(command);
	
	// 2. Intercambiar un byte dummy por el byte solicitado por el comando
	SPI_Transmit(0X00);
	
	*slave_port |= (1 << slave_pin);
	
	// Mostrar la salida obtenida
	return SPDR;
}