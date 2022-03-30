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
TeensyLeadscrew els(spindleEnc, zStepper, sysSpecs, 100);

// CONTROL PANEL SETUP

// TFT Display Pin Info (3/21/2022)
// SCK -> 13
// MISO -> 12
// MOSI -> 11
// LCD_CS -> 10
// SD_CS -> n/c
// RESET -> 15
// D/C -> 14
Adafruit_ILI9341 tftObject(10, 14, 11, 13, 15, 12);

// Alphanumeric Display (for RPM) Pin Info
// SDA -> 18
// SCL -> 19
// We don't actually have to deal with this here, because the class is forced to use the hardware i2c pins

// Create control panel class
elsControlPanel cPanel(tftObject);

IntegerStepHelper queuedSteps;
int absoluteEncoderPosition=0;

void setup()
{
  Serial.begin(19200);

  // Initialize spindle encoder
  spindleEnc.setInitConfig();
  spindleEnc.init();

  pinMode(4, OUTPUT); // TODO: do we need this?
  
  // Initialize Z stepper
  zStepper.setMaxSpeed(100000.0);
  zStepper.setAcceleration(100000.0);

  // Initialize control panel hardware
  cPanel.init();

  // Initialize electronic leadscrew backend
  els.init();

  // TEMPORARY: Configure for test screw, 20tpi, no rapids
  els.gearbox_enableMotorBraking = true;
  els.gearbox_pitch = {20, tpi, rightHandThread_feedLeft};
  els.engageZFeedLeft();
  els.clutchState.engaged = false;
  els.clutchState.locked = true;
}

void loop()
{
  elapsedMillis stopwatch;

  // Feed normally 10sec
  Serial.println("ENGAGING FWD FEED");
  els.clutchState.engaged = true;
  while (stopwatch<10000) {
    els.cycle();
  }

  els.zStepper.runToPosition();

  // Feed neutral for 10 sec
  stopwatch=0;
  Serial.println("DISENGAGING FEED");

  els.clutchState.engaged = false;
  els.clutchState.locked = false;
  els.clutchState.inputShaftAngle = 0;
  els.encoderTicksRecorded=0;

  while (stopwatch<5000) {
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
}
