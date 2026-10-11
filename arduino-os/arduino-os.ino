#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE

#include <Dabble.h>
#include <Servo.h>

// Pin Configurations
const byte MOTOR1A = 13;
const byte MOTOR1B = 12;
const byte MOTOR2A = 11;
const byte MOTOR2B = 10;
const byte WARNING_LEFT = 4;
const byte WARNING_RIGHT = 8;
const byte BRAKE_LIGHTS = 9;
const int drill = 6;
const int moistureSensor = A0;
const byte PIN_7 = 7; 

// Timing and Threshold Constants
const int DRILL_START_ANGLE = 0;
const int DRILL_END_ANGLE = 130;
const unsigned long SERVO_STEP_TIME = 15;
const unsigned long WARNING_BLINK_INTERVAL = 500;
const int MOISTURE_THRESHOLD = 1200;
const unsigned long SOW_INTERVAL = 2000;       // 5 seconds between drill cycles
const unsigned long MOISTURE_INTERVAL = 2000;   // Check moisture every 2 seconds

// Global System States
bool readyToRun = false;
bool movingForward = false;
bool lamp = false;
bool warningLightState = false;
bool previousLampState = false;
bool crossWasPressed = false;

// Non-blocking Timing Control Variables
unsigned long lastWarningBlink = 0;
unsigned long lastSowTime = 0;
unsigned long lastMoistureCheck = 0;
unsigned long lastServoStep = 0;

// Servo Sweep State Machine Variables
enum DrillState { IDLE, MOVING_DOWN, WAITING_AT_BOTTOM, MOVING_UP };
DrillState currentDrillState = IDLE;
int currentServoPos = DRILL_START_ANGLE;
unsigned long drillWaitStartTime = 0;
bool sowCycleActive = false; // Tracks if a drilling process is currently running

Servo driller; 

void goForward();
void goBackward();
void goLeft();
void goRight();
void stopMotors();
void warningLamps();
void checkMoisture();
void sweepServoNonBlocking();

void setup() {
  Serial.begin(9600);
  Dabble.begin(9600);

  pinMode(MOTOR1A, OUTPUT);
  pinMode(MOTOR1B, OUTPUT);
  pinMode(MOTOR2A, OUTPUT);
  pinMode(MOTOR2B, OUTPUT);

  pinMode(WARNING_LEFT, OUTPUT);
  pinMode(WARNING_RIGHT, OUTPUT);
  pinMode(BRAKE_LIGHTS, OUTPUT);
  pinMode(PIN_7, OUTPUT);
  
  pinMode(moistureSensor, INPUT);

  stopMotors();

  Serial.println(F("AgriX:: Moisture sensing seed system started."));
  Serial.println(F("Waiting for Dabble / HC-05..."));

  digitalWrite(PIN_7, HIGH);

  // Attach global instance
  driller.attach(drill);
  driller.write(DRILL_START_ANGLE);

  // Initial check
  checkMoisture();
  lastSowTime = millis();
}

void loop() {
  Dabble.processInput();
  unsigned long currentMillis = millis();

  // Periodic Soil Check (Every 2 seconds)
  if (currentMillis - lastMoistureCheck >= MOISTURE_INTERVAL) {
    lastMoistureCheck = currentMillis;
    checkMoisture();
  }

  // Cross Button Toggle Logic for Hazard Lights
  bool crossPressed = GamePad.isCrossPressed();
  if (crossPressed && !crossWasPressed) {
    lamp = !lamp;
  }
  crossWasPressed = crossPressed;

  // Run Robot Movement Controls only if Soil is "Ready"
  if (readyToRun) {
    movingForward = false;
    
    // Check if a drilling cycle is active
    if (sowCycleActive) {
      // Force motors to stop so the robot stands still while drilling
      stopMotors();
    } else {
      // Normal driving controls when NOT drilling
      if (GamePad.isStartPressed()) {
        stopMotors();
      } else if (GamePad.isUpPressed()) {
        goForward();
        movingForward = true;
        digitalWrite(BRAKE_LIGHTS, LOW);
      } else if (GamePad.isDownPressed()) {
        goBackward();
        digitalWrite(BRAKE_LIGHTS, LOW);
      } else if (GamePad.isLeftPressed()) {
        goLeft();
        digitalWrite(BRAKE_LIGHTS, LOW);
      } else if (GamePad.isRightPressed()) {
        goRight();
        digitalWrite(BRAKE_LIGHTS, LOW); 
      } else {
        stopMotors();
      }
    }
    
    sweepServoNonBlocking();
    warningLamps();
    
  } else {
    // If soil condition changes or not ready, halt all operations safely
    stopMotors();
  }
}

void goForward() {
  digitalWrite(MOTOR1A, HIGH);
  digitalWrite(MOTOR1B, LOW);
  digitalWrite(MOTOR2A, HIGH);
  digitalWrite(MOTOR2B, LOW);
}

void goBackward() {
  digitalWrite(MOTOR1A, LOW);
  digitalWrite(MOTOR1B, HIGH);
  digitalWrite(MOTOR2A, LOW);
  digitalWrite(MOTOR2B, HIGH);
}

void goLeft() {
  digitalWrite(MOTOR1A, HIGH);
  digitalWrite(MOTOR1B, LOW);
  digitalWrite(MOTOR2A, LOW);
  digitalWrite(MOTOR2B, HIGH);
}

void goRight() {
  digitalWrite(MOTOR1A, LOW);
  digitalWrite(MOTOR1B, HIGH);
  digitalWrite(MOTOR2A, HIGH);
  digitalWrite(MOTOR2B, LOW);
}

void stopMotors() {
  digitalWrite(MOTOR1A, LOW);
  digitalWrite(MOTOR1B, LOW);
  digitalWrite(MOTOR2A, LOW);
  digitalWrite(MOTOR2B, LOW);
  digitalWrite(BRAKE_LIGHTS, HIGH);
}

void warningLamps() {
  unsigned long currentMillis = millis();
  
  if (lamp && !previousLampState) {
    warningLightState = true;
    lastWarningBlink = currentMillis;
  }

  if (lamp && (currentMillis - lastWarningBlink >= WARNING_BLINK_INTERVAL)) {
    lastWarningBlink = currentMillis;
    warningLightState = !warningLightState;
  }

  if (!lamp) {
    warningLightState = false;
  }

  previousLampState = lamp;

  digitalWrite(WARNING_LEFT, warningLightState ? HIGH : LOW);
  digitalWrite(WARNING_RIGHT, warningLightState ? HIGH : LOW);
}

void checkMoisture() {
  int moisture = analogRead(moistureSensor);
  Serial.print(F("Moisture Level: "));
  Serial.println(moisture);
  
  if (moisture < MOISTURE_THRESHOLD) {
    if (!readyToRun) Serial.println(F("Status: Ready to Run"));
    readyToRun = true;
  } else {
    if (readyToRun) Serial.println(F("Status: Soil wet/wrong threshold - Stopped"));
    readyToRun = false;
  }
}

void sweepServoNonBlocking() {
  unsigned long currentMillis = millis();

  switch (currentDrillState) {
    case IDLE:
      // Trigger new sow cycle if moving forward AND time interval has passed
      if (movingForward && (currentMillis - lastSowTime >= SOW_INTERVAL)) {
        Serial.println(F("Time to Sow - Lowering Drill..."));
        currentDrillState = MOVING_DOWN;
        sowCycleActive = true; // Lock the cycle so it finishes automatically
        lastServoStep = currentMillis;
      }
      break;

    case MOVING_DOWN:
      if (currentMillis - lastServoStep >= SERVO_STEP_TIME) {
        lastServoStep = currentMillis;
        currentServoPos++;
        driller.write(currentServoPos);
        
        if (currentServoPos >= DRILL_END_ANGLE) {
          currentDrillState = WAITING_AT_BOTTOM;
          drillWaitStartTime = currentMillis;
        }
      }
      break;

    case WAITING_AT_BOTTOM:
      // Hold at bottom position for 1 second
      if (currentMillis - drillWaitStartTime >= 1000) {
        currentDrillState = MOVING_UP;
        lastServoStep = currentMillis;
      }
      break;

    case MOVING_UP:
      if (currentMillis - lastServoStep >= SERVO_STEP_TIME) {
        lastServoStep = currentMillis;
        currentServoPos--;
        driller.write(currentServoPos);
        
        if (currentServoPos <= DRILL_START_ANGLE) {
          Serial.println(F("Sow Cycle Complete. Resuming Drive Mode."));
          currentDrillState = IDLE;
          sowCycleActive = false;       // Release the lock so you can drive again
          lastSowTime = currentMillis;   // Reset the 5-second interval clock
        }
      }
      break;
  }
}
