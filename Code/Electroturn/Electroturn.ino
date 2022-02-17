#include <AccelStepper.h>
#include <MultiStepper.h>

#include <QuadEncoder.h>

// Declare spindle encoder
// Hardware quadrature channel 1, phase A pin 3, phase B pin 2
// This encoder gives us 8000 counts per revolution
QuadEncoder spindleEnc(1, 3, 2, 0);

// Declare Z axis stepper
AccelStepper zStepper(AccelStepper::DRIVER, 4, 6);

// Spindle encoder data variables
int spindleEnc_sample;
int spindleEnc_sinceLastSpeedComputation;
int mode;

// Time between each iteration of the main loop
elapsedMicros microsSinceSample;
elapsedMicros microsSinceSpeedComputation;

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

void data() {
  // Get encoder movement since last sample, store in variable, immediately write encoder to 0
  spindleEnc_sample = spindleEnc.read();
  spindleEnc.write(0);
  // Add this movement to the movement since the last speed computation
  spindleEnc_sinceLastSpeedComputation = spindleEnc_sinceLastSpeedComputation + spindleEnc_sample;
  
  // Get time elapsed since last sample, store in variable, immediately write time elapsed to 0
  float microsElapsed_sample = microsSinceSample;
  microsElapsed_sample = 0;

  // Speed computation about every .25 seconds
  if (microsSinceSpeedComputation > 250000) {
    float spindleRevs = ((float)spindleEnc_sinceLastSpeedComputation/8000.0)*2.0; // the 2.0 is for the pulley ratio
    float minutes = (float)microsSinceSpeedComputation/6e7;

    float spindleRPM = spindleRevs/minutes;

    // Reset the stopwatch and the encoder accumulator
    microsSinceSpeedComputation = 0;
    spindleEnc_sinceLastSpeedComputation = 0;

    // Temporarily just print this here
    Serial.print("Spindle RPM: ");
    Serial.println(spindleRPM);
  }
}

  void control() {
    // Get control panel data?
  }

  int main () {
    // Setup microcontroller pins and get initial readings from encoder and control panel
    setup();
    data();
    control();

    // Default/reset mode
    mode = 0;
    switch (mode) {

        // Power Feed 
        case '1' : 
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
          for();
          break;
          
      // Place holder for other things to do?
    }
  }
