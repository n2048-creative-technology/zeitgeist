#include <ServoTimer2.h>

#define VORTEX_PIN 11
#define MIRROR_PIN 6

int vortexCounterMax = 2; // seconds
int vortexValues[] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100}; // %

int mirrorValues[] = {0, 180}; // ~degrees // CHANGE THIS VALUES IN PLACE
int mirrorValueIndex = 0;


ServoTimer2 mirrorServo;

volatile int vortexCounter = 0;
int vortexValueIndex = 0;

volatile int mirrorCounter = 0;
int mirrorCounterMax = 5; // seconds

volatile int counter = 0;

int maxVortexIndex = (sizeof(vortexValues) / sizeof(vortexValues[0])) -1;
int maxMirrorIndex = (sizeof(mirrorValues) / sizeof(mirrorValues[0])) -1;

void setup() {

  cli();//stop interrupts
//  Serial.begin(9600);

  pinMode(VORTEX_PIN, OUTPUT);  
  pinMode(ENABLE_PIN, OUTPUT);
  mirrorServo.attach(MIRROR_PIN);  // attaches the servo on pin 9 to the servo object


  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);

  sei();//allow interrupts
}

void loop () {

  if (counter > 10) {
    counter = 0;
  }

  // Mirror Update:

  if (mirrorCounter > mirrorCounterMax) {
    mirrorCounter = 0;
    mirrorValueIndex += 1;
    if (mirrorValueIndex > maxMirrorIndex) {
      mirrorValueIndex = 0;
    }
  }

  int mirrorPos = map(mirrorValues[mirrorValueIndex], 0, 180, 750, 2250);
  mirrorServo.write(mirrorPos);


  // Vortex Update:
  if (vortexCounter > vortexCounterMax) {
    vortexCounter = 0;
    vortexValueIndex += 1;

    if (vortexValueIndex > maxVortexIndex) {
      vortexValueIndex = 0;
    }
  }

  int vortexSpeed = map(vortexValues[vortexValueIndex], 0, 100, 0, 255);
  analogWrite(VORTEX_PIN, vortexSpeed);

//  Serial.println(vortexSpeed);
  delay(2);
}

ISR(TIMER1_COMPA_vect) { //timer1 interrupt 1Hz
  counter += 1;
  vortexCounter += 1;
  mirrorCounter += 1;
}
