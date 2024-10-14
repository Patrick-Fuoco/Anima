// SLAVE 1 - Controls Module 1 (Containers) & Module 2 (Pump)
// Module 1: Two stepper motors (U fork and C fork)
// Module 2: DC motor for sauce dispensing (controlled by direction and torque)
#include <AccelStepper.h>
#include <Wire.h>
#include <EEPROM.h>  // Ensure EEPROM is included for reading/writing motor positions

#define SLAVE_ADDRESS 1

// Pin assignments for stepper motors (Module 1)
const int enPin = 8;
const int stepUPin = 4;
const int dirUPin = 7;
const int stepCPin = 3;
const int dirCPin = 6;

// Pin assignments for DC motor (Module 2 - Pump)
const int dirDCPin = 9;  // Assign actual pin value for direction control
const int enDC = 10;     // Enable pin for DC motor
int in1 = 11;            // Motor direction control pin 1
int in2 = 12;            // Motor direction control pin 2

// Variables for stepper motor movement
int TIME_U;
int DIR_U;
int TIME_C;
int DIR_C;
int StartTime;

// Variables for CIP cleaning processes
bool soap_CIP;
bool rinse_CIP;

// Variables received from master
int received_quantity = -1;
int received_item = -1;
int loop_counter = 0;

// Stepper motor speed and acceleration
const int v = 200;
const int a = 600;

// Time estimation constants (to be defined based on motor trials)
int full_rotation_milliseconds_mod1 = 400;  // This constant needs to be tested

// Function to write integer values to EEPROM
void writeIntToEEPROM(int address, unsigned int value) {
  EEPROM.write(address, lowByte(value));
  EEPROM.write(address + 1, highByte(value));
}

// Function to read integer values from EEPROM
int readIntFromEEPROM(int address) {
  byte lowByteValue = EEPROM.read(address);
  byte highByteValue = EEPROM.read(address + 1);
  return (lowByteValue << 0) | (highByteValue << 8);
}

// Function to convert two bytes into an integer
int Byte_Int(byte highByte, byte lowByte) {
  return (int)((highByte << 8) | lowByte);
}

// I2C event to receive data from master
void ReceiveInformation() {
  byte lowByte = Wire.read();
  byte highByte = Wire.read();
  received_quantity = Byte_Int(highByte, lowByte);

  lowByte = Wire.read();
  highByte = Wire.read();
  received_item = Byte_Int(highByte, lowByte);

  soap_CIP = Wire.read();
  rinse_CIP = Wire.read();
}

// Stepper motor instances for U fork and C fork
AccelStepper StepperU(1, stepUPin, dirUPin);  // U fork of module 1
AccelStepper StepperC(1, stepCPin, dirCPin);  // C fork of module 1

/* EEPROM memory address map:
14: StepperU milliseconds
16: StepperU direction
18: StepperC milliseconds
20: StepperC direction
*/

// Module 1: Container manipulation using two stepper motors
void module1() {
  if (loop_counter < received_quantity) {
    loop_counter++;

    // Move C Fork (forward)
    StartTime = millis();
    writeIntToEEPROM(20, 1);  // Set forward direction for StepperC
    while (StepperC.distanceToGo() != 0) {
      StepperC.run();
      writeIntToEEPROM(18, (millis() - StartTime));  // Record time to EEPROM
    }

    // Move U Fork (backward)
    StartTime = millis();
    writeIntToEEPROM(16, 1);  // Set backward direction for StepperU
    while (StepperU.distanceToGo() != 0) {
      StepperU.run();
      writeIntToEEPROM(14, (millis() - StartTime));  // Record time to EEPROM
    }

    // Return U Fork to original position
    StepperU.moveTo(200);
    delay(250);

    // Move U Fork (forward) to the original starting position
    StartTime = millis();
    writeIntToEEPROM(16, -1);  // Reverse direction for U Fork
    while (StepperU.distanceToGo() != 0) {
      StepperU.run();
      writeIntToEEPROM(14, full_rotation_milliseconds_mod1 - (millis() - StartTime));  // Shortest route back
    }

    // Return C Fork to original position
    StepperC.moveTo(0);
    StartTime = millis();
    writeIntToEEPROM(20, -1);  // Reverse direction for C Fork
    while (StepperC.distanceToGo() != 0) {
      StepperC.run();
      writeIntToEEPROM(18, full_rotation_milliseconds_mod1 - (millis() - StartTime));  // Record time
    }

    delay(500);  // Delay based on speed of other modules and conveyor movement
  }
}

// Module 2: DC motor control for sauce dispensing
void module2() {
  if (loop_counter == 1) {
    // Dispense the first container
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);  // Set motor direction forward
    switch (received_item) {
      case 1: analogWrite(enDC, 150); break;
      case 2: analogWrite(enDC, 180); break;
      case 3: analogWrite(enDC, 200); break;
      case 4: analogWrite(enDC, 220); break;
    }
    delay(1000);  // Dispense time for first container (test for accuracy)
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);  // Stop motor
  }

  if (loop_counter > 1 && loop_counter < (received_quantity + 1)) {
    // Dispense subsequent containers
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);  // Set motor direction forward
    switch (received_item) {
      case 1: analogWrite(enDC, 150); break;
      case 2: analogWrite(enDC, 180); break;
      case 3: analogWrite(enDC, 200); break;
      case 4: analogWrite(enDC, 220); break;
    }
    delay(1000);  // Dispense time (adjust based on tests)
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);  // Stop motor
  }

  if (loop_counter == (received_quantity + 1)) {
    // Reverse motor direction to pump sauce back into the bucket
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);  // Set motor direction reverse
    switch (received_item) {
      case 1: analogWrite(enDC, 150); break;
      case 2: analogWrite(enDC, 180); break;
      case 3: analogWrite(enDC, 200); break;
      case 4: analogWrite(enDC, 220); break;
    }
    delay(1000);  // Time to reverse pump (adjust based on tests)
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);  // Stop motor
  }

  delay(500);  // Delay to sync with other modules
}

// Default cleaning function (used for soap/rinse CIP)
void Default_run() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);  // Set motor direction forward
  analogWrite(enDC, 255);  // Full power for soapy water
  delay(30000);  // Run for 30 seconds
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);  // Stop motor
}

void setup() {
  // Initialize motor control pins
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, LOW);
  pinMode(stepUPin, OUTPUT);
  pinMode(dirUPin, OUTPUT);
  pinMode(stepCPin, OUTPUT);
  pinMode(dirCPin, OUTPUT);
  pinMode(enDC, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  // Initialize steppers with speed and acceleration
  StepperU.setMaxSpeed(v);
  StepperU.setAcceleration(a);
  StepperU.setCurrentPosition(200);
  StepperU.moveTo(0);

  // Read stored motor states from EEPROM
  TIME_U = readIntFromEEPROM(14);
  DIR_U = readIntFromEEPROM(16);
  StepperU.setSpeed(DIR_U);
  StepperU.runSpeed();
  delay(TIME_U);
  StepperU.stop();

  TIME_C = readIntFromEEPROM(18);
  DIR_C = readIntFromEEPROM(20);
  StepperC.setSpeed(DIR_C);
  StepperC.runSpeed();
  delay(TIME_C);
  StepperC.stop();

  // Set motor to stop after sauce is pumped back
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);  // Reverse motor
  analogWrite(enDC, 150);
  delay(500);  // Delay for reverse action
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);  // Stop motor

  // Initialize I2C for communication
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(ReceiveInformation);
}

void loop() {
  if (received_quantity >= 0 && loop_counter <= (received_quantity + 3)) {
    // Run modules based on received data
    module1();
    module2();
    loop_counter++;
  } else if (soap_CIP) {
    Default_run();  // Clean with soapy water
  } else if (rinse_CIP) {
    Default_run();  // Clean with rinse water
  }
}
