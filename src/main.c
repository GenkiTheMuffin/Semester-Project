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

//// FUNCTION PROTOTYPES ////
void init();
void button_press(
    uint8_t buffer[]); // prototype of function to detect a button press
void read_value(
    char *pReadBuffer,
    uint32_t *pReadValue);  // prototype of function to read value from screen
void measure_volt_adc();    // prototype of function to measure voltage and ADC
void button_press_detect(); // prototype of function to detect button press
void print_result();        // prototype of function to print the results
void measure_speed(
    uint32_t); // prototype of function to measure the current speed
unsigned int get_enc_wheel_time(); // prototype of function to get time
                                   // measured by the encoder wheel
uint32_t get_enc_period(); // prototype of encoder wheel time measurement

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//// VARIABLES ////
uint8_t buffer[7]; // array for bytes of signals from the display
uint32_t readValue = 0;
uint32_t *pReadValue = &readValue;
char readBuffer[100];
unsigned int time_value; // for time measurement on the optocoupler
char *pReadBuffer = readBuffer;

unsigned int adc_value;
float voltage;
float speed;

uint8_t button1_page; // Variables for the different parts of the ID for the
                      // first button
uint8_t button1_ID;
uint8_t button1_release;
uint8_t button2_page; // Variables for the different parts of the ID for the
                      // second button
uint8_t button2_ID;
uint8_t button2_release;

// variable to check if car is in specific state
bool start = false;
bool section1 = false;
bool section2 = false;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(void) {
  printf("page 0%c%c%c", 255, 255, 255); // init at 9600 baud.
  _delay_ms(20);

  pwm1_set_duty(20); // sets the duty cycle to 20 %

  while (1) {
    /*    while(1){
      while(!distances speed set){
        read the display to set values
        }

      while(!start){
        check for start button press}

      calculate speed necessary for sections,

      while(start & section 1){
        give motor power
        check optocoupler
        if(speed too low){
          increase power}
        if(speed too high){
          decrease power}
      }
      */
    /*
    // Display 2 numbers on the Nextion display //
    printf("page0.n0.val=%d%c%c%c", 69, 255, 255, 255);
    printf("page0.n0.val=%d%c%c%c", 420, 255, 255, 255);

    // Read integer from the Nextion display  //
    read_value(pReadBuffer, pReadValue);
    printf("page0.n0.val=%d%c%c%c", 2 * (int)readValue, 255,255,255);
    printf("page0.n1.val=%d%c%c%c", (int)readValue, 255,255,255);
    */

    // Measure time //
    time_value = get_enc_period() /
                 1000; // gets encoder wheel time output in milliseconds
    printf("\nTime between impulses: %u ms\n", time_value);

    // Read voltage; ADC conversion//
    measure_volt_adc();

    // Button press detection on the Nextion display //
    button_press_detect();

    // Speed measurement //
    measure_speed(time_value);

    // Print results //
    print_result();

    _delay_ms(100);
  }
  return 0;
}

//// FUNCTION IMPLEMENTATIONS ////

void button_press_detect() {
  scanf("%c", &buffer[0]); // scan for first byte
  button_press(buffer);

  button1_page = buffer[1];
  button1_ID = buffer[2];
  button1_release = buffer[3];

  if (button1_page == 0x00 && button1_ID == 0x01 &&
      button1_release ==
          0x00) { // checking on which button has been pressed; it is true if
    // button1 on page0 would be released
    printf("page page1%c%c%c", 255, 255, 255); // changes to page 2
  }

  scanf("%c", &buffer[0]); // repetition of the above; now checking for a
                           // release signal for a button1 on page1
  button_press(buffer);

  button2_page = buffer[1];
  button2_ID = buffer[2];
  button2_release = buffer[3];

  if (button2_page == 0x01 && button2_ID == 0x01 && button2_release == 0x00) {
    printf("page page0%c%c%c", 255, 255, 255);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void print_result() {
  printf("\nTime between impulses: %u ms   \n", time_value);
  printf("Speed: %.1f m/s   \n", speed);
  printf("ADC: %u | Voltage: %.3f V\r   \n", adc_value, voltage);
}
