#include "uart.h"
#include "makra.h"
#include <stdio.h>

void UART0_init(uint16_t Baudrate) {
	uint16_t ubrr = (F_CPU / 16 / Baudrate) - 1;
	UBRR0H = (uint8_t)(ubrr >> 8);
	UBRR0L = (uint8_t)ubrr;
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	UCSR0B = (1 << RXEN0) | (1 << RXCIE0);
}

void UART1_init(uint16_t Baudrate) {
	uint16_t ubrr = (F_CPU / 16 / Baudrate) - 1;
	UBRR1H = (uint8_t)(ubrr >> 8);
	UBRR1L = (uint8_t)ubrr;
	UCSR1C = (1 << UCSZ11) | (1 << UCSZ10);
	UCSR1B = (1 << TXEN1);
}

void UART1_SendChar(uint8_t data) {
	while (!(UCSR1A & (1 << UDRE1)));
	UDR1 = data;
}

void UART1_SendString(char *text) {
	while (*text) {
		UART1_SendChar(*text++);
	}
}

void UART0_SendChar(uint8_t data) {
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = data;
}

void UART0_SendString(char *text) {
	while (*text) {
		UART0_SendChar(*text++);
	}
}


int UART1_printCHAR(char c, FILE *stream) {
	if (c == '\n') UART1_SendChar('\r');
	UART1_SendChar(c);
	return 0;
}
