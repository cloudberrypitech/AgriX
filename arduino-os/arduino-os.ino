#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE

#include <Dabble.h>
#include <Servo.h>

const byte motor1a = 13;
const byte motor1b = 12;
const byte motor2a = 11;
const byte motor2b = 10;

const byte microbitPin = 9;
const byte drillerPin = 6;
const byte moistureServoPin = 5;

const byte WARNING_LEFT = 4;
const byte WARNING_RIGHT = 8;
const byte BRAKE_LIGHTS = 9;

const int DRILL_START_ANGLE = 0;
const int DRILL_END_ANGLE = 180;
const int MOISTURE_UP_ANGLE = 0;
const int MOISTURE_DOWN_ANGLE = 90;

const unsigned long SERVO_STEP_TIME = 15;
const unsigned long WARNING_BLINK_INTERVAL = 500;

Servo driller;
Servo moisture_sensor;

enum DrillState {
  DRILL_IDLE,
  MOISTURE_DOWN,
  DRILL_DOWN,
  DRILL_UP,
  MOISTURE_UP
};

DrillState drillState = DRILL_IDLE;

int position = 0;
int moistpos = 0;

unsigned long lastServoMove = 0;
unsigned long lastWarningBlink = 0;

bool lamp = false;
bool warningLightState = false;
bool previousLampState = false;
bool selectWasPressed = false;
bool crossWasPressed = false;

void goForward();
void goBackward();
void goLeft();
void goRight();
void stopMotors();
void startDrillingSequence();
void updateDrillingSequence();
void warningLamps();

void setup() {
  Serial.begin(9600);
  Dabble.begin(9600);

  pinMode(motor1a, OUTPUT);
  pinMode(motor1b, OUTPUT);
  pinMode(motor2a, OUTPUT);
  pinMode(motor2b, OUTPUT);

  pinMode(microbitPin, INPUT);
  pinMode(WARNING_LEFT, OUTPUT);
  pinMode(WARNING_RIGHT, OUTPUT);
  pinMode(BRAKE_LIGHTS, OUTPUT);

  driller.attach(drillerPin);
  moisture_sensor.attach(moistureServoPin);

  driller.write(DRILL_START_ANGLE);
  moisture_sensor.write(MOISTURE_UP_ANGLE);

  stopMotors();

  Serial.println("AgriX started.");
  Serial.println("Waiting for Dabble / HC-05...");
}

void loop() {
  Dabble.processInput();
  analogWrite(BRAKE_LIGHTS, 100);

  bool crossPressed = GamePad.isCrossPressed();
  if (crossPressed && !crossWasPressed) {
    lamp = !lamp;
  }
  crossWasPressed = crossPressed;

  bool selectPressed = GamePad.isSelectPressed();
  if (selectPressed && !selectWasPressed && drillState == DRILL_IDLE) {
    startDrillingSequence();
  }
  selectWasPressed = selectPressed;

  if (drillState != DRILL_IDLE || GamePad.isStartPressed()) {
    stopMotors();
  } else if (GamePad.isUpPressed()) {
    goForward();
    digitalWrite(BRAKE_LIGHTS, 30);
  } else if (GamePad.isDownPressed()) {
    goBackward();
    digitalWrite(BRAKE_LIGHTS, 30);
  } else if (GamePad.isLeftPressed()) {
    goLeft();
    digitalWrite(BRAKE_LIGHTS, 30);
  } else if (GamePad.isRightPressed()) {
    goRight();
    digitalWrite(BRAKE_LIGHTS, 30);
  } else {
    stopMotors();
  }

  updateDrillingSequence();
  warningLamps();
}

void goForward() {
  digitalWrite(motor1a, HIGH);
  digitalWrite(motor1b, LOW);
  digitalWrite(motor2a, HIGH);
  digitalWrite(motor2b, LOW);
}

void goBackward() {
  digitalWrite(motor1a, LOW);
  digitalWrite(motor1b, HIGH);
  digitalWrite(motor2a, LOW);
  digitalWrite(motor2b, HIGH);
}

void goLeft() {
  digitalWrite(motor1a, HIGH);
  digitalWrite(motor1b, LOW);
  digitalWrite(motor2a, LOW);
  digitalWrite(motor2b, HIGH);
}

void goRight() {
  digitalWrite(motor1a, LOW);
  digitalWrite(motor1b, HIGH);
  digitalWrite(motor2a, HIGH);
  digitalWrite(motor2b, LOW);
}

void stopMotors() {
  digitalWrite(motor1a, LOW);
  digitalWrite(motor1b, LOW);
  digitalWrite(motor2a, LOW);
  digitalWrite(motor2b, LOW);
  digitalWrite(BRAKE_LIGHTS, HIGH);
}

void startDrillingSequence() {
  stopMotors();

  Serial.println("Drilling sequence started.");

  position = DRILL_START_ANGLE;
  moistpos = MOISTURE_UP_ANGLE;
  drillState = MOISTURE_DOWN;
  lastServoMove = millis();
}

void updateDrillingSequence() {
  if (drillState == DRILL_IDLE) {
    return;
  }

  if (millis() - lastServoMove < SERVO_STEP_TIME) {
    return;
  }

  lastServoMove = millis();

  if (drillState == MOISTURE_DOWN) {
    moisture_sensor.write(moistpos);
    moistpos++;

    if (moistpos > MOISTURE_DOWN_ANGLE) {
      position = DRILL_START_ANGLE;
      drillState = DRILL_DOWN;
    }
    return;
  }

  if (drillState == DRILL_DOWN) {
    driller.write(position);
    position++;

    if (position > DRILL_END_ANGLE) {
      position = DRILL_END_ANGLE;
      drillState = DRILL_UP;
    }
    return;
  }

  if (drillState == DRILL_UP) {
    driller.write(position);
    position--;

    if (position < DRILL_START_ANGLE) {
      position = DRILL_START_ANGLE;
      drillState = MOISTURE_UP;
    }
    return;
  }

  if (drillState == MOISTURE_UP) {
    moisture_sensor.write(moistpos);
    moistpos--;

    if (moistpos < MOISTURE_UP_ANGLE) {
      moistpos = MOISTURE_UP_ANGLE;
      moisture_sensor.write(moistpos);
      drillState = DRILL_IDLE;
      Serial.println("Drilling sequence complete.");
    }
  }
}

void warningLamps() {
  if (lamp && !previousLampState) {
    warningLightState = true;
    lastWarningBlink = millis();
  }

  if (lamp && millis() - lastWarningBlink >= WARNING_BLINK_INTERVAL) {
    lastWarningBlink = millis();
    warningLightState = !warningLightState;
  }

  if (!lamp) {
    warningLightState = false;
  }

  previousLampState = lamp;

  digitalWrite(WARNING_LEFT, warningLightState ? HIGH : LOW);
  digitalWrite(WARNING_RIGHT, warningLightState ? HIGH : LOW);
}
