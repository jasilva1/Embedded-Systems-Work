// Joshua Silva
// 33435438
// ECE 388 Project

#define TRIG1 PB1
#define ECHO1 PB0
#define TRIG2 PB4
#define ECHO2 PB5
#define MOTOR_PIN PB2
#define GREEN_LED PD2
#define YELLOW_LED PD3
#define RED_LED PD4
#define RANGE_PER_CLOCK 1.098
#define CONVERT_CM_TO_IN 2.54 // Centimeters to Inches conversion
#define OCCUPANCY_RANGE_CM 60.96 // range within which a person is considered entering or exiting (2 feet in cm)
#define Vref 1.1
#define MAX_TEMPERATURE 100.4 // Maximum allowed body temperature for occupancy
#define MAX_OCCUPANCY 25 // Maximum room occupancy
#include <avr/io.h>
#include "i2c.h"
#include "SSD1306.h"
#include <util/delay.h>

void disp_info(int, double, float, float); //void display on OLED
void timer0_init(); //initialize timer
double calculate_temp(unsigned int Vout);
void adc_init(void);
unsigned int get_adc(void);

// Display occupancy level on OLED
void disp_info(int occupancy, double temperature, float distance1, float distance2){
    OLED_GoToLine(0);
    OLED_DisplayString("Occupancy: ");
    OLED_DisplayNumber(10, occupancy, 3);

    OLED_GoToLine(2);
    OLED_DisplayString("Temperature: ");
    OLED_DisplayNumber(10, temperature, 2);
    OLED_DisplayString(" F");
    
    OLED_GoToLine(4);
    OLED_DisplayString("Distance 1: ");
    OLED_DisplayNumber(10, distance1, 2);
    OLED_DisplayString(" cm");

    OLED_GoToLine(6);
    OLED_DisplayString("Distance 2: ");
    OLED_DisplayNumber(10, distance2, 2);
    OLED_DisplayString(" cm");


}

// Initialize timer
void timer0_init(){
    TCCR0A = 0; //Normal mode (count up)
    TCCR0B = 5; //Prescaler = 1024
    TCNT0 = 0; //Load counter with 0
}

// Generate 10us pulse on TRIG pin
void trig_pulse(uint8_t pin){
    PORTB |= (1<<pin); //TRIG to high
    _delay_us(10); // 10us delay
    PORTB &= ~(1<<pin); //TRIG to low
}

// Wait for ECHO pin to go high
unsigned char start_time(uint8_t pin){
    while (!(PINB & (1<<pin))); // wait for ECHO to go high
    return TCNT0; // return timer value
}

// Wait for ECHO pin to go low
unsigned char end_time(uint8_t pin) {
    while ((PINB & (1<<pin))); // wait for ECHO to go low
    return TCNT0; // return timer value
}

// Send pulse, measure time (pulse width), and calculate distance
float measure_distance(uint8_t trig_pin, uint8_t echo_pin) {
    trig_pulse(trig_pin);

    unsigned char start_clocks = start_time(echo_pin); //Timer value at start of echo
    unsigned char end_clocks = end_time(echo_pin); // timer value at end of echo

    if(end_clocks > start_clocks){
        return (end_clocks - start_clocks) * RANGE_PER_CLOCK; //Calculating distance
    } else {
        return -1;
    }
}

// Initialize ADC
void adc_init(void){
    ADMUX = 0xc3;        // Select ADC5 Vref=1.1V
    ADCSRA = 0x87;       // enable ADC and setting speed to 125 KHz for 16 MHz clock
}

// Read ADC value
unsigned int get_adc(){
    ADCSRA |= (1 << ADSC);                //starting ADC conversion
    while (!(ADCSRA & (1 << ADIF)));  
    return ADCL | (ADCH << 8);            //read ADCL first 
}

// calculating temperature function
double calculate_temp(unsigned int Vout){
    double temperatureC = Vout*Vref/10.24 - 50;
    return temperatureC*1.8 + 32; // converting to F
}
// Run Motor for 3 seconds
void run_motor(){
    PORTB |= (1<<MOTOR_PIN); // Turn motor on
    _delay_ms(3000); // Wait for 3 seconds
    PORTB &= ~(1<<MOTOR_PIN); //Turn motor off
    _delay_ms(500);
}

// Main Function
int main(void) {
    OLED_Init();
    DDRB |= (1<<TRIG1) | (1<<TRIG2) | (1<<MOTOR_PIN); // set TRIG pins and motor as output
    PORTB &= ~((1<<TRIG1) | (1<<TRIG2)); // set TRIG pins to low
    DDRD |= (1<<RED_LED) | (1<<GREEN_LED) | (1<<YELLOW_LED);
    adc_init(); // Initialize ADC
    timer0_init(); // Initialize timer
    int occupancy = 0; // Initialize occupancy
    PORTD &= ~((1<<RED_LED) | (1<<YELLOW_LED) | (1<<GREEN_LED));

    while (1) {
        double temperature = calculate_temp(get_adc()); // Calculate temperature
        float distance1 = measure_distance(TRIG1, ECHO1); // Measure distance with the first sensor
        float distance2 = measure_distance(TRIG2, ECHO2); // Measure distance with the second sensor

        if(distance1 > 0 && distance2 >0){
            disp_info(occupancy, temperature, distance1, distance2); // Display occupancy and temperature

            // Check if a person is entering or exiting
            if(distance1 < OCCUPANCY_RANGE_CM && occupancy < MAX_OCCUPANCY && temperature < MAX_TEMPERATURE){
                occupancy++; // Someone is entering, add one to the occupancy number
                run_motor();
            } else if(distance2 < OCCUPANCY_RANGE_CM && occupancy > 0){
                occupancy--; //Someone is exiting, subtract one from the occupancy number
                run_motor();
            }
            else{
                PORTD |= (1<<RED_LED); // Turn on red led 
            }
            
        }
        if(occupancy <= 12){
            PORTD |= (1<<GREEN_LED); //Room low occupancy (less than 12 people in the room)
            PORTD &= ~((1<<RED_LED) | (1<<YELLOW_LED));
        } else if(occupancy <= 24){
            PORTD |= (1<<YELLOW_LED); //Room medium occupancy (13-24 people in the room)
            PORTD &= ~((1<<GREEN_LED) | (1<<RED_LED));
        } else{
            PORTD |= (1<<RED_LED); // Room full
            PORTD &= ~((1<<YELLOW_LED) | (1<<GREEN_LED));
        }
    }
}