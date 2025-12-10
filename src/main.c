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
  int page = 1; // int page = 0;
  int distance1 = 1, distance2 = 3, time1 = 3, time2 = 38, duty = 50;
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

    // update_nextion(&page, &distance1, &distance2, &time1, &time2,
    // &progressbar);
    if (page == 1) {
      start = true;
    } else {
      start = false;
    }
    // Execute //
    pwm1_set_duty(*pDuty);
    if (start && section1) {
      time_value = get_enc_period();
      *pCurrent_speed = measure_speed(time_value);
      // set_speed(time1, distance1, voltage);   // sets speed according to
      // section 1

      active_speed_control(pNeeded_speed_1, pCurrent_speed, pDuty, 5);

    } else if (start && section2) {
      // set_speed(time2, distance2, voltage); // sets speed according to
      // section 2
      active_speed_control(pNeeded_speed_2, pCurrent_speed, pDuty, 5);
    } else {
      set_speed(1, 0, voltage); // stops the car
    }

    if (total_distance >= distance1 &&
        section1) { // switches from section 1 to section 2
      section1 = false;
      section2 = true;
    }
    if (total_distance >=
        (distance1 + distance2)) { // resets the sections and start
      section1 = false;
      section2 = false;
      start = false;
      total_distance = 0;
    }

    if (start) {
      // Measure time //
      time_value = get_enc_period() /
                   1000; // gets encoder wheel time output in milliseconds

      // Speed measurement //
      speed = measure_speed(time_value);

      // Update distance //
      update_current_distance(speed, time_value,
                              &total_distance); // updates the total taken
                                                // distance until this moment
    }
  }
  return 0;
}
