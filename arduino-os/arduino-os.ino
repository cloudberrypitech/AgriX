#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#define INCLUDE_TERMINAL_MODULE

#include <Dabble.h>
#include <Servo.h>
#include <EEPROM.h> 

// EEPROM Memory Address Config
const int EEPROM_THRES_ADDR = 0; 

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
int moistureThreshold = 400; 
const unsigned long SOW_INTERVAL = 2000;       

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
unsigned long lastServoStep = 0;

// Servo Sweep State Machine Variables
enum DrillState { IDLE, MOVING_DOWN, WAITING_AT_BOTTOM, MOVING_UP };
DrillState currentDrillState = IDLE;
int currentServoPos = DRILL_START_ANGLE;
unsigned long drillWaitStartTime = 0;
bool sowCycleActive = false; 

Servo driller; 

// Software Reset Function 
void (* resetFunc) (void) = 0; 

void goForward();
void goBackward();
void goLeft();
void goRight();
void stopMotors();
void warningLamps();
void checkMoisture();
void sweepServoNonBlocking();
void processTerminalCommands();

void setup() {
  Serial.begin(9600);
  Dabble.begin(9600);

  // Read saved threshold value from permanent memory on startup
  int savedThreshold;
  EEPROM.get(EEPROM_THRES_ADDR, savedThreshold);
  
  if (savedThreshold > 0 && savedThreshold < 1024) {
    moistureThreshold = savedThreshold;
  } else {
    EEPROM.put(EEPROM_THRES_ADDR, moistureThreshold); 
  }

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

  driller.attach(drill);
  driller.write(DRILL_START_ANGLE);

  // Check moisture ONLY here during startup
  checkMoisture();
  lastSowTime = millis();
}

void loop() {
  Dabble.processInput();

  // Process Terminal Module inputs
  processTerminalCommands();

  // Cross Button Toggle Logic for Hazard Lights
  bool crossPressed = GamePad.isCrossPressed();
  if (crossPressed && !crossWasPressed) {
    lamp = !lamp;
  }
  crossWasPressed = crossPressed;

  // Run Robot Movement Controls only if Soil is "Ready"
  if (readyToRun) {
    movingForward = false;
    
    if (sowCycleActive) {
      stopMotors();
    } else {
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
    stopMotors();
  }
}

void processTerminalCommands() {
  if (Terminal.available()) {
    String input = Terminal.readString();
    input.trim(); 

    if (input.equalsIgnoreCase("status")) {
      int currentMoisture = analogRead(moistureSensor);
      
      // Live evaluation for the text status report
      String liveStatus = (currentMoisture < moistureThreshold) ? "Ready to Run" : "Stopped (Soil wet/exceeds threshold)";
      
      String statusMessage = "\n====================\n"
                             "SYSTEM STATUS REPORT\n"
                             "====================\n"
                             "Live Soil Status: " + liveStatus + "\n"
                             "Motor Block State: " + String(readyToRun ? "Enabled (Passed Boot Check)" : "Disabled (Failed Boot Check)") + "\n"
                             "Soil Moisture: " + String(currentMoisture) + "\n"
                             "Current Threshold: " + String(moistureThreshold) + "\n"
                             "====================\n";
      
      Terminal.print(statusMessage);
    } 
    else if (input.startsWith("change moisture threshold ")) {
      String valStr = input.substring(26);
      valStr.trim();
      int newValue = valStr.toInt();
      
      if (newValue > 0) {
        moistureThreshold = newValue;
        
        // Write it permanently to memory address 0
        EEPROM.put(EEPROM_THRES_ADDR, moistureThreshold);
        
        Terminal.print("Threshold permanently updated to: ");
        Terminal.println(moistureThreshold);
        Terminal.println("Restarting machine...");
        delay(1000); 
        resetFunc(); // System reboots here, triggering setup()'s moisture check
      } else {
        Terminal.println("Invalid threshold value!");
      }
    }
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
  
  if (moisture < moistureThreshold) {
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
      if (movingForward && (currentMillis - lastSowTime >= SOW_INTERVAL)) {
        Serial.println(F("Time to Sow - Lowering Drill..."));
        currentDrillState = MOVING_DOWN;
        sowCycleActive = true; 
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
          sowCycleActive = false;       
          lastSowTime = currentMillis;   
        }
      }
      break;
  }
}
