// Import Dabble Module
#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE

#include <Dabble.h>

// Motor pins
const int motor1a = 13;  // Left motor
const int motor1b = 12;
const int motor2a = 11;  // Right motor
const int motor2b = 10;
const int microbitPin = 9; 

void setup() {
  Dabble.begin(9600);
  pinMode(motor1a, OUTPUT);
  pinMode(motor1b, OUTPUT);
  pinMode(motor2a, OUTPUT);
  pinMode(motor2b, OUTPUT);  
  pinMode(microbitPin, INPUT); // Configured as input
}

void loop() {
  Dabble.processInput();

  // Example Use: If the microbit pin sends a HIGH signal, stop the robot completely
  if (digitalRead(microbitPin) == HIGH) {
    stopMotors();
  } 
  // Otherwise, fallback to standard gamepad controls
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
  else {
    stopMotors();
  }
}

// --- Movement Functions ---

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
