#define F_CPU 16000000UL
#define BAUD 9600
#define NUMBER_STRING 1001
#define WHEEL_RADIUS 0.31f // wheel radius (in meters)
#define ENCODER_SLOTS 8    // number of holes on encoder wheel

#include "adcpwm.h"
#include "i2cmaster.h"
#include "nextion.h"
#include "usart.h"
#include "util.h"
#include <avr/io.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//// VARIABLES ////
unsigned int time_value; // for time measurement on the optocoupler
unsigned int adc_value;
float voltage, speed;


// variable to check if car is in specific state
bool start = false;
bool section1 = false;
bool section2 = false;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(void) {
  init();
  pwm1_set_duty(20); // sets the duty cycle to 20 %
  int page = 0;

  while (1) {
    update_nextion(&page);
    
    //Measure time //
    time_value = get_enc_period() / 1000; // gets encoder wheel time output in milliseconds

    // Read voltage; ADC conversion//
    voltage = measure_volt_adc();

    // Speed measurement //
    speed = measure_speed(time_value);
  }
  return 0;
}

