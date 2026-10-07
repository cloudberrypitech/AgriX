/*
   AgriX Robot
   Arduino Uno / Nano
   HC-05 Bluetooth + Dabble GamePad
   Two DC motors + drilling servo + moisture-sensor servo

   Dabble:
   - UP       = Forward
   - DOWN     = Backward
   - LEFT     = Turn left
   - RIGHT    = Turn right
   - SELECT   = Run drilling/moisture sequence
   - START    = Stop motors

   IMPORTANT:
   Install the Dabble library by STEMpedia.
*/

#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE

#include <Dabble.h>
#include <Servo.h>

// =====================================================
// PIN DEFINITIONS
// =====================================================

// Motor 1 - Left
const byte motor1a = 13;
const byte motor1b = 12;

// Motor 2 - Right
const byte motor2a = 11;
const byte motor2b = 10;

// Micro:bit input
const byte microbitPin = 9;

// Servos
const byte drillerPin = 6;
const byte moistureServoPin = 5;

// =====================================================
// SERVO SETTINGS
// =====================================================

const int DRILL_START_ANGLE = 0;
const int DRILL_END_ANGLE   = 40;

const int MOISTURE_UP_ANGLE   = 0;
const int MOISTURE_DOWN_ANGLE = 90;

const unsigned long SERVO_STEP_TIME = 15;

// =====================================================
// SERVO OBJECTS
// =====================================================

Servo driller;
Servo moisture_sensor;

// =====================================================
// DRILLING STATE MACHINE
// =====================================================

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

// Prevent repeatedly starting the sequence while SELECT
// is being held down.
bool selectWasPressed = false;


// =====================================================
// SETUP
// =====================================================

void setup() {

  // Start serial monitor for debugging
  Serial.begin(9600);

  // Start Dabble / HC-05
  Dabble.begin(9600);

  // Motor pins
  pinMode(motor1a, OUTPUT);
  pinMode(motor1b, OUTPUT);
  pinMode(motor2a, OUTPUT);
  pinMode(motor2b, OUTPUT);

  // Micro:bit input
  pinMode(microbitPin, INPUT);

  // Attach servos
  driller.attach(drillerPin);
  moisture_sensor.attach(moistureServoPin);

  // Initial servo positions
  driller.write(DRILL_START_ANGLE);
  moisture_sensor.write(MOISTURE_UP_ANGLE);

  // Make sure motors are stopped
  stopMotors();

  Serial.println("AgriX started.");
  Serial.println("Waiting for Dabble / HC-05...");
}


// =====================================================
// MAIN LOOP
// =====================================================

void loop() {

  // VERY IMPORTANT:
  // This must run continuously so Dabble receives
  // commands from the phone.
  Dabble.processInput();


  // ===================================================
  // MOTOR CONTROL
  // ===================================================

  if (GamePad.isUpPressed()) {

    goForward();

  }
  else if (GamePad.isDownPressed()) {

    goBackward();

  }
  else if (GamePad.isLeftPressed()) {

    goLeft();

  }
  else if (GamePad.isRightPressed()) {

    goRight();

  }
  else {

    // Stop when no directional button is pressed.
    stopMotors();
  }


  // ===================================================
  // START BUTTON
  // ===================================================

  if (GamePad.isStartPressed()) {

    stopMotors();
  }


  // ===================================================
  // SELECT BUTTON
  // ===================================================

  bool selectPressed = GamePad.isSelectPressed();

  // Start sequence only when SELECT changes
  // from released -> pressed.
  if (selectPressed && !selectWasPressed) {

    if (drillState == DRILL_IDLE) {

      startDrillingSequence();
    }
  }

  selectWasPressed = selectPressed;


  // ===================================================
  // RUN DRILLING SEQUENCE
  // ===================================================

  updateDrillingSequence();
}


// =====================================================
// MOVE FORWARD
// =====================================================

void goForward() {

  digitalWrite(motor1a, HIGH);
  digitalWrite(motor1b, LOW);

  digitalWrite(motor2a, HIGH);
  digitalWrite(motor2b, LOW);
}


// =====================================================
// MOVE BACKWARD
// =====================================================

void goBackward() {

  digitalWrite(motor1a, LOW);
  digitalWrite(motor1b, HIGH);

  digitalWrite(motor2a, LOW);
  digitalWrite(motor2b, HIGH);
}


// =====================================================
// TURN LEFT
// =====================================================

void goLeft() {

  digitalWrite(motor1a, HIGH);
  digitalWrite(motor1b, LOW);

  digitalWrite(motor2a, LOW);
  digitalWrite(motor2b, HIGH);
}


// =====================================================
// TURN RIGHT
// =====================================================

void goRight() {

  digitalWrite(motor1a, LOW);
  digitalWrite(motor1b, HIGH);

  digitalWrite(motor2a, HIGH);
  digitalWrite(motor2b, LOW);
}


// =====================================================
// STOP MOTORS
// =====================================================

void stopMotors() {

  digitalWrite(motor1a, LOW);
  digitalWrite(motor1b, LOW);

  digitalWrite(motor2a, LOW);
  digitalWrite(motor2b, LOW);
}


// =====================================================
// START DRILLING SEQUENCE
// =====================================================

void startDrillingSequence() {

  // Stop robot before operating the mechanism.
  stopMotors();

  Serial.println("Drilling sequence started.");

  position = DRILL_START_ANGLE;
  moistpos = MOISTURE_UP_ANGLE;

  drillState = MOISTURE_DOWN;

  lastServoMove = millis();
}


// =====================================================
// NON-BLOCKING DRILLING SEQUENCE
// =====================================================

void updateDrillingSequence() {

  // Nothing to do
  if (drillState == DRILL_IDLE) {
    return;
  }


  // Wait until it is time for the next servo step.
  if (millis() - lastServoMove < SERVO_STEP_TIME) {
    return;
  }

  lastServoMove = millis();


  // ===================================================
  // LOWER MOISTURE SENSOR
  // ===================================================

  if (drillState == MOISTURE_DOWN) {

    moisture_sensor.write(moistpos);

    moistpos++;

    if (moistpos > MOISTURE_DOWN_ANGLE) {

      position = DRILL_START_ANGLE;

      drillState = DRILL_DOWN;
    }

    return;
  }


  // ===================================================
  // LOWER DRILL
  // ===================================================

  if (drillState == DRILL_DOWN) {

    driller.write(position);

    position++;

    if (position > DRILL_END_ANGLE) {

      position = DRILL_END_ANGLE;

      drillState = DRILL_UP;
    }

    return;
  }


  // ===================================================
  // RAISE DRILL
  // ===================================================

  if (drillState == DRILL_UP) {

    driller.write(position);

    position--;

    if (position < DRILL_START_ANGLE) {

      position = DRILL_START_ANGLE;

      drillState = MOISTURE_UP;
    }

    return;
  }


  // ===================================================
  // RAISE MOISTURE SENSOR
  // ===================================================

  if (drillState == MOISTURE_UP) {

    moisture_sensor.write(moistpos);

    moistpos--;

    if (moistpos < MOISTURE_UP_ANGLE) {

      moistpos = MOISTURE_UP_ANGLE;

      moisture_sensor.write(moistpos);

      drillState = DRILL_IDLE;

      Serial.println("Drilling sequence complete.");
    }

    return;
  }
}


// =====================================================
// MOTOR TEST
// =====================================================

void testMotors() {

  goForward();
  delay(2000);

  stopMotors();
  delay(500);

  goBackward();
  delay(2000);

  stopMotors();
  delay(500);

  goLeft();
  delay(2000);

  stopMotors();
  delay(500);

  goRight();
  delay(2000);

  stopMotors();
}
