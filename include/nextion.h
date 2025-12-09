#ifndef NEXTION_H_
#define NEXTION_H_

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

int getchar_timeout(uint16_t timeout_ms);
int read_nextion_message(uint8_t *buf, int maxlen);
uint32_t read_nextion_value(char* valtype);
void update_nextion(int *page, int *distance1, int *distance2, int *time1, int *time2, float *progressbar);
int get_page();

#endif