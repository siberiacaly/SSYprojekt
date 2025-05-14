#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <math.h>
#include "uart.h"
#include "nmea_parser.h"
#include "config.h"
#include "hal.h"
#include "phy.h"
#include "sys.h"
#include "nwk.h"
#include "sysTimer.h"
#include "halBoard.h"
#include "halUart.h"
#include "main.h"

#define APP_ENDPOINT 1

volatile uint16_t TRP = 0;
volatile uint16_t TSR = 0;


FILE uart_str = FDEV_SETUP_STREAM(UART1_printCHAR, NULL, _FDEV_SETUP_WRITE);


void board_init() {
	UART0_init(38400);
	UART1_init(38400);
}

void confirm(NWK_DataReq_t *req);

void confirm(NWK_DataReq_t *req) {
	if (req) {
		if (req->data) free(req->data);
		free(req);
	}
}

void HAL_UartBytesReceived(uint16_t bytes)
{
	(void)bytes; 
}
bool appDataInd(NWK_DataInd_t *ind)
{
	if (ind->size > 0 && strncmp((char*)ind->data, "SYNC,", 5) == 0) {
		TRP = TCNT1;
		_delay_us(100);  
		TSR = TCNT1;

		printf("SYNC PRIJAT: %s\r\n", ind->data);

		char *ack = malloc(64);
if (!ack) return false;
snprintf(ack, 64, "ACK,TRP=%u,TSR=%u", TRP, TSR);

NWK_DataReq_t *req = malloc(sizeof(NWK_DataReq_t));
if (!req) {
	free(ack);
	return false;
}

req->dstAddr = 0;
req->dstEndpoint = APP_ENDPOINT;
req->srcEndpoint = APP_ENDPOINT;
req->options = 0;
req->data = (uint8_t*)ack;
req->size = strlen(ack) + 1;
req->confirm = confirm;

NWK_DataReq(req);
	}
	return true;
}


void timer1_init() {
	TCCR1A = 0;
	TCCR1B = (1 << CS11) | (1 << CS10);
	TCNT1 = 0;
}

int main(void) {
board_init();
timer1_init();
	SYS_Init();
	HAL_BoardInit();
	HAL_UartInit(38400);
	stdout = &uart_str;  

	NWK_SetAddr(APP_ADDR);
	NWK_SetPanId(APP_PANID);
	PHY_SetChannel(APP_CHANNEL);
	PHY_SetRxState(true);
	NWK_OpenEndpoint(APP_ENDPOINT, appDataInd);

	printf("START SLAVE\n");

	sei();

	while (1) {
		SYS_TaskHandler();
		HAL_UartTaskHandler();
	}
}
