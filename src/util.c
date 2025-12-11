#include "util.h"

#define WHEEL_RADIUS 0.31f // wheel radius (in meters)
#define ENCODER_SLOTS 16   // number of holes on encoder wheel
#define VOLT_SPEED 500     // voltage-speed conversion number

void init() {
  uart_init();   // communication with PC - debugging
  io_redirect(); // redirect printf to uart, text will be shown on PC or
                 // Nextion screen
  pwm1_init();   // PWM signal at pin PD5 (4 kHz)
  adc_init();    // ADC module init

  DDRC = 0xf0;  // PC3-PC0 are inputs
  PORTC = 0x30; // (PC3-PC0 use no pullups for the ADC)

  // Setup timer 1 as pure ticks counter, the clock ticks with 1024 prescaling;
  // i.e. no interrupts //
  TCCR1A = 0x00;
  TCCR1B = 0xC5; // Input capture on positive edge ICP1 pin (PB0). Filter is
                 // enabled. 1024 clock prescaler*
  DDRB &= ~0x01; // PINB0 as input for ICP1 use
  PORTB |= 0x01; // Enable pullup

  printf("page 0%c%c%c", 255, 255, 255); // init at 9600 baud.
  _delay_ms(20);
}

uint32_t get_enc_period() {
  uint32_t time1 = 0;
  uint32_t time2 = 0;

  // wait for 1 step up
  while (!(TIFR1 & (1 << ICF1)))
    ;
  time1 = ICR1;
  TIFR1 |= (1 << ICF1);

  // wait for 2 step up
  while (!(TIFR1 & (1 << ICF1)))
    ;
  time2 = ICR1;
  TIFR1 |= (1 << ICF1);

  // count diference
  uint32_t ticks = (time2) - (time1);

  // return in microseconds
  return ticks * 64L;
}

float measure_speed(uint32_t time) {
  float speed =
      (2 * M_PI * WHEEL_RADIUS / ENCODER_SLOTS) /
      ((float)time /
       1000.0f); // (calculation: distance per pulse / time between pulses)
  return speed;
}

float measure_volt_adc() {
  int adc_value = adc_read(0); // analog voltage on PC0
  float voltage = adc_value * (5.0 / 1023.0f);
  return voltage;
}

void set_speed(int time, int distance, float voltage) {
  float speed = (float)(distance) / (float)(time);
  float required_volt =
      speed *
      (float)
          VOLT_SPEED; // gets the required voltage for the sepcific speed value
  pwm1_set_duty(required_volt /
                voltage); // sets the pwm duty to the required voltage %
  printf("\ndistance: %d, time: %d", distance, time);
  printf("\ndistance/time: %f", (float)(distance) / (float)(time));
  printf("\nspeed: %.1f", speed);
  printf("\nRequired voltage/voltage: %.1f", required_volt / voltage);

  /*speed ~ voltage
   "x" m/s = "y" volts
   "Z": total voltage of battery ("Z" >= "y")
   if "y" volts needed -> "y"/"Z" % of total voltage -> pwm1_set_duty("y"/"Z")*/
}

void update_current_distance(float *total_distance) {
  *total_distance += (2 * M_PI * WHEEL_RADIUS / ENCODER_SLOTS);
}

void active_speed_control(float *pNeeded_speed, float *pCurrent_speed,
                          int *pDuty, int step) {
  if (*pCurrent_speed > *pNeeded_speed) {
    *pDuty -= step;
  } else if (*pCurrent_speed < *pNeeded_speed) {
    *pDuty += step;
  }
  pwm1_set_duty(*pDuty);
}
