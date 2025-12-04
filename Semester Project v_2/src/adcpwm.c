// Author: AD-SDU, 2024
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include "adcpwm.h"

volatile uint8_t _duty0 = 0, _duty1 = 0, _duty2 = 0, _timer_tick;

void pwm1_init(void){
    cli();
    DDRD |= (1<<PD5);   // sets to PD5
    // Set Fast PWM mode, non-inverted output on Timer 1
    TCCR0A = (1<<WGM00) | (1<<WGM01) | (1<<COM0B1); // Fast PWM, 8-bit
    TCCR0B = (1<<CS01) | (1<<CS00); // Prescaler: 64 > Frequency approx. 4 kHz
}


void pwm3_init(void){
    DDRD |= (1<<PD6)|(1<<PD5)|(1<<PD4); // on PD6, PD5, PD4
    // Disable Timer1
   // TCCR1B = 0; // Makes sure timer 1 is not running
    TCCR1A = (1 << WGM11); // Normal Mode 
    TIMSK1 = (1 << OCIE1A);
    OCR1A = 1;
    sei();
    TCCR1B = (1 << CS10); // No prescaler:  Frequency approx. 150Hz
}


void pwm1_set_duty(unsigned char input){
    if (input <= 100){
        OCR0B = input*2.55; // 0 .. 255 range
    }
       
}

void pwm3_set_duty(uint8_t input0, uint8_t input1, uint8_t input2){
    TCNT1 = 0;
    if (input0 <= 100){
        _duty0 = input0;
    }
    if (input1 <= 100){
        _duty1 = input1;
    }
    if (input2 <= 100){
        _duty2 = input2;
    }
       
}

void adc_init(void){
    ADMUX = (1<<REFS0); //set prescaler to 128 and turn on the ADC module
    ADCSRA = (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADEN);
}


uint16_t adc_read(uint8_t adc_channel){
    ADMUX &= 0xf0; // clear any previously used channel, but keep internal reference
    ADMUX |= adc_channel; // set the desired channel
    //start a conversion
    ADCSRA |= (1<<ADSC);
    // now wait for the conversion to complete
    while ((ADCSRA & (1<<ADSC)));
    // now we have the result, so we return it to the calling function as a 16 bit unsigned int
    return ADC;
}


ISR(TIMER1_COMPA_vect){
    // sets the pins to HIGH at start
    if (_timer_tick == 0){
        PORTD |= (1<<PD4)|(1<<PD5)|(1<<PD6);
    }
     // sets the pins to LOW at corresponding duty cycle
    if (_timer_tick == _duty0){
        PORTD &=~(1<<PD4);
    }
    if (_timer_tick == _duty1){
        PORTD &=~(1<<PD5);
    }
    if (_timer_tick == _duty2){
        PORTD &=~(1<<PD6);
    }
    
    _timer_tick++;
    if (_timer_tick == 100){
        _timer_tick = 0;
    }

}
