#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <AccelStepper.h>
#include <MultiStepper.h>
#include <QuadEncoder.h> // Teensy hardware quadrature decoder library
// "In-House" Libraries
#include <EncoderTach.h> // Our encoder RPM helper library
#include <TeensyLeadscrew.h> // Our main "virtual gearbox" backend lib

// Debugging stuff
/*
#include <TeensyDebug.h>
#pragma GCC optimize ("O0")
*/

// System Specs
LatheHardwareInfo sysSpecs = {
  1.0, // encoderPulleyMultiplier : e.g. if the encoder runs at 2X spindle speed, make this 2
  8000, // encoderTicksPerRev
  2000, // stepsPerRev : usable steps per rev, including microsteps
  (float)1000000, // maxStepRate : maximum allowable rate for stepper motor (steps per sec)
  {20, tpi, leftHandThread_feedRight}, // leadscrewPitch : a Pitch struct with leadscrew specifications
};

// Declare spindle encoder
// Hardware quadrature channel 1, phase A pin 3, phase B pin 2
QuadEncoder spindleEnc(1, 3, 2, 0);

// Declare Z axis stepper
AccelStepper zStepper(AccelStepper::DRIVER, 4, 6);

// Backend Electronic Leadscrew "Gearbox" lib setup
TeensyLeadscrew els(spindleEnc, zStepper, sysSpecs, 100);

void setup()
{
  Serial.begin(19200);

  // Initialize spindle encoder
  spindleEnc.setInitConfig();
  spindleEnc.init();

  pinMode(4, OUTPUT); // TODO: do we need this?

  // TEMPORARY for TEST LED
  pinMode(13, OUTPUT);
  
  // Initialize Z stepper
  zStepper.setMaxSpeed(10000.0);
  zStepper.setAcceleration(5000.0);

  els.init();

  // TEMPORARY: Configure for test screw, 20tpi, no rapids
  els.gearbox_enableMotorBraking = true;
  els.gearbox_pitch = {20, tpi, rightHandThread_feedLeft};
  els.engageZFeedLeft();
}

void loop()
{
  els.cycle();
}
