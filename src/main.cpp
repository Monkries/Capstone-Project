
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <AccelStepper.h>
#include <MultiStepper.h>
#include <QuadEncoder.h> // Teensy hardware quadrature decoder library
// "In-House" Libraries
#include <EncoderTach.h> // Our encoder RPM helper library
#include <TeensyLeadscrew.h> // Our main "virtual gearbox" backend lib
#include <elsControlPanel.h>
#include "Adafruit_LEDBackpack.h"


// Debugging stuff
/*
#include <TeensyDebug.h>
#pragma GCC optimize ("O0")
*/

// System Specs
LatheHardwareInfo sysSpecs = {
  -1.0, // encoderPulleyMultiplier : e.g. if the encoder runs at 2X spindle speed, make this 2
  8000, // encoderTicksPerRev
  2000, // stepsPerRev : usable steps per rev, including microsteps
  1000000, // maxStepRate : maximum allowable rate for stepper motor (steps per sec)
  {20, tpi, leftHandThread_feedRight}, // leadscrewPitch : a Pitch struct with leadscrew specifications
};

// Declare spindle encoder
// Hardware quadrature channel 1, phase A pin 3, phase B pin 2
QuadEncoder spindleEnc(1, 3, 2, 0);

// Declare Z axis stepper
AccelStepper zStepper(AccelStepper::DRIVER, 4, 6);

// Backend Electronic Leadscrew "Gearbox" lib setup
TeensyLeadscrew els(spindleEnc, zStepper, sysSpecs, 500);

/* CONTROL PANEL SETUP
// TFT Display Pin Info (3/21/2022)
// SCK -> 13
// MISO -> 12
// MOSI -> 11
// LCD_CS -> 10
// SD_CS -> n/c
// RESET -> 15
// D/C -> 14
*/
Adafruit_ILI9341 tftObject(10, 14, 11, 13, 15, 12);

// Alphanumeric Display (for RPM) Pin Info
// SDA -> 18
// SCL -> 19
// We don't actually have to deal with this here, because the class is forced to use the hardware i2c pins

// Create control panel class
elsControlPanel cPanel(tftObject);

// Temporary Global Variables for Testing
IntegerStepHelper queuedSteps;
int absoluteEncoderPosition=0;

void setup()
{
  // Initialize spindle encoder
  spindleEnc.setInitConfig();
  spindleEnc.init();

  // Setup pins for the teensy
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(17, OUTPUT);
  pinMode(16, OUTPUT);
 
  // Initialize Z stepper
  zStepper.setMaxSpeed(100000.0);
  zStepper.setAcceleration(100000.0);

  // Initialize control panel hardware
  cPanel.init();

  // Initialize electronic leadscrew backend
  els.init();
  
  // Start the TFT splash screen
  cPanel.TFT_splashscreen();

  // Test Config for screw, 20tpi, no rapids
  els.gearbox.enableMotorBraking = true;
  els.gearbox.configuredPitch = {20, tpi, rightHandThread_feedLeft};
  
}

void loop()
{
  elapsedMillis motorRunTime=0;
  elapsedMillis stopwatch=0;


  // Feed Forward 10 seconds
  Serial.println("FORWARD!");
  stopwatch=0;
  els.clutch.engageForward();
  while (stopwatch<10000) {
    motorRunTime=0;
    els.cycle();
    while (motorRunTime<30){
      els.zStepper.run();
    }
  }

  // Feed Neutral 5 seconds
  Serial.println("NEUTRAL!");
  stopwatch=0;
  els.clutch.disengage();
  while(stopwatch<5000){
    els.cycle();
  }

  // Feed Reverse 10 seconds
  Serial.println("REVERSE!");
  stopwatch=0;
  els.clutch.engageReverse();
  while (stopwatch<10000) {
    motorRunTime=0;
    els.cycle();
    while (motorRunTime<30){
      els.zStepper.run();
    }
  }

  // Feed Neutral 5 seconds
  Serial.println("NEUTRAL!");
  stopwatch=0;
  els.clutch.disengage();
  while(stopwatch<5000){
    els.cycle();
  }

  /*
  // WORKING TEST CODE #1
  absoluteEncoderPosition = els.spindleEncoder.read();
  queuedSteps.totalValue = els.calculateMotorSteps(absoluteEncoderPosition);
  els.zStepper.moveTo(queuedSteps.getIntegerPart());
  els.zStepper.run();
  */

  /*
  // WORKING TEST CODE #2
  // This has the stepper motor in absolute coordinates, but the encoder in relative coordinates
  int relativeEncoderMovement = els.spindleEncoder.read();
  if (relativeEncoderMovement!=0) {
      els.spindleEncoder.write(0); // This IF keeps us from losing "unfinished" steps by zeroing midway through a pulse train
  }
  queuedSteps.totalValue += els.calculateMotorSteps(relativeEncoderMovement);
  els.zStepper.moveTo(queuedSteps.getIntegerPart());
  els.zStepper.run();
  */

  /*
  // WORKING TEST CODE #3
  // Both encoder and stepper in relative coordinates
  int relativeEncoderMovement = els.spindleEncoder.read();
  if (relativeEncoderMovement!=0) {
      els.spindleEncoder.write(0); // This IF keeps us from losing "unfinished" steps by zeroing midway through a pulse train
  }
  queuedSteps.totalValue += els.calculateMotorSteps(relativeEncoderMovement);
  els.zStepper.move(queuedSteps.popIntegerPart()+els.zStepper.distanceToGo());
  els.zStepper.run();
  */

  // RPM display
  int spindleRpm = (int)round(els.spindleTach.getRPM());
  cPanel.alphanum_writeRPM(spindleRpm);
  cPanel.writeOverspeedLED(spindleRpm);
}
