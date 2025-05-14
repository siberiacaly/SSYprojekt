/*
 * makra.h
 *
 * Created: 15.02.2023 12:26:25
 *  Author: Ondra
 */ 


#ifndef MAKRA_H_
#define MAKRA_H_

#include <avr/io.h>

//Bitove operace
#define sbi(var, mask)  ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)  ((var) &= (uint8_t)~(1 << mask))
#define tbi(var,mask)	(var & (1 << mask))
#define xbi(var,mask)	((var)^=(uint8_t)(1 << mask))
#define sbiX(var, mask) (var|= (1 << mask))
//hardware








#endif /* MAKRA_H_ */