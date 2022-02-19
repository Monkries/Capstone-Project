#include <Arduino.h>
#include <AccelStepper.h>
#include <MultiStepper.h>
#include <QuadEncoder.h> // Teensy hardware quadrature decoder library
#include <EncoderTach.h> // Our encoder RPM helper library

// System Configuration Info
const float encoderPulleyMultiplier = 1.0; // e.g. if the encoder runs at 2X spindle speed, make this 2
const unsigned int encoderTicksPerRev = 8000;

// Declare spindle encoder
// Hardware quadrature channel 1, phase A pin 3, phase B pin 2
QuadEncoder spindleEnc(1, 3, 2, 0);

// Declare Z axis stepper
AccelStepper zStepper(AccelStepper::DRIVER, 4, 6);

// Tachometer library setup
// Parameters: 100ms minimum sample interval, 8000 encoder ticks per rev, spindle speed is encoderPulleyMultiplier*encoder speed
EncoderTach spindleTach(100000, encoderTicksPerRev, encoderPulleyMultiplier);

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
  // Temporary code to test RPM measurement
  spindleTach.recordTicks(spindleEnc.read());
  spindleEnc.write(0);
  delay(75);
  Serial.println(spindleTach.getRPM());

  /*
  Basic framework of real code (commented out temporarily so this will build)

  // Default/reset mode
  mode = 0;
  switch (mode)
  {

  // Power Feed
  case '1':
    // Check that zStepper.distanceToGo() is within acceptable margin (to make sure stepper isn't lagging way behind encoder)
    // Compute stepper movement from encoder movement
    // Call zStepper.moveTo()
    break;
  // Threading
  case '2':
    // Move stepper motor x in sync with the spindle
    break;
  // Do nothing with no mode selected/reset hit on control panel
  default:
    for ()
      ;
    break;

    // Place holder for other things to do?
  }
  */
}