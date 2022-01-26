#include <QuadEncoder.h>

// Declare spindle encoder
// Hardware quadrature channel 1, phase A pin 3, phase B pin 2
// This encoder gives us 8000 counts per revolution
QuadEncoder spindleEnc(1, 3, 2, 0);

void setup() {
  // Initialize spindle encoder
  spindleEnc.setInitConfig();
  spindleEnc.init();
  
  // Debugging: wait to give time to open serial monitor, then initialize serial console
  Serial.begin(9600);
}

void loop() {
  // Simple encoder test code

  int currentSample = spindleEnc.read();
  
  // Roll over to 0 again after 8000 pulses (1 rev)
  if (currentSample > 8000 || currentSample < -8000)
  {
    spindleEnc.write(0);
  }

  // Print count
  Serial.print("Encoder = ");
  Serial.println(currentSample);
  delay(50);
}
