#include <Arduino.h>
#include <AccelStepper.h>
#include <MultiStepper.h>
#include <QuadEncoder.h> // Teensy hardware quadrature decoder library
#include <EncoderTach.h> // Our encoder RPM helper library

// System Configuration Info
const float cfg_encoderPulleyMultiplier = 1.0; // e.g. if the encoder runs at 2X spindle speed, make this 2
const unsigned int cfg_encoderTicksPerRev = 8000;
const unsigned int cfg_stepsPerRev = 2000;
const unsigned int cfg_leadscrewTpi = 20; //1/4-20TPI leadscrew in our case
const bool cfg_leadscrewIsLeftHand = true; // TODO: integrate this
const unsigned int cfg_rapidStepRate = 2000;

// Declare spindle encoder
// Hardware quadrature channel 1, phase A pin 3, phase B pin 2
QuadEncoder spindleEnc(1, 3, 2, 0);

// Declare Z axis stepper
AccelStepper zStepper(AccelStepper::DRIVER, 4, 6);

// Tachometer library setup
// Parameters: 100ms minimum sample interval, 8000 encoder ticks per rev, spindle speed is cfg_encoderPulleyMultiplier*encoder speed
EncoderTach spindleTach(100000, cfg_encoderTicksPerRev, cfg_encoderPulleyMultiplier);

void setup()
{
  // Initialize spindle encoder
  spindleEnc.setInitConfig();
  spindleEnc.init();

  pinMode(4, OUTPUT);

  // Initialize Z stepper
  zStepper.setMaxSpeed(10000.0);
  zStepper.setAcceleration(500000.0);
}

void loop()
{
  
}