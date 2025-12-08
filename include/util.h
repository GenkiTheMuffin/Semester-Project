#ifndef UTIL_H_
#define UTIL_H_

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

void init(void);
uint32_t get_enc_period();
void measure_speed(uint32_t time);
void measure_volt_adc();

#endif