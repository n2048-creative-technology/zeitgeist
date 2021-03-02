#define DIRECTION_PIN 2
#define STEP_PIN 3
#define ENABLE_PIN 4
#define LIMIT_SWITCH_PIN 7

//driver settings:
//Micro step: 32
//Pulse/rev 800
//s1 off
//s2 off
//s3 off
//
//current: 1A
//PK current: 1.2A
//s4 on
//s5 off
//s6 on

int counter = 0;
int dir = 1;
int maxCount = 50;
int switchPos = 0;
int eps = 100; // steps to wait to re-activate switch;

int  isHoming = LOW;
int atHome = LOW;
int isCentered = LOW;

const int stepDurationMicroSec = 9000;

volatile int stepperCounter = 0;
// Motor is rotating for 5 minutes seconds and steady for 10 seconds
int stepperCounterMax = 10; // seconds
int stepperValues[] = {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
int stepperValueIndex = 0;

int maxStepperIndex = (sizeof(stepperValues) / sizeof(stepperValues[0])) - 1;

volatile int homingCounter = 0; // seconds
int homingCounterMax = 1800; // seconds ==> recenter every half hour

void setup() {
  cli();//stop interrupts

  // Declare pins as output:
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIRECTION_PIN, OUTPUT);
  pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);

  //  Serial.begin(9600);
  isHoming = LOW;

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

void homing() {
  homingCounter = 0;
  if (isHoming == LOW) {
    atHome = LOW;
    digitalWrite(DIRECTION_PIN, LOW); // go back to origin;
    switchPos = 0;
    //    Serial.println("HOMING...");
    isHoming = HIGH;
    isCentered = LOW;
  }
  int switchState = digitalRead(LIMIT_SWITCH_PIN);

  if (switchState == HIGH  &&
      atHome == HIGH &&
      switchPos == 0 &&
      counter > eps) {
    switchPos = counter;
    counter = 0;

    //    Serial.print("switchPos = ");
    //    Serial.println(switchPos);
    //    Serial.print("END POSITION FOUND AT ");
    //    Serial.println(switchPos);

    digitalWrite(DIRECTION_PIN, LOW); // start counting up to the next switch

  }

  if (switchPos != 0 && counter >= switchPos / 2 ) {
    isCentered = HIGH;
    //    Serial.print("CENTERED at ");
    //    Serial.println(counter);
    counter == 0;
  }

  if (switchState == HIGH
      && atHome == LOW) {

    atHome = HIGH;
    //    Serial.println("HOME POSITION FOUND.");
    //    Serial.print("switchPos = ");
    //    Serial.println(switchPos);

    counter = 0;
    digitalWrite(DIRECTION_PIN, HIGH); // start counting up to the next switch
  }

  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(stepDurationMicroSec);
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(stepDurationMicroSec);
  counter ++;
}

void loop() {

  if (homingCounter > homingCounterMax) {
    // restart homing:
    isHoming = LOW;
    atHome = LOW;
    isCentered = LOW;
    homingCounter = 0;
    switchPos = 0;
    counter = 0;
    digitalWrite(ENABLE_PIN, HIGH);
  }

  //  Serial.println(counter);
  if (isCentered == LOW) {
    homing();
  }
  else if (isHoming == HIGH) {
    //    Serial.println("HOMING DONE!");
    isHoming = LOW;
    maxCount = switchPos / 3;
  }
  else {

    if (stepperCounter > stepperCounterMax) {
      stepperCounter   = 0;
      stepperValueIndex += 1;

      if (stepperValueIndex > maxStepperIndex) {
        stepperValueIndex = 0;
      }
    }

    bool stepperOn = stepperValues[stepperValueIndex] > 0;
    if (stepperOn) {
      digitalWrite(ENABLE_PIN, HIGH);


      // These four lines result in 1 step:
      digitalWrite(STEP_PIN, HIGH);
      delayMicroseconds(stepDurationMicroSec);
      digitalWrite(STEP_PIN, LOW);
      delayMicroseconds(stepDurationMicroSec);
      counter ++;
      if (counter >= maxCount) {
        if (dir == 0 ) {
          dir = 1;
          digitalWrite(DIRECTION_PIN, LOW);
        }
        else {
          dir = 0;
          digitalWrite(DIRECTION_PIN, HIGH);
        }
        counter = 0;
      }
    }
    else {
      digitalWrite(ENABLE_PIN, LOW);
      digitalWrite(STEP_PIN, LOW);
      digitalWrite(DIRECTION_PIN, LOW);
    }

  }
}



ISR(TIMER1_COMPA_vect) { //timer1 interrupt 1Hz
  stepperCounter += 1;
  homingCounter += 1;
}
