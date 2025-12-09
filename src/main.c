#define F_CPU 16000000UL
#define BAUD 9600
#define NUMBER_STRING 1001
#define WHEEL_RADIUS 0.31f // wheel radius (in meters)
#define ENCODER_SLOTS 8    // number of holes on encoder wheel
#define VOLT_SPEED 1       // voltage-speed conversion number


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
  int page = 0;
  int distance1 = 1, distance2 = 3, time1 = 15, time2 = 38;
  float progressbar = 0;
  float total_distance = 0;

  while (1) {

    update_nextion(&page, &distance1, &distance2, &time1, &time2, &progressbar);

    // Measure time //
    time_value = get_enc_period() / 1000; // gets encoder wheel time output in milliseconds

    // Read voltage; ADC conversion //
    voltage = measure_volt_adc();

    // Speed measurement //
    speed = measure_speed(time_value);

    // Update distance //
    update_current_distance(speed, time_value, &total_distance);    // updates the total taken distance until this moment


    // Execute //
    if (start && section1) {
      set_speed(time1, distance1, voltage);   // sets speed according to section 1

    } else if (start && section2) {
      set_speed(time2, distance2, voltage);   // sets speed according to section 2

    } else {
      set_speed(1, 0, voltage);               // stops the car
    }

    if (total_distance >= distance1 && section1) {    // switches from section 1 to section 2
      section1 = false;
      section2 = true;
    }
    if (total_distance >= (distance1 + distance2)) {  // resets the sections and start
      section1 = false;
      section2 = false;
      start = false;
      total_distance = 0;
    }

 
  }
  return 0;
}

