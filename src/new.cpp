#include <QMC5883LCompass.h>
#include <Arduino.h>
/*
 * Version: 2
 * Headband-based compass program, using LEDs to indicate target direction
 * relative to head movement. Uses 8-bit shift register to control LEDs 
 * (LEDs to be swapped for vibration motors once complete).
 */

void setRandomDirection();
void setAdjBearing(int x);
int mod( int x, int y );

QMC5883LCompass compass;

//Shift register variables
int latchPin = 11;
int clockPin = 9;
int dataPin = 12;

int buttonPin = 13; //For random bearing button
//int targetDeg = 180; //Target degrees
int diff;

//Define bearings containing LEDs.
const int DEF_DEG = 180;
const int SEGMENT = 30;

//Adjusted degree bearings of LEDs
int yellowR = DEF_DEG + (2*SEGMENT);
int yellowL = DEF_DEG - (2*SEGMENT);
int redR = DEF_DEG + (3*SEGMENT);
int redL = DEF_DEG - (3*SEGMENT);
int greenL = DEF_DEG - SEGMENT;
int greenR = DEF_DEG + SEGMENT;


//Shift register input bytes for LEDs
const byte RED_R = 1;
const byte YELLOW_R = 2;
const byte GREEN_C = 4;
const byte YELLOW_L = 8;
const byte RED_L = 16;
const byte ERROR_LED = 31;

int dt = 30; //Delay time
int dt2 = 100;

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

    compass.read();
    int x = compass.getAzimuth();

    setAdjBearing(x);
}

void loop() {
    if (digitalRead(buttonPin) == HIGH) {
        setRandomDirection();  
    }
  
  //Update Bearing
    compass.read();
    int deg = compass.getAzimuth();
  
  //Turn on correct LEDs
    byte chosenLED = 0; //Power no LEDs if not within correct direction parameter.
    int x = mod((deg + diff), 360);
      
    if ((x >= greenL) && (x <= greenR)) {
        chosenLED = GREEN_C;
    } else if ((x < greenL) && (x >= yellowL)) {
        chosenLED = YELLOW_R;
    } else if ((x > greenR) && (x <= yellowR)) {
        chosenLED = YELLOW_L;
    } else if ((x < yellowL)) {
        chosenLED = RED_R;
    } else if ((x > yellowR)) {
        chosenLED = RED_L;
    } else {
        chosenLED = ERROR_LED;
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
        delay(dt2);  
    }
        delay(100);
    for (int i=3; i >= 0; i--) {
        byte x = 1 << i;
        digitalWrite(latchPin, LOW);
        shiftOut(dataPin, clockPin, MSBFIRST, x);
        digitalWrite(latchPin, HIGH);
        delay(dt2);  
    }

    //Set new target direction
    int randDegree = random(360);
    setAdjBearing(randDegree);
}

void setAdjBearing(int x) {
    if (x < DEF_DEG) {
        diff = 180 - x;    
    } else {
        diff = x - 180;
    }
    Serial.print("Bearing Set at: ");
    Serial.println(x);
}

int mod( int x, int y ){
    return x < 0 ? ((x+1) % y) + y-1 : x % y;
}