// Import Dabble Module
#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <Dabble.h>
// Variables
const int motor1a=13; // Left Motor
const int motor1b=12;
const int motor2a=11; // Right Motor
const int motor2b=10;
const int microbit=9; // Analog input to allow multiple charges for multiple values
void setup() {
  Dabble.begin(9600);
  pinMode(motor1a, OUTPUT);
  pinMode(motor1b, OUTPUT);
  pinMode(motor2a, OUTPUT);
  pinMode(motor2b, OUTPUT);
  pinMode(microbit, INPUT);
}

void loop() {
  Dabble.processInptut();
  if (GamePad.isUpPressed()) {
    goForward();
  }
  if (GamePad.isDownPressed()) {
    goBackward();
  }
  if (GamePad.isLeftPressed()) {
    goLeft();
  }
  if (GamePad.isRightPressed()) {
    goRight()
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