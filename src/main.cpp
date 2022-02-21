#include <Arduino.h>
#include <AccelStepper.h>
#include <MultiStepper.h>
#include <QuadEncoder.h> // Teensy hardware quadrature decoder library
#include <EncoderTach.h> // Our encoder RPM helper library

// System Configuration Info
const float encoderPulleyMultiplier = 1.0; // e.g. if the encoder runs at 2X spindle speed, make this 2
const unsigned int encoderTicksPerRev = 8000;
const unsigned int stepsPerRev = 2000;
const unsigned int leadscrewTpi = 20; //1/4-20TPI leadscrew in our case

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
  int sampledEncTicks = spindleEnc.read();
  spindleTach.recordTicks(sampledEncTicks);
  spindleEnc.write(0);
  delay(75);
  Serial.println(spindleTach.getRPM());

  // Default/reset mode
  int mode = 0;
  switch (mode)
  {
  // Power Feed
  case 0:
    float desiredPitch = 1; // example: cut 13tpi thread
    float stepsPerEncoderTick = (1/encoderTicksPerRev)*(1/desiredPitch)*(leadscrewTpi)*(stepsPerRev);
    int steps = (int)round(stepsPerEncoderTick*sampledEncTicks);
    zStepper.move(steps);
    break;
  // Traditional Threading (using thread dial)
  case 1:
    // Move stepper motor x in sync with the spindle
    break;
  // "Hardinge" Threading
  case 2:
    break;
  }
}