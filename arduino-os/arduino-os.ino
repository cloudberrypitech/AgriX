// Import Dabble Module
#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE

#include <Dabble.h>
#include <Servo.h>

// Setup Servo
Servo driller;
Servo moisture_sensor;

// Motor pins
const int motor1a = 13;  // Left motor
const int motor1b = 12;
const int motor2a = 11;  // Right motor
const int motor2b = 10;
const int microbitPin = 9;
const int sweeprate = 40;
const int moistrate = 90;
int position=0;
int moistpos=0;

void setup() {
  Dabble.begin(9600);
  Serial.begin(9600);
  pinMode(motor1a, OUTPUT);
  pinMode(motor1b, OUTPUT);
  pinMode(motor2a, OUTPUT);
  pinMode(motor2b, OUTPUT);
  pinMode(microbitPin, INPUT); // Configured as input
  driller.attach(6); // PWM Pin
  moisture_sensor.attach(5); // PWM Pin
}

void loop() {
  Dabble.processInput();

  // Otherwise, fallback to standard gamepad controls
  if (GamePad.isStartPressed()) {

  }
  else if (GamePad.isUpPressed()) {
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

  if (GamePad.isSelectPressed()) {
    servoDrill();
  }
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
}

void servoDrill() {
  // ------------------- LOWER MOISTURE SENSOR -------------------- //
  for (moistpos = 0; moistpos <= 90; moistpos += 1) {
    moisture_sensor.write(moistpos);
    delay(15);
  }

  // ------------- DRILLER SERVO ----------------- //
  for (position = 0; position <= 40; position += 1) {
    driller.write(position);
    delay(15);
  }
  for (position = 40; position >= 0; position -= 1) {
    driller.write(position);
    delay(15);
  }

  // -------------- RETURN TO BASE POSITION ------------------ //
  for (moistpos = 90; moistpos >= 0; moistpos -= 1) {
    moisture_sensor.write(moistpos);
    delay(15);
  }
}

void testMotors() {
  // Calibrate all when the start button is pressed

  // Movement functions - Forward, Backward, Left, Right
  goForward(); // Move Forward
  delay(2000); // Continue moving forward for 2 seconds
  goBackward(); // Move Backward
  delay(2000); // Continue moving backward for 2 seconds
  goLeft(); // Turn Left
  delay(2000); // Continue turning left for 2 seconds
  goRight(); // Turn Right
  delay(2000); // Continue turning right for 2 seconds.

  // 
}