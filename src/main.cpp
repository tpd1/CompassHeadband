#include <Arduino.h>
#include <QMC5883LCompass.h>
/*
 * Version: 1.1
 * Headband-based compass program, using LEDs to indicate target direction
 * relative to head movement. Uses 8-bit shift register to control LEDs 
 * (LEDs to be swapped for vibration motors once complete).
 */
 
QMC5883LCompass compass;
void setBounds();
void setRandomDirection();

//Shift register variables
int latchPin = 11;
int clockPin = 9;
int dataPin = 12;

int buttonPin = 13; //For random bearing button
byte targetB = 0; //Target bearing, 0 = North, 15 = South.

//Define bearings containing LEDs.
byte yellowR;
byte yellowL;
byte redR;
byte redL;

//Shift register input bytes for LEDs
const byte RED_R = 1;
const byte YELLOW_R = 2;
const byte GREEN_C = 4;
const byte YELLOW_L = 8;
const byte RED_L = 16;
const byte ERROR_LED = 31;

int dt = 50; //Delay time

void setup() {
  Serial.begin(9600);
  compass.init();
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(buttonPin, INPUT);

  //Test LEDs - turn all on for 1s.
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, 31);
  digitalWrite(latchPin, HIGH);
  delay(1000);

  //Set current compass bearing as target.
  compass.read();
  int a = compass.getAzimuth();
  targetB = compass.getBearing(a); //bearing of target (0-15)
  setBounds();
}

void loop() {
  if (digitalRead(buttonPin) == HIGH) {
    setRandomDirection();  
  }
  
  //Update Bearing
  compass.read();
  int a = compass.getAzimuth();
  byte bearing = compass.getBearing(a);
  Serial.println(a);
  
  //Turn on correct LEDs
  byte chosenLED = 0; //Power no LEDs if not within correct direction parameter.
  if (bearing == targetB) {
    chosenLED = GREEN_C;
  } else if (bearing == yellowL) {
     chosenLED = YELLOW_L;
  } else if (bearing == yellowR) {
    chosenLED = YELLOW_R;
  } else if (bearing == redL) {
    chosenLED = RED_L;
  } else if (bearing == redR) {
    chosenLED = RED_R;
  } else {
    Serial.println("No LED Chosen");
  }

   digitalWrite(latchPin, LOW);
   shiftOut(dataPin, clockPin, MSBFIRST, chosenLED);
   digitalWrite(latchPin, HIGH);
      
   delay(dt);
}

void setRandomDirection() {
  //Loop up and down lights to indicate button has been pressed
  for (int i=0; i < 6; i++) {
    byte x = 1 << i;
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, x);
    digitalWrite(latchPin, HIGH);
    delay(dt);  
  }
    delay(100);
  for (int i=3; i >= 0; i--) {
    byte x = 1 << i;
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, x);
    digitalWrite(latchPin, HIGH);
    delay(dt);  
  }

  //Set new target direction
  byte randBearing = random(16);
  targetB = randBearing;
  setBounds();
}

void setBounds() {
  //If target is 0 or 15, loop around for yellow / red lights
  if (targetB + 1 > 15) {
    yellowL = 0;
  } else {
    yellowL = targetB + 1;
  }

  if (targetB - 1 < 0) {
    yellowR = 15;
  } else {
    yellowR = targetB - 1;
  }

  if (targetB + 2 > 15) {
    redL = targetB - 14;
  } else {
    redL = targetB + 2;
  }

  if (targetB - 2 < 0) {
    redR = targetB + 14;
  } else {
    redR = targetB - 2;
  }
}