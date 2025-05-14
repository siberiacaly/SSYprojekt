#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
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

#define NMEA_BUF_SIZE 82
#define APP_ENDPOINT 1

volatile char nmea_buffer[NMEA_BUF_SIZE];
volatile uint8_t nmea_index = 0;
char last_gps_time[20] = "??:??:??,??? UTC";
volatile uint16_t TSP = 0;
volatile uint16_t TSP_last = 0;
volatile uint8_t gps_time_valid = 0;

volatile char gps_line_to_print[40];
volatile uint8_t gps_line_ready = 0;

volatile char rtt_line[60];
volatile uint8_t rtt_ready = 0;

FILE uart_str = FDEV_SETUP_STREAM(UART1_printCHAR, NULL, _FDEV_SETUP_WRITE);

static char syncMsg[64];
static char helloMsg[] = "HELLO from MASTER";
static uint8_t helloSent = 0;

void confirm(NWK_DataReq_t *req) {
	if (req) {
		if (req->data) free(req->data);
		free(req);
	}
}

bool masterDataInd(NWK_DataInd_t *ind) {
	if (strncmp((char*)ind->data, "ACK,", 4) == 0 && gps_time_valid) {
		gps_time_valid = 0; // ? spot?ebuj validní ?as

		uint16_t TRP = 0, TSR = 0, TRF = TCNT1;
		sscanf((char*)ind->data, "ACK,TRP=%hu,TSR=%hu", &TRP, &TSR);

		uint16_t TSP = TSP_last;
		int32_t RTT = (int32_t)(TRF - TSP);
		int32_t procDelay = (int32_t)(TSR - TRP);
		int32_t ToF = (RTT - procDelay) / 2;

		uint32_t RTT_ms = (RTT * 1000UL) / 250000UL;
		uint32_t ToF_ms = (ToF * 1000UL) / 250000UL;

		printf("RTT = %ld ticks (%lu ms), ToF = %ld ticks (%lu ms)\r\n",
		RTT, RTT_ms, ToF, ToF_ms);
	}
	return true;
}




void HAL_UartBytesReceived(uint16_t bytes)
{
	(void)bytes;
}

void board_init() {
	cli();
	UART0_init(9600);      // GPS vstup
	UART1_init(38400);     // terminál
	sei();
}

void timer1_init() {
	TCCR1A = 0;
	TCCR1B = (1 << CS11) | (1 << CS10); // prescaler 64 ? 250 kHz
	TCNT1 = 0;
}

void timer2_init() {
	cli();
	TCCR2A = (1 << WGM21);  // CTC režim
	TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20); // prescaler 1024
	OCR2A = 78;  // 10 ms
	TIMSK2 |= (1 << OCIE2A);
	sei();
}

int main(void) {
	board_init();
	timer1_init();
	timer2_init();

	SYS_Init();
	HAL_BoardInit();
	HAL_UartInit(38400);
	stdout = &uart_str;  // ? D?LEŽITÉ PRO FUNK?NÍ printf()

	NWK_SetAddr(APP_ADDR);       // master = 0
	NWK_SetPanId(APP_PANID);
	PHY_SetChannel(APP_CHANNEL);
	PHY_SetRxState(true);
	NWK_OpenEndpoint(APP_ENDPOINT, masterDataInd);
	printf("START MASTER SYNC\r\n");

	sei();

	while (1) {
		SYS_TaskHandler();
		HAL_UartTaskHandler();
	}
}

ISR(USART0_RX_vect) {
	char received = UDR0;

	if (received == '\n') {
		nmea_buffer[nmea_index] = '\0';

		NMEAData data;
		NMEA_ParseSentence((char*)nmea_buffer, &data);

		// Zkontroluj validitu ?asu: HH:MM:SS,xxx UTC
		if (data.time[0] >= '0' && data.time[0] <= '9' &&
		strcmp(data.time, last_gps_time) != 0) {

			strcpy(last_gps_time, data.time);
			gps_time_valid = 1;

			// Výpis rovnou
			printf("Cas z GPS: %s\r\n", last_gps_time);
		}

		nmea_index = 0;
		} else {
		if (nmea_index < NMEA_BUF_SIZE - 1)
		nmea_buffer[nmea_index++] = received;
		else
		nmea_index = 0;
	}
}



ISR(TIMER2_COMPA_vect) {
	static uint8_t tick_count = 0;
	tick_count++;

	if (tick_count >= 100) {
		tick_count = 0;

		TSP = TCNT1;
		TSP_last = TSP;

		// ?? Dynamická alokace bufferu
		char *msg = malloc(64);
		if (!msg) return;

		snprintf(msg, 64, "SYNC,%s,TSP=%u", last_gps_time, TSP);

		// ?? Dynamická alokace struktury požadavku
		NWK_DataReq_t *req = malloc(sizeof(NWK_DataReq_t));
		if (!req) {
			free(msg);
			return;
		}

		req->dstAddr = 1;
		req->dstEndpoint = APP_ENDPOINT;
		req->srcEndpoint = APP_ENDPOINT;
		req->options = 0;
		req->data = (uint8_t*)msg;
		req->size = strlen(msg) + 1;
		req->confirm = confirm;

		NWK_DataReq(req);
	}
}


