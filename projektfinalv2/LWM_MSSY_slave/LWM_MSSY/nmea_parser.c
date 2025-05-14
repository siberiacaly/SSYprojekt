/*
 * nmea_parser.c
 *
 * Created: 4/7/2025 12:44:11
 *  Author: Student
 */ 

#include "nmea_parser.h"
#include <string.h>
#include <stdio.h>

void NMEA_ParseSentence(char *sentence, NMEAData *data) {
	if(sentence[0] == '$') {
		sentence++;
	}
	
	char *token;
	int field_index = 0;
	
	token = strtok(sentence, ",");
	
	while(token != NULL) {
		switch(field_index) {
			case 0:
			strncpy(data->sentence_type, token, sizeof(data->sentence_type) - 1);
			data->sentence_type[sizeof(data->sentence_type) - 1] = '\0';
			break;
			case 1:
			if (strlen(token) >= 9) {
				char hours[3], minutes[3], seconds[3], frac[4];
				strncpy(hours, token, 2);
				hours[2] = '\0';
				strncpy(minutes, token + 2, 2);
				minutes[2] = '\0';
				strncpy(seconds, token + 4, 2);
				seconds[2] = '\0';
				char *dot = strchr(token, '.');
				if (dot != NULL) {
					strncpy(frac, dot + 1, 3);
					frac[3] = '\0';
					} else {
					strcpy(frac, "000");
				}
				snprintf(data->time, sizeof(data->time), "%s:%s:%s,%s UTC", hours, minutes, seconds, frac);
				} else {
				strncpy(data->time, token, sizeof(data->time) - 1);
				data->time[sizeof(data->time) - 1] = '\0';
			}
			break;
			default:
			break;
		}
		field_index++;
		token = strtok(NULL, ",");
	}
}