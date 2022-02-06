#include <AccelStepper.h>
#include <MultiStepper.h>

#include <QuadEncoder.h>

// Declare spindle encoder
// Hardware quadrature channel 1, phase A pin 3, phase B pin 2
// This encoder gives us 8000 counts per revolution
QuadEncoder spindleEnc(1, 3, 2, 0);

// Declare Z axis stepper
AccelStepper zStepper(AccelStepper::DRIVER, 4, 6);

void setup() {
  // Initialize spindle encoder
  spindleEnc.setInitConfig();
  spindleEnc.init();

  pinMode(4, OUTPUT);

  // Initialize Z stepper
  zStepper.setMaxSpeed(10000.0);
  zStepper.setAcceleration(500000.0);
  
  // Debugging: wait to give time to open serial monitor, then initialize serial console
  Serial.begin(9600);
  delay(1000);
  zStepper.move(200.0);
  zStepper.run();
  delay(1000);
}

void loop() {
  int currentSample = spindleEnc.read();

  // Convert encoder steps to motor movement steps
  float motorSteps = ((float)currentSample/8000.0)*2000.0;

  zStepper.moveTo(motorSteps);
  zStepper.run();
}
