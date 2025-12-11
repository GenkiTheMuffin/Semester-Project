#define F_CPU 16000000UL
#define BAUD 9600
#define NUMBER_STRING 1001
#define WHEEL_RADIUS 0.31f // wheel radius (in meters)
#define ENCODER_SLOTS 16   // number of holes on encoder wheel
#define VOLT_SPEED 500     // voltage-speed conversion number

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
bool section1 = true;
bool section2 = false;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(void) {
  init();
  int page = 0; // int page = 0;
  int distance1 = 1, distance2 = 3, time1 = 3, time2 = 38, duty = 100;
  float progressbar = 0;
  float total_distance = 0;
  float needed_speed_1, needed_speed_2, current_speed;
  float *pNeeded_speed_1 = &needed_speed_1;
  float *pNeeded_speed_2 = &needed_speed_2;
  float *pCurrent_speed = &current_speed;
  int *pDuty = &duty;

  while (1) {
    
    // Read voltage; ADC conversion //
    voltage = measure_volt_adc();

    // Updates the display //
    update_nextion(&page, &distance1, &distance2, &time1, &time2, &progressbar, &total_distance);


    while(page == 1) {
      // Execute //
      pwm1_set_duty(*pDuty);                          // sets the motor voltage to the required value
      *pCurrent_speed = measure_speed(time_value);    // gets the current speed
      
    if (section1) {
      active_speed_control(pNeeded_speed_1, pCurrent_speed, pDuty, 2);  // sets speed according to section 1

    } else if (section2) {
      active_speed_control(pNeeded_speed_2, pCurrent_speed, pDuty, 2);  // sets speed according to section 2

    } else {
      pwm1_set_duty(0);   // stops the car
    }

    if (total_distance >= distance1 && section1) { // switches from section 1 to section 2
      section1 = false;
      section2 = true;
    }
    if (total_distance >= (distance1 + distance2)) { // resets the sections and start
      section1 = false;
      section2 = false;
      page = 0;
      total_distance = 0;
      progressbar = 0;
      printf("page 0%c%c%c", 255,255,255);

    }
    // Measure time //
    time_value = get_enc_period() / 1000; // gets encoder wheel time output in milliseconds

    // Speed measurement //
    speed = measure_speed(time_value);

    // Update distance //
    update_current_distance(&total_distance); // updates the total taken distance until this moment

    // Updates the display //
    update_nextion(&page, &distance1, &distance2, &time1, &time2, &progressbar, &total_distance);

    }
    
  }
  return 0;
}
