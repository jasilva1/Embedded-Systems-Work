// Joshua Silva
// ID Number: 33435438
// ECE 231: Lab Assignment #3

#define TRIG PB1 //PB1 = pin 15
#define ECHO PB0 //PB0 = pin 14
#define RANGE_PER_CLOCK 1.098
#include <avr/io.h>
#include "i2c.h"
#include "SSD1306.h"
#include <util/delay.h>
void send_to_OLED(float);
void timer0_init(void);

int main(void) {
  OLED_Init();  //initialize the OLED

  unsigned char rising_edge_clocks, falling_edge_clocks, echo_width_clocks;
  float target_range;
  DDRB = 1<<TRIG;         // TRIG is output pin
  PORTB &= ~(1<<TRIG);    // Setting TRIG low
  timer0_init();          // Initialize timer0
  while (1) {
    TCNT0 = 0;          // load counter with 0

    PORTB |= 1<<TRIG;
    _delay_us(10);      // 10 usec pulse on 
    PORTB &= ~(1<<TRIG);// TRIG pin

    // wait till ECHO pulse goes high
    while ((PINB & (1<<ECHO)) == 0);
    rising_edge_clocks = TCNT0; // note time
    // wait till ECHO pulse goes low
    while(!(PINB & (1<<ECHO)) == 0);
    falling_edge_clocks = TCNT0;

    if(falling_edge_clocks > rising_edge_clocks){
      // compute target range and send to OLED
      echo_width_clocks = falling_edge_clocks - rising_edge_clocks;
      target_range = echo_width_clocks * RANGE_PER_CLOCK;
      send_to_OLED(target_range);
    }
    _delay_ms(500); // Delay then go again
  }
  return 0;
}

// send info to OLED
void send_to_OLED(float range){
  OLED_GoToLine(2);
  OLED_DisplayString(" Target Range: ");
  OLED_DisplayNumber(10,range,3);
  OLED_DisplayString(" cm ");
  OLED_GoToLine(4);
  OLED_DisplayString(" Target Range: ");
  OLED_DisplayNumber(10,range/2.54,3);
  OLED_DisplayString(" in ");

}

// Initialize timer0
void timer0_init(){
    TCCR0A = 0;     // Timer 1 Normal mode (count up)
    TCCR0B = 5;     // Divide clock by 1024
    TCNT0=0;        // Start the timer at 0
}
//End File//