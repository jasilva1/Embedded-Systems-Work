// Josh Silva
// ECE 231 Lab 1
// 03/07/23

#include <avr/io.h> 
#include <util/delay.h> //defines _delay_ms

#define DELAY 100 //100ms delay, Frquency: 5Hz 


/*
D4 = Button 1
D5 = Button 2
D3 = Button 3
*/

int main(void){

    DDRD =1<<PORTD6|1<<PORTD7; // Outputs D6 & D7 
    PORTD = 1<<PORTD4|1<<PORTD3|1<<PORTD5; // setting D4 D5 D3
    while(1){

        if (((PIND & ( 1<<PORTD4)) == 0) && ((PIND & ( 1<<PORTD5)) != 0) && ((PIND & ( 1<<PORTD3)) != 0))   { //checking that D4 was selected
            PORTD |= 1<<PORTD6; //sets pin to high (LED On)
            _delay_ms(MYDELAY);
            PORTD &= ~ (1<<PORTD6); // sets pin to low (LED off) 
            _delay_ms(MYDELAY); // Flashes LED
        }

        else if (((PIND & ( 1<<PORTD5)) == 0) && ((PIND & ( 1<<PORTD4)) != 0) && ((PIND & ( 1<<PORTD3)) != 0)){ 
            PORTD |= 1<<PORTD7; //sets pin to high (LED On)
            _delay_ms(MYDELAY);
            PORTD &= ~ (1<<PORTD7); // Sets pin to low (LED Off) 
            _delay_ms(MYDELAY); // Flashes LED
        }
         else if  (((PIND & ( 1<<PORTD3)) == 0) && ((PIND & ( 1<<PORTD5)) != 0) && ((PIND & ( 1<<PORTD5)) != 0)){
            PORTD |= 1<<PORTD7 ;
            PORTD |= 1<<PORTD6 ; 
            _delay_ms(MYDELAY); 
            PORTD &= ~ (1<<PORTD6) ;
            PORTD &= ~ (1<<PORTD7) ; 
             _delay_ms(MYDELAY);

         } 

    } 
    return 0; 
} 
