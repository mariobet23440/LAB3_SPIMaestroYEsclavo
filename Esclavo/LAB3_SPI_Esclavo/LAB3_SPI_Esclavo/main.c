/*
 * Lab3_Esclavo_SPI.c
 * Integración de ADC, SPI por interrupciones y Mapeo Dinámico de LEDs
 */ 

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



// #define F_CPU 16000000
// #include <avr/io.h>
// #include <avr/interrupt.h>
// #include <stdio.h>
// #include <stdint.h>
// #include <util/delay.h>
// #include "ADC.h"
// #include "SPI.h"
// #include "UART.h"
// 

// typedef enum
// {
// 	SPI_CMD_GET1	= 0X01,
// 	SPI_CMD_GET2	= 0x02,
// 	SPI_SEND_UART	= 0X03,
// 	SPI_SEND_DUMMY	= 0X04
// } COMMAND;
// 
// const char* status_text = "IDLE";
// 
// 

// 								  
// 								  
// volatile uint8_t adc_curr_channel = 0;
// volatile uint8_t adc_chan1 = 0;
// volatile uint8_t adc_chan2 = 1; 
// 
// volatile uint8_t pot1_value = 0;
// volatile uint8_t pot2_value = 0;
// 
// // Número enviado desde computadora
// volatile uint8_t computer_data = 5;
// 


// /*-------------------------------------------------------------------
// / SETUP
// /-------------------------------------------------------------------*/
// void setup(void){
// 	cli();
// 	
// 	// Inicializar puerto de contadores

// 	
// 	// Inicializar ADC
// 	ADC_Init(1,0,0x07,0,0);
// 	ADC_SetChannel(0);
// 	ADC_StartConversion();
// 	
// 	// Inicializar SPI
// 	SPI_Init(SLAVE_SS, MODE0_LE_SAMPLE_RISING, LSB_FIRST, SPI_INTERRUPTS_ENABLED);
// 	SPDR = 0X00; // Dummy
// 	
// 	UART_Init(UART_BAUD_9600_16MHZ, UART_INTERRUPTS_DISABLED);
// 	sei();
// }
// 
// /*-------------------------------------------------------------------
// / MAINLOOP
// /-------------------------------------------------------------------*/
// int main(void){
// 	setup();
// 	
// 	UART_sendString("\r\n");
// 	UART_sendString("-------------------------------------------------------------------- \r\n");
// 	UART_sendString("| LABORATORIO 3 - COMUNICACIÓN SPI - ESCLAVO                       | \r\n");
// 	UART_sendString("-------------------------------------------------------------------- \r\n");
// 	
// 	char mensaje[80]; // Buffer para construir la cadena
// 	
//     /* Replace with your application code */
//     while (1) 
//     {
// 		// Desplazamos 4 bits a la derecha para quedarnos con los 4 más significativos (0-15)
// 		uint8_t high_nibble = (pot1_value >> 4);
// 		uint8_t low_nibble  = (pot2_value >> 4);
// 
// 		// Colocamos pot1 en la parte alta y pot2 en la baja
// 		uint8_t output = (high_nibble << 4) | (low_nibble & 0x0F);
// 
// 		WritePORT(output);
// 		
// 		// Formateamos la cadena:
// 		// %03u indica un entero sin signo (uint8_t), de 3 dígitos, rellenado con '0'
// 		sprintf(mensaje, "| POTENCIOMETRO 1 : %03u | POTENCIOMETRO 2 = %03u | UART VALUE = %03u | SPI STATUS : %s \r\n", pot1_value, pot2_value, computer_data, status_text);
// 		
// 		// Enviamos la cadena completa por UART
// 		UART_sendString(mensaje);
// 		
// 		// Un pequeño delay para no saturar la terminal
// 		_delay_ms(1);
//     }
// }
// 
// /*-------------------------------------------------------------------
// / RUTINAS DE INTERRUPCIÓN
// /-------------------------------------------------------------------*/
// // Configurar siguiente recepción
// ISR(SPI_STC_vect)
// {
// 	// Dato recibido
// 	uint8_t message = SPDR;
// 	
// 	if(message == SPI_CMD_GET1){
// 		SPDR = pot1_value;
// 		status_text = "GET1";
// 	}
// 	else if(message == SPI_CMD_GET2){
// 		SPDR = pot2_value;
// 		status_text = "GET2";
// 	}
// 	else
// 	{
// 		SPDR = 0x00;
// 		status_text = "NONE";
// 	}
// }
// 
// // Interrupción de ADC - Leer ADC y multiplexar canales
// ISR(ADC_vect){
// 	uint16_t temp = ADC; 
// 	
// 	if(adc_curr_channel == adc_chan1){
// 		pot1_value = (uint8_t)(temp >> 2);
// 		adc_curr_channel = adc_chan2;
// 		ADC_SetChannel(adc_chan2);
// 		
// 	}
// 	else {
// 		pot2_value = (uint8_t)(temp >> 2);
// 		adc_curr_channel = adc_chan1;
// 		ADC_SetChannel(adc_chan1);
// 	}
// 	
// 	ADC_StartConversion();
// }
// 
 
