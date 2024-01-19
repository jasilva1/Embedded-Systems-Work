#include <SPI.h>  //llibrary to utilize SPI bus
#include "avr/io.h" //micro controller library
#include "util/delay.h" //busy and wait functions

#include <DS3231.h> //RTC library, external
DS3231 rtc(SDA, SCL);  //use dedicated SDA and SCL pins 18 and 19
Time t; //initilize time data structure

#include <TMRpcm.h> //audio files library 
TMRpcm audio;

#include <SD.h> //SD card library
#define SDcardPin 10  //connect pin 10 of arduino to cs pin of sd card
#define spkPin 9  //speaker pin
//we'll just put the songs on the SD card the easy way, with an SD card reader and a computer

#include <UltraDistSensor.h>  //ultrasonic sensor library, not sure 
UltraDistSensor sensor1; //initialize sensors
UltraDistSensor sensor2;
float reading1; //initialize sensor readings
float reading2;

//arbitrarily set pins for each LED, look into adafruit neopixel for LED strips
#define ledPin1 0
#define ledPin2 1
#define ledPin3 2
#define ledPin4 3
#define ledPin5 4
#define ledPin6 5
#define ledPin7 6
#define ledPin8 7
#define ledPin9 8
/*LED matrix: 7->4->1, 8->5->2, 9->6->3
  ----------
  7 | 8 | 9
  ----------
  4 | 5 | 6
  ----------
  1 | 2 | 3
  ----------
*/

//#include <Adafruit_SSD1306.h> // OLED display libraries, not necessary to fulfill MDR requirements, can use console
//#include <Adafruit_GFX.h>
//#include <Wire.h>

float BPM;  //beats per minute

int handRegion; //determines where hand is located, 0 = not in region
/*
  4 | 5 | 6
  ----------
  1 | 2 | 3     0
*/

void setup() {
  Serial.begin(115200); //setup serial connection baud rate, higher speed over connection = better
  rtc.begin();  //initialize RTC object 
  rtc.setTime(0, 0, 0);  //Set the time to 00:00:00 (24hr format)

//  audio.speakerPin = spkPin; //assign speaker to speaker pin
  if (!SD.begin(SDcardPin)){ // returns 1 if the card is present
    //SD Card fail
    Serial.println("SD fail");
    return;
  }
  if (!SD.exists("METAL.WAV")){
    Serial.println("file does not exist");
  }
  audio.setVolume(4);  //set volume
  audio.play("METAL.WAV"); //play song of designated full file name and extension "song1.wav"

  sensor1.attach(14,15);//(Trigger pin , Echo pin)
  sensor2.attach(16,17);
//  sensor3.attach(18,19);

  //setup LED pins
  pinMode(ledPin1, OUTPUT); pinMode(ledPin2, OUTPUT); pinMode(ledPin3, OUTPUT); pinMode(ledPin4, OUTPUT); pinMode(ledPin5, OUTPUT); pinMode(ledPin6, OUTPUT); pinMode(ledPin7, OUTPUT); pinMode(ledPin8, OUTPUT); pinMode(ledPin9, OUTPUT); 

  BPM = 120; //initialize BPM, 120BPM = 1 beat every 0.5 seconds
  handRegion = 0; //handRegion 0 means not in any region

}

void loop() {
  t = rtc.getTime();
  // Serial.print(t.sec, DEC); returns time in seconds
  // Serial.print(t.min, DEC); returns time in minutes

  reading1 = sensor1.distanceInCm();  //update readings
  reading2 = sensor2.distanceInCm();
//  reading3 = sensor3.distanceInCm();

  //arbitrary bounds, need to test and figure out accurate region dimensions
  if(reading1 > 100 && reading1 < 999){
    handRegion = 1;
  }
  else if(reading1 > 1000 && reading1 < 2000){
    handRegion = 4;
  }
  else if(reading2 > 100 && reading2 < 999){
    handRegion = 2;
  }
  else if(reading2 > 1000 && reading2 < 2000){
    handRegion = 5;
  }
  /*
  else if(reading3 > 100 && reading3 < 999){
    handRegion = 3;
  }
  else if(reading3 > 1000 && reading3 < 2000){
    handRegion = 6;
  }
  */
  else{
    handRegion = 0;
  }

  //loop keeping lights in time with tempo
  if(t.sec > 0.1){  //note spawned every 2 beats if timerPointer + 1
    rtc.setTime(0, 0, 0);
    updateLights();
    spawnLight();
    collisionCheck();
  }
}

//light up a random LED 7, 8, or 9
void spawnLight(){
  int rng = random(3);
  if(rng == 0){
    digitalWrite(ledPin7,HIGH);
  }
  if(rng == 1){
    digitalWrite(ledPin8,HIGH);
  }
  if(rng == 2){
    digitalWrite(ledPin9,HIGH);
  }
}

//UPDATE ORDER IS IMPORTANT, if you start with upper LEDs subsequent LEDs will update out of time
void updateLights(){
  if(digitalRead(ledPin1) == HIGH){
    digitalWrite(ledPin1,LOW);
  }
  if(digitalRead(ledPin2) == HIGH){
    digitalWrite(ledPin2,LOW);
  }
  if(digitalRead(ledPin3) == HIGH){
    digitalWrite(ledPin3,LOW);
  }

  if(digitalRead(ledPin4) == HIGH){
    digitalWrite(ledPin4,LOW);
    digitalWrite(ledPin1,HIGH);
  }
  if(digitalRead(ledPin5) == HIGH){
    digitalWrite(ledPin5,LOW);
    digitalWrite(ledPin2,HIGH);
  }
  if(digitalRead(ledPin6) == HIGH){
    digitalWrite(ledPin6,LOW);
    digitalWrite(ledPin3,HIGH);
  }

  if(digitalRead(ledPin7) == HIGH){
    digitalWrite(ledPin7,LOW);
    digitalWrite(ledPin4,HIGH);
  }
  if(digitalRead(ledPin8) == HIGH){
    digitalWrite(ledPin8,LOW);
    digitalWrite(ledPin5,HIGH);
  }
  if(digitalRead(ledPin9) == HIGH){
    digitalWrite(ledPin9,LOW);
    digitalWrite(ledPin6,HIGH);
  }
}

//compares current hand position and current HIGH lights
void collisionCheck(){
  if(handRegion == 1 && digitalRead(ledPin1) == HIGH){
    Serial.println("Player collided with a note!");
  }
  else if(handRegion == 2 && digitalRead(ledPin2) == HIGH){
    Serial.println("Player collided with a note!");
  }
  else if(handRegion == 3 && digitalRead(ledPin3) == HIGH){
    Serial.println("Player collided with a note!");
  }
  else{
    Serial.println("Note Dodged!");
  }
}