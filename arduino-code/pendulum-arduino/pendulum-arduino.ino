#include <ServoTimer2.h>

#define DIRECTION_PIN 2
#define STEP_PIN 3
#define ENABLE_PIN 4
#define MIRROR_PIN 6

volatile int pendulumCounter = 0;
// Motor is rotating for 60 seconds and steady for 10
int pendulumCounterMax = 10; // seconds
int pendulumValues[] = {0, 1, 1, 1, 1, 1, 1};
int pendulumValueIndex = 0;

ServoTimer2 mirrorServo;
volatile int mirrorCounter = 0;
int mirrorCounterMax = 5; // seconds
int mirrorValues[] = {0, 180}; // ~degrees // CHANGE THIS VALUES IN PLACE
int mirrorValueIndex = 0;

int maxPendulumIndex = (sizeof(pendulumValues) / sizeof(pendulumValues[0])) - 1;
int maxMirrorIndex = (sizeof(mirrorValues) / sizeof(mirrorValues[0])) - 1;

int dir = 1;
const int stepDurationMicroSec = 9000;

void setup() {
  cli();//stop interrupts

  // Declare pins as output:
  pinMode(STEP_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  pinMode(DIRECTION_PIN, OUTPUT);

  mirrorServo.attach(MIRROR_PIN);  // attaches the servo on pin 9 to the servo object

  digitalWrite(DIRECTION_PIN, LOW);
  digitalWrite(ENABLE_PIN, LOW);

  //  Serial.begin(9600);

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

void loop() {

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


  // Pendulum Update:

  if (pendulumCounter > pendulumCounterMax) {
    pendulumCounter  = 0;
    pendulumValueIndex += 1;

    if (pendulumValueIndex > maxPendulumIndex) {
      pendulumValueIndex = 0;
    }
  }

  bool pendulumOn = pendulumValues[pendulumValueIndex] > 0;
  if (pendulumOn) {
    digitalWrite(ENABLE_PIN, HIGH);
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(stepDurationMicroSec);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(stepDurationMicroSec);
  }
  else {
    digitalWrite(ENABLE_PIN, LOW);
    digitalWrite(STEP_PIN, LOW);
  }

  delay(2);

}



ISR(TIMER1_COMPA_vect) { //timer1 interrupt 1Hz
  pendulumCounter += 1;
  mirrorCounter += 1;
}
