#include <ServoTimer2.h>

#define DIRECTION_PIN 2
#define STEP_PIN 3
#define ENABLE_PIN 4
#define MIRROR_PIN 6
#define LIMIT_SWITCH_PIN 7

int pendulumStepCounter = 0;
int pendulumStepCounterMax = 800;


volatile int pendulumCounter = 0;
// Motor is rotating for 60 seconds and steady for 10
int pendulumCounterMax = 10; // seconds
int pendulumValues[] = {1, 0};
int pendulumValueCounters[] = {60, 10};
int pendulumValueIndex = 0;

ServoTimer2 mirrorServo;
volatile int mirrorCounter = 0;

int mirrorValues[] = {85, 120}; // ~degrees // CHANGE THIS VALUES IN PLACE
int mirrorTimes[] = {10, 30}; // seconds // rotate

int mirrorValueIndex = 0;

int maxPendulumIndex = (sizeof(pendulumValues) / sizeof(pendulumValues[0])) - 1;
int maxMirrorIndex = (sizeof(mirrorValues) / sizeof(mirrorValues[0])) - 1;

int dir = 1;
const int stepDurationMicroSec = 500;

void setup() {
  cli();//stop interrupts

  // Declare pins as output:
  pinMode(STEP_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  pinMode(DIRECTION_PIN, OUTPUT);
  pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);

  mirrorServo.attach(MIRROR_PIN);  // attaches the servo on pin 9 to the servo object

  digitalWrite(DIRECTION_PIN, LOW);
  digitalWrite(ENABLE_PIN, HIGH);

    Serial.begin(9600);

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

  int switchState = digitalRead(LIMIT_SWITCH_PIN);

  Serial.println(switchState);
  
  if (switchState) {
    digitalWrite(DIRECTION_PIN, HIGH);
    pendulumStepCounter = 0;
  }
  else if (pendulumStepCounter > pendulumStepCounterMax) {
    digitalWrite(DIRECTION_PIN, LOW);
  }

  // Mirror Update:

  if (mirrorCounter > mirrorTimes[mirrorValueIndex]) {
    mirrorCounter = 0;
    mirrorValueIndex += 1;
    if (mirrorValueIndex > maxMirrorIndex) {
      mirrorValueIndex = 0;
    }
  }

  int targetMirrorPos = map(mirrorValues[mirrorValueIndex], 0, 180, 750, 2250);
  float currMirrorPos = mirrorServo.read();

  while (targetMirrorPos != currMirrorPos) {
    currMirrorPos += abs(targetMirrorPos - currMirrorPos) / (targetMirrorPos - currMirrorPos);
    mirrorServo.write(int(currMirrorPos));
    delay(4);
  }

  // Pendulum Update:

  if (pendulumCounter > pendulumValueCounters[pendulumValueIndex]) {
    pendulumCounter  = 0;
    pendulumValueIndex += 1;

    if (pendulumValueIndex > maxPendulumIndex) {
      pendulumValueIndex = 0;
    }
  }

  bool pendulumOn = pendulumValues[pendulumValueIndex] > 0;
  if (pendulumOn) {
    digitalWrite(ENABLE_PIN, LOW);
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(stepDurationMicroSec);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(stepDurationMicroSec);
    pendulumStepCounter ++;
  }
  else {
    digitalWrite(ENABLE_PIN, HIGH);
    digitalWrite(STEP_PIN, LOW);
  }

  delay(2);

}



ISR(TIMER1_COMPA_vect) { //timer1 interrupt 1Hz
  pendulumCounter += 1;
  mirrorCounter += 1;
}
