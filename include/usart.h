#ifndef USART_H
#define USART_H

#include <stdio.h>
#include <avr/io.h>

int uart_putchar(char c, FILE *stream);
int uart_getchar(FILE *stream);

void uart_init(void);

#endif

