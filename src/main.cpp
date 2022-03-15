#include <Arduino.h>
#include <AccelStepper.h>
#include <MultiStepper.h>
#include <QuadEncoder.h> // Teensy hardware quadrature decoder library
// "In-House" Libraries
#include <EncoderTach.h> // Our encoder RPM helper library
#include <TeensyLeadscrew.h> // Our main "virtual gearbox" backend lib

// System Specs
LatheHardwareInfo sysSpecs = {
  1.0, // encoderPulleyMultiplier : e.g. if the encoder runs at 2X spindle speed, make this 2
  8000, // encoderTicksPerRev
  2000, // stepsPerRev : usable steps per rev, including microsteps
  10000.0, // maxStepRate : maximum allowable rate for stepper motor (steps per sec)
  {20, tpi, leftHandThread_feedRight}, // leadscrewPitch : a Pitch struct with leadscrew specifications
};

// Declare spindle encoder
// Hardware quadrature channel 1, phase A pin 3, phase B pin 2
QuadEncoder spindleEnc(1, 3, 2, 0);

// Declare Z axis stepper
AccelStepper zStepper(AccelStepper::DRIVER, 4, 6);

// Tachometer library setup
// Parameters: 100ms minimum sample interval, 8000 encoder ticks per rev, spindle speed is sys_encoderPulleyMultiplier*encoder speed
EncoderTach spindleTach(100000, sysSpecs.encoderTicksPerRev, sysSpecs.encoderPulleyMultiplier);

void setup()
{
  // Initialize spindle encoder
  spindleEnc.setInitConfig();
  spindleEnc.init();

  pinMode(4, OUTPUT); // TODO: do we need this?
  
  // Initialize Z stepper
  zStepper.setMaxSpeed(sysSpecs.maxStepRate);
  zStepper.setAcceleration(500000.0);
}

void loop()
{
  
}