// ECE231 Lab Assignment #2
//Joshua Silva
//33435438

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>      // strlen() function
#include <stdlib.h>      // itoa() function
#define PERSISTENCE 5
#define COUNTTIME 200     // # time between counts (ms)
#define Vref 1.1
void uart_init(void);
void uart_send(char letter);
void send_string(char *stringAddress);
void adc_init(void);
unsigned int get_adc(void);

int main(void){

    unsigned char led24digits[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D,0x07, 0x7F, 0x67};
    unsigned char led3digits[] = {0xBF, 0x86, 0xDB, 0xCF, 0xE6, 0xED, 0xFD,0x87, 0xFF, 0xE7};
    unsigned char units[] = {0x39, 0x71};
    unsigned char DIG1, DIG2, DIG3;
    DDRC = 0x0F;                   // display segments a-d use PC0-PC3
    DDRD = 0xF0;                   // display segments e-g & dp use PD4-7
    PORTD |= 1<<PORTD2;            // Set pull-up on PD2
    DDRB = 0xFF;                   // Digit enable pins

    unsigned int Vout; 
    double temperatureC, temperatureF;
    adc_init();
    uart_init();                   

    while (1) {
        Vout = get_adc();

        temperatureC = Vout*Vref/10.24 - 50;            // temperature in Celsius             
        temperatureF = temperatureC*1.8 + 32;          // temperature in Fahrenheit
        
        if ((PIND & (1<<PIND2)) == 0){

            DIG1 = temperatureC/10;                       // 10's digit
            DIG2 = (int)temperatureC%10;                  // 1's digit
            DIG3 = (temperatureC - (DIG1*10) - DIG2)*10;  // 0.1's digit
            int j;
            for (j=0; j<COUNTTIME/PERSISTENCE/4; j++){

                send_string("Temperature in degrees of Celsius: ");

                // 10's digit
                PORTC = led24digits[DIG1];
                PORTD = led24digits[DIG1];
                uart_send(DIG1+'0');               // Tx 10's digit
                PORTB = ~ (1<<4);                  // Enable DIG4
                _delay_ms(PERSISTENCE);

                // 1's digit
                PORTC = led3digits[DIG2];
                PORTD = led3digits[DIG2];
                uart_send(DIG2+'0');               // Tx 1's digit
                uart_send('.');
                PORTB = ~ (1<<3);                 // DIG3
                _delay_ms(PERSISTENCE);

                // 0.1's digit
                PORTC = led24digits[DIG3];
                PORTD = led24digits[DIG3];
                uart_send(DIG3+'0');               // Tx 0.1's digit
                PORTB = ~ (1<<2);                 // DIG2
                _delay_ms(PERSISTENCE);

                // Display units
                PORTC = units[0];
                PORTD = units[0];
                uart_send('C');
                PORTB = ~ (1<<1);              // DIG1
                _delay_ms(PERSISTENCE);

                PORTB = 0xFF;                      // Disable digits
                uart_send(13);                     // Tx carriage return
                uart_send(10);                     // Tx line feed

            }
            
        }else {

            DIG1 = temperatureF/10;                       // 10's digit
            DIG2 = (int)temperatureF%10;                  // 1's digit
            DIG3 = (temperatureF - (DIG1*10) - DIG2)*10;  // 0.1's digit
            int j;
            for (j=0; j<COUNTTIME/PERSISTENCE/4; j++){

                send_string("Temperature in degrees of Fahrenheit: ");

                // 10's digit
                PORTC = led24digits[DIG1];
                PORTD = led24digits[DIG1];
                uart_send(DIG1+'0');               // Tx 10's digit
                PORTB = ~ (1<<4);                 // DIG4
                _delay_ms(PERSISTENCE);

                // 1's digit
                PORTC = led3digits[DIG2];
                PORTD = led3digits[DIG2];
                uart_send(DIG2+'0');               // Tx 1's digit
                uart_send('.');
                PORTB = ~ (1<<3);                 // DIG3
                _delay_ms(PERSISTENCE);

                // 0.1's digit
                PORTC = led24digits[DIG3];
                PORTD = led24digits[DIG3];
                uart_send(DIG3+'0');               // Tx 0.1's digit
                PORTB = ~ (1<<2);                  // DIG2
                _delay_ms(PERSISTENCE);

                // Display units
                PORTC = units[1];
                PORTD = units[1];
                uart_send('F');
                PORTB = ~ (1<<1);                  // DIG1
                _delay_ms(PERSISTENCE);

                PORTB = 0xFF;                      // Disable digits
                uart_send(13);                     // Tx carriage return
                uart_send(10);                     // Tx line feed

            }            
        }
    }
    return 0;
}


// Initialize ADC and select ADC5; Vref=1.1v;
// Enable ADC and set speed to 125 KHz for a 16 MHz clock
void adc_init(void){
    ADMUX = 0xc5;        // Select ADC5 Vref=1.1V
    ADCSRA = 0x87;       // Enable ADC and setting speed to 125 KHz for 16 MHz clock
}
// Read ADC value
unsigned int get_adc(){
    ADCSRA |= (1 << ADSC);                //starting ADC conversion
    while (!(ADCSRA & (1 << ADIF)));  
    return ADCL | (ADCH << 8);            //read ADCL first 
}
// Initialize ATmega328P UART enable TX
void uart_init(void){
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    UBRR0L = 103;
}
// Send ASCII character via UART (single character)
void uart_send(char letter){
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = letter;
}
// Send string of ASCII characters
void send_string(char *stringAddress){
    unsigned char i;
    for (i = 0; i < strlen(stringAddress); i++){
        uart_send(stringAddress[i]);
    }
}
