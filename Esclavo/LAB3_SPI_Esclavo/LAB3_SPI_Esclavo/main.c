/*-------------------------------------------------------------------
/ ELECTRÓNICA DIGITAL 2 - LABORATORIO 3 (ESCLAVO)
/ Creado por: David Carranza y Mario Betancourt
/-------------------------------------------------------------------*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "ADC.h"

/*-------------------------------------------------------------------
/ CONFIGURACIÓN DE COMANDOS
/-------------------------------------------------------------------*/
#define CMD_GET1     0x11
#define CMD_GET2     0x22
#define CMD_SET_LEDS 0x33

/*-------------------------------------------------------------------
/ ESTRUCTURAS DE DATOS PARA LEDS (Mapeo de pines mezclados)
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
volatile uint8_t pot1 = 0;
volatile uint8_t pot2 = 0;
volatile uint8_t estado_leds = 0; // 0 = Esperando Comando, 1 = Esperando Dato LED

/*-------------------------------------------------------------------
/ PROTOTIPOS DE FUNCIONES
/-------------------------------------------------------------------*/
void InitPORT(void);
void WritePORT(uint8_t number);

/*-------------------------------------------------------------------
/ MAINLOOP
/-------------------------------------------------------------------*/
int main(void) {
    // 1. Inicializar Hardware
    InitPORT();         // Configurar pines de LEDs como salida
    DDRB |= (1 << DDB4); // MISO como salida (importante para Esclavo SPI)
    
    // 2. Inicializar ADC (Polling, 8 bits/ADLAR, prescaler 128)
    ADC_Init(0, 1, 0x07, 0, 0);

    // 3. Inicializar SPI Esclavo con interrupción
    SPCR = (1 << SPE) | (1 << SPIE);
    
    sei(); // Habilitar interrupciones globales

    while (1) {
        // --- Muestreo constante de Potenciómetros ---
        
        // Canal 0
        ADC_SetChannel(0);
        ADC_StartConversion();
        while(ADCSRA & (1 << ADSC));
        pot1 = ADCH;

        // Canal 1
        ADC_SetChannel(1);
        ADC_StartConversion();
        while(ADCSRA & (1 << ADSC));
        pot2 = ADCH;
    }
}

/*-------------------------------------------------------------------
/ INTERRUPCIÓN SPI (Máquina de Estados)
/-------------------------------------------------------------------*/
ISR(SPI_STC_vect) {
    uint8_t byte_recibido = SPDR;

    // Caso: Estamos en modo "Esperando dato para los LEDs"
    if (estado_leds == 1) {
        WritePORT(byte_recibido); // Usamos tu función de mapeo
        estado_leds = 0;          // Volver a modo comando
        return;                   
    }

    // Caso: Estamos en modo "Esperando Comando"
    if (byte_recibido == CMD_GET1) {
        SPDR = pot1; // Devolvemos Pot 1
    } 
    else if (byte_recibido == CMD_GET2) {
        SPDR = pot2; // Devolvemos Pot 2
    } 
    else if (byte_recibido == CMD_SET_LEDS) {
        estado_leds = 1; // Cambiar estado para la próxima interrupción
        SPDR = 0x00;     // Respuesta dummy
    }
}

/*-------------------------------------------------------------------
/ FUNCIONES AUXILIARES
/-------------------------------------------------------------------*/

// Inicializar LEDs usando tus vectores de punteros
void InitPORT(void) {
    for (uint8_t i = 0; i < 8; i++) {
        *LED_ddrs[i] |= (1 << LED_pins[i]);
    }
}

// Mostrar número binario recorriendo los puertos asignados
void WritePORT(uint8_t number) {
    for (uint8_t i = 0; i < 8; i++) {
        if (number & (1 << i)) {
            *LED_ports[i] |= (1 << LED_pins[i]);
        } else {
            *LED_ports[i] &= ~(1 << LED_pins[i]);
        }
    }
}

