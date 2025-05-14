/*
 * uart.h
 *
 * Created: 15.02.2023 12:19:38
 *  Author: Ondra
 */ 


#ifndef UART_H_
#define UART_H_

#include <stdio.h>
void UART0_init(uint16_t Baudrate);
void UART1_init(uint16_t Baudrate);
void UART1_SendChar(uint8_t data);
void UART1_SendString(char *text);
int UART1_printCHAR(char c, FILE *stream);



#endif /* UART_H_ */