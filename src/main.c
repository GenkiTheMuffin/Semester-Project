#define F_CPU 16000000UL
#define BAUD 9600
#define NUMBER_STRING 1001
#define WHEEL_RADIUS 0.31f  // wheel radius (in meters)
#define ENCODER_SLOTS 8  // number of holes on encoder wheel

#include <avr/io.h> 
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "usart.h"
#include "i2cmaster.h"
#include "adcpwm.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//// FUNCTION PROTOTYPES ////
void init();
void button_press (uint8_t buffer[]);                     // prototype of function to detect a button press
void read_value(char *pReadBuffer, uint32_t *pReadValue); // prototype of function to read value from screen
void measure_volt_adc();                                  // prototype of function to measure voltage and ADC
void button_press_detect();                               // prototype of function to detect button press
void print_result();                                      // prototype of function to print the results
void measure_speed(uint32_t);                             // prototype of function to measure the current speed
unsigned int get_enc_wheel_time();                        // prototype of function to get time measured by the encoder wheel
uint32_t get_enc_period();                                // prototype of encoder wheel time measurement

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//// VARIABLES ////
uint8_t buffer[7];        // array for bytes of signals from the display
uint32_t readValue = 0;
uint32_t *pReadValue = &readValue;
char readBuffer[100];
unsigned int time_value;  // for time measurement on the optocoupler
char *pReadBuffer = readBuffer;

unsigned int adc_value;
float voltage;
float speed;

uint8_t button1_page; // Variables for the different parts of the ID for the first button
uint8_t button1_ID;
uint8_t button1_release;
uint8_t button2_page; // Variables for the different parts of the ID for the second button
uint8_t button2_ID;
uint8_t button2_release;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(void)
{
  printf("page 0%c%c%c", 255,255,255); //init at 9600 baud.
  _delay_ms(20);
  
  pwm1_set_duty(20); // sets the duty cycle to 20 %

  
	while(1) {
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
    time_value = get_enc_period() / 1000; // gets encoder wheel time output in milliseconds
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
void init(){ 
	uart_init();    // communication with PC - debugging
	io_redirect();  // redirect printf to uart, text will be shown on PC or Nextion screen
  i2c_init();     // serial communication protocol
  pwm1_init();    // PWM signal at pin PD5 (4 kHz)
  adc_init();     // ADC module init
  
  DDRC = 0xf0;    // PC3-PC0 are inputs
  PORTC = 0x30;   // (PC3-PC0 use no pullups for the ADC)

  // Setup timer 1 as pure ticks counter, the clock ticks with 1024 prescaling; i.e. no interrupts //
  TCCR1A = 0x00;
  TCCR1B = 0xC5;  //Input capture on positive edge ICP1 pin (PB0). Filter is enabled. 1024 clock prescaler*
  DDRB  &= ~0x01; //PINB0 as input for ICP1 use
  PORTB |= 0x01;  //Enable pullup
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void button_press (uint8_t buffer[]) {
	if(buffer[0] == 0x65)        // conditioned on button press
	 {
		for (int i =1; i < 7; i++) // filling array with numbers from button signal
		{
			scanf("%c", &buffer[i]);
    	}
     } 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void read_value(char *pReadBuffer, uint32_t *pReadValue) {
  _delay_ms(20);

  printf("get %s.val%c%c%c", "page0.n0", 255,255,255); // sends "get page0.n0.val"    
  int typeExpected = 0;

  for (int i = 0; i < 8; i++) {
    scanf("%c", &pReadBuffer[i]);
    if (pReadBuffer[i] == 0x71) { // expect number string
      typeExpected = NUMBER_STRING;
      pReadBuffer[0] = 0x71; // move indicator to front, just to keep the nice format
      break;
      }
  }
  if (typeExpected == NUMBER_STRING) {
      for (int i = 1; i < 8; i++) {
        scanf("%c", &pReadBuffer[i]);
      }

      if (pReadBuffer[0] == 0x71 && pReadBuffer[5] == 0xFF && pReadBuffer[6] == 0xFF && pReadBuffer[7] == 0xFF) { // this is a complete number return
        *pReadValue = pReadBuffer[1] | (pReadBuffer[2] << 8) | (pReadBuffer[3] << 16) | (pReadBuffer[4] << 24);
      }
  }
  for (int i = 0; i < 7; i++) {
    scanf("%c", &pReadBuffer[i]);
    if (pReadBuffer[i] == 0x1A)    // retrieve the 0xFF commands and start over
    {
      scanf("%c", &pReadBuffer[i]);
      scanf("%c", &pReadBuffer[i]);
      scanf("%c", &pReadBuffer[i]);
      continue;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t get_enc_period(){
  uint32_t time1 = 0;
  uint32_t time2 = 0;

  // wait for 1 step up
  while (!(TIFR1 & (1 << ICF1)));
  time1 = ICR1;
  TIFR1 |= (1 << ICF1);  

  // wait for 2 step up
  while (!(TIFR1 & (1 << ICF1)));
  time2 = ICR1;
  TIFR1 |= (1 << ICF1);

  // count diference
  uint32_t ticks = (time2) - (time1);

  // return in microseconds
  return ticks * 64L;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void measure_speed(uint32_t time) {
  speed = (2 * M_PI * WHEEL_RADIUS / ENCODER_SLOTS) / ((float)time / 1000.0f); // (calculation: distance per pulse / time)
  printf("\nSpeed: %.1f m/s", speed);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void measure_volt_adc() {
  adc_value = adc_read(0);  // analog voltage on PC0
  voltage = adc_value * (5.0 / 1023.0f);
  printf("\nADC: %u | Voltage: %.3f V\r\n", adc_value, voltage);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void button_press_detect() {
  scanf("%c", &buffer[0]); //scan for first byte 
	button_press(buffer);

  button1_page = buffer[1];
	button1_ID = buffer[2];
	button1_release =buffer[3];

  if(button1_page == 0x00 && button1_ID == 0x01 && button1_release == 0x00) { // checking on which button has been pressed; it is true if button1 on page0 would be released
		printf("page page1%c%c%c", 255, 255, 255); // changes to page 2
	}	

	scanf("%c", &buffer[0]); // repetition of the above; now checking for a release signal for a button1 on page1
  button_press(buffer);

	button2_page = buffer[1];
	button2_ID = buffer[2];
	button2_release = buffer[3];

	if(button2_page == 0x01 && button2_ID == 0x01 && button2_release == 0x00) {
		printf("page page0%c%c%c", 255, 255, 255);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void print_result() {
  printf("\nTime between impulses: %u ms   \n", time_value);
  printf("Speed: %.1f m/s   \n", speed);
  printf("ADC: %u | Voltage: %.3f V\r   \n", adc_value, voltage);
}



