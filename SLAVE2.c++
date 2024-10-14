// SLAVE 2 - Controls Module 3 (Lids) & Module 4 (Stamp)
// Module 3: Two stepper motors for lids (U fork and C fork)
// Module 4: One stepper motor for the stamp
#include <AccelStepper.h>
#include <Wire.h>
#include <EEPROM.h>  // Make sure to include EEPROM library

#define SLAVE_ADDRESS 2

// Pin assignments
const int enPin = 8;
const int stepUPin = 4;
const int dirUPin = 7;
const int stepCPin = 3;
const int dirCPin = 6;
const int stepSPin = 5;  // Assign actual pin value
const int dirSPin = 9;   // Assign actual pin value

// Variables for stepper motor times and directions
int TIME_U;
int DIR_U;
int TIME_C;
int DIR_C;
int TIME_S;
int DIR_S;
int StartTime;  // Keeps track of time for motor movements

// Flags and other global variables
bool soap_CIP;
bool rinse_CIP;
int received_quantity = -1;
int received_item = -1;
int loop_counter = 0;
const int v = 200;
const int a = 600;
int half_rotation_milliseconds_mod3 = 200; // Estimate for 1/2 rotation time

// Function to write an integer value to EEPROM
void writeIntToEEPROM(int address, unsigned int value) {
  EEPROM.write(address, lowByte(value));
  EEPROM.write(address + 1, highByte(value));
}

// Function to read an integer value from EEPROM
int readIntFromEEPROM(int address) {
  byte lowByteValue = EEPROM.read(address);
  byte highByteValue = EEPROM.read(address + 1);
  return (lowByteValue << 0) | (highByteValue << 8);
}

// Function to combine two bytes into an integer
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

// Stepper motor instances for different axes
AccelStepper StepperU(1, stepUPin, dirUPin); // U fork of module 3
AccelStepper StepperC(1, stepCPin, dirCPin); // C fork of module 3
AccelStepper StepperS(1, stepSPin, dirSPin); // Stamp in module 4

/* EEPROM Memory Addresses:
2: StepperU milliseconds
4: StepperU direction
6: StepperC milliseconds
8: StepperC direction
10: StepperS milliseconds
12: StepperS direction
*/

// Function to control Module 3 (Lids - U fork and C fork)
void module3() {
  if (loop_counter >= 2 && loop_counter < (received_quantity + 2)) {
    loop_counter++;

    // Handle C Fork (Forward)
    StartTime = millis();
    writeIntToEEPROM(8, 1);  // Set direction for StepperC (forward)
    while (StepperC.distanceToGo() != 0) {
      StepperC.run();
      writeIntToEEPROM(6, (millis() - StartTime));  // Log movement time to EEPROM
    }

    // Handle U Fork (Backward)
    StartTime = millis();
    writeIntToEEPROM(4, 1);  // Set direction for StepperU (backward)
    while (StepperU.distanceToGo() != 0) {
      StepperU.run();
      writeIntToEEPROM(2, (millis() - StartTime));  // Log movement time to EEPROM
    }

    // Return U Fork to original position
    StepperU.moveTo(200);
    delay(250);

    StartTime = millis();
    writeIntToEEPROM(4, -1);  // Reverse direction for U Fork
    while (StepperU.distanceToGo() != 0) {
      StepperU.run();
      writeIntToEEPROM(2, half_rotation_milliseconds_mod3 - (millis() - StartTime)); // Record shortest route to OG position
    }

    // Return C Fork to original position
    StepperC.moveTo(0);
    StartTime = millis();
    writeIntToEEPROM(8, -1);  // Reverse direction for C Fork
    while (StepperC.distanceToGo() != 0) {
      StepperC.run();
      writeIntToEEPROM(6, half_rotation_milliseconds_mod3 - (millis() - StartTime)); // Log time
    }

    // Delay to synchronize with other modules and conveyor
    delay(/*delay time*/);
  }
}

// Function to control Module 4 (Stamp)
void module4() {
  if (loop_counter >= 3 && loop_counter < (received_quantity + 3)) {
    // Move stamp forward
    writeIntToEEPROM(12, 1);  // Forward direction for StepperS
    StartTime = millis();
    while (StepperS.distanceToGo() != 0) {
      StepperS.run();
      writeIntToEEPROM(10, (millis() - StartTime));  // Log movement time to EEPROM
    }

    // Short delay for stamp operation
    delay(100);
    StepperS.setCurrentPosition(400);  // Reset stamp position

    // Move stamp backward
    writeIntToEEPROM(12, -1);  // Reverse direction for StepperS
    StartTime = millis();
    while (StepperS.distanceToGo() != 0) {
      StepperS.run();
      writeIntToEEPROM(10, (half_rotation_milliseconds_mod3 - (millis() - StartTime))); // Log shortest time back to OG position
    }

    // Delay for synchronization with other modules
    delay(/*delay time*/);
  }
}

void setup() {
  // Set up pins for stepper motor control
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, LOW);  // Enable stepper motors

  pinMode(stepUPin, OUTPUT);
  pinMode(dirUPin, OUTPUT);
  pinMode(stepCPin, OUTPUT);
  pinMode(dirCPin, OUTPUT);

  // Initialize steppers with speed and acceleration
  StepperU.setMaxSpeed(v);
  StepperU.setAcceleration(a);
  StepperU.setCurrentPosition(200);  // Starting position for U fork
  StepperU.moveTo(0);

  StepperC.setMaxSpeed(v);
  StepperC.setAcceleration(a);
  StepperC.moveTo(200);  // Starting position for C fork

  // Read previous positions from EEPROM
  TIME_U = EEPROM.read(2);
  DIR_U = EEPROM.read(4);
  StepperU.setSpeed(DIR_U);
  StepperU.runSpeed();
  delay(TIME_U);
  StepperU.stop();

  TIME_C = EEPROM.read(6);
  DIR_C = EEPROM.read(8);
  StepperC.setSpeed(DIR_C);
  StepperC.runSpeed();
  delay(TIME_C);
  StepperC.stop();

  TIME_S = EEPROM.read(10);
  DIR_S = EEPROM.read(12);
  StepperS.setSpeed(DIR_S);
  StepperS.runSpeed();
  delay(TIME_S);
  StepperS.stop();

  // Set up I2C communication as a slave
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(ReceiveInformation);
}

void loop() {
  // Main control loop
  if (received_quantity >= 0) {
    while (loop_counter <= (received_quantity * 25 + 3)) {
      module3();
      module4();
      loop_counter++;
    }
  }
}
