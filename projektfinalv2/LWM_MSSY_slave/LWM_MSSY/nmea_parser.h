/*
 * nmea_parser.h
 *
 * Created: 4/7/2025 12:44:59
 *  Author: Student
 */ 


#ifndef NMEA_PARSER_H_
#define NMEA_PARSER_H_
#include <stdint.h>

#define NMEA_MAX_SENTENCE_LENGTH 82

typedef struct {
	char sentence_type[6];
	char time[20];
} NMEAData;

void NMEA_ParseSentence(char *sentence, NMEAData *data);

#endif
