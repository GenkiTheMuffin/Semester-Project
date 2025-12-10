#ifndef UTIL_H_
#define UTIL_H_

#include "adcpwm.h"
#include "usart.h"
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

void init(void);
uint32_t get_enc_period();
float measure_speed(uint32_t time);
float measure_volt_adc();
void set_speed(int time, int distance, float voltage);
void update_current_distance(float speed, uint32_t time, float *total_distance);
void active_speed_control(float *needed_speed, float *current_speed, int *duty,
                          int step);
#endif