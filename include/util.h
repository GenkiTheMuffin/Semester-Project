#ifndef UTIL_H_
#define UTIL_H_

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include "adcpwm.h"
#include "usart.h"

void init(void);
uint32_t get_enc_period();
float measure_speed(uint32_t time);
float measure_volt_adc();

#endif