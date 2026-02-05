#include "SPI.h"

void SPI_Init(SPI_CONFIG config, SPI_CLOCK_MODE clock_mode, SPI_DORD data_order, SPI_INTERRUPT_CONFIG en_interrupts)
{
	// 1. Configurar SPCR
	// Usamos una máscara 0x13 para dejar pasar solo MSTR (bit 4) y SPR1:0 (bits 1:0)
	// Así evitamos que la basura de la enumeración afecte otros bits.
	SPCR = (1 << SPE) | (config & 0x13) | clock_mode | data_order;
	
	// Habilitar interrupciones si se solicita
	if(en_interrupts == SPI_INTERRUPTS_ENABLED) {
		SPCR |= (1 << SPIE);
	}

	// 2. Configurar SPI2X en SPSR
	// Buscamos nuestra bandera manual de 0x04 (bit 2) en la configuración
	if (config & 0x04) {
		SPSR |= (1 << SPI2X);
		} else {
		SPSR &= ~(1 << SPI2X);
	}

	// 3. Configuración de Puertos (Crucial para estabilidad)
	if(config & (1 << MSTR)) {
		// MODO MAESTRO:
		// Forzamos MOSI, SCK y SS como SALIDAS.
		// ¡PB2 (SS) DEBE ser salida para que el Maestro no se desactive!
		DDR_SPI |= (1 << DD_MOSI) | (1 << DD_SCK) | (1 << DD_SS);
		} else {
		// MODO ESCLAVO:
		// Solo MISO es salida. SS, MOSI y SCK son entradas.
		DDR_SPI |= (1 << DD_MISO);
		DDR_SPI &= ~((1 << DD_MOSI) | (1 << DD_SCK) | (1 << DD_SS));
	}
}

void SPI_Transmit(uint8_t data)
{
	SPDR = data;
	while(!(SPSR & (1 << SPIF)));
}

uint8_t SPI_Recieve(uint8_t data)
{
	// Para el esclavo, solo esperamos a que el Maestro mande algo
	while(!(SPSR & (1 << SPIF)));
	return SPDR;
}