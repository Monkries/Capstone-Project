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
#include "Adafruit_MCP23X17.h"
#include "Bounce2mcp.h"

// System Specs for Backend
LatheHardwareInfo sysSpecs = {
  2.0, // encoderPulleyMultiplier : e.g. if the encoder runs at 2X spindle speed, make this 2
  8000, // encoderTicksPerRev
  2000, // stepsPerRev : usable steps per rev, including microsteps
  1000000, // maxStepRate : maximum allowable rate for stepper motor (steps per sec)
  {20, tpi, leftHandThread_feedRight}, // leadscrewPitch : a Pitch struct with leadscrew specifications
};

// Other Config Items
#define MAX_SPINDLE_RPM 3000
#define STEP_INTERRUPT_INTERVAL_MICROS 5 // How often (in microseconds) to call AccelStepper.run()
#define FEEDSWITCH_INT_TEENSYPIN 17
#define STEPPER_DRIVE_ENABLE_PIN 5

// BEGIN REAL CODE

// Declare spindle encoder
// Hardware quadrature channel 1, phase A pin 3, phase B pin 2
QuadEncoder spindleEnc(1, 3, 2, 0);

// Declare Z axis stepper
AccelStepper zStepper(AccelStepper::DRIVER, 4, 6);

// Backend Electronic Leadscrew "Gearbox" lib setup
TeensyLeadscrew els(spindleEnc, zStepper, sysSpecs, 100000);

////////////////////////////////////////////////////////////////////////////////////////////////
//                  CONTROL PANEL SETUP                                                       //
// TFT Display Pin Info (3/21/2022)
// SCK -> 13
// MISO -> 12
// MOSI -> 11
// LCD_CS -> 10
// SD_CS -> n/c
// RESET -> 15
// D/C -> 14
Adafruit_ILI9341 tftObject(10, 14, 11, 13, 15, 12);

// Control Panel Encoder
// Hardware quadrature channel 2, phase A on pin 0, phase B on pin 1
QuadEncoder panelEnc(2, 0, 1, 0);

// Create control panel class
elsControlPanel cPanel(tftObject, panelEnc);

// Mechanism for ensuring the stepper's run() function gets called often enough
IntervalTimer stepTimer;

void stepInterruptRoutine() {
  els.cycle();
};

void updateFeedSwitch() {
  if (cPanel.feedSwitch.currentState == neutral) {
    els.clutch.disengage();
  }
  else if (cPanel.feedSwitch.currentState == left_towardHeadstock) {
    els.clutch.engageForward();
  }
  else if (cPanel.feedSwitch.currentState == right_towardTailstock) {
    els.clutch.engageReverse();
  }
}

String generateDisplayablePitch(Pitch input) {
  String outputValue;

  String outputUnits;
  if (input.units == tpi){
    outputUnits = "TPI";
    outputValue = String(input.value, 0);
  }
  else if (input.units == mm){
    outputUnits = "mm";
    outputValue = String(input.value, 2);
  }
  else if (input.units == in_per_rev){
    outputUnits = "\"/REV";
    outputValue = String(input.value, 3);
  }

  return outputValue + outputUnits;
}

void setup()
{
  // Initialize spindle encoder
  spindleEnc.setInitConfig();
  spindleEnc.init();

  // Initialize Z stepper
  zStepper.setMaxSpeed(100000.0);
  zStepper.setAcceleration(100000.0);
  zStepper.setEnablePin(STEPPER_DRIVE_ENABLE_PIN);

  // Initialize control panel hardware
  cPanel.init();

  // Initialize electronic leadscrew backend
  els.init();
  
  // Start the TFT splash screen
  cPanel.TFT_splashscreen();
  delay(2000);

  stepTimer.begin(stepInterruptRoutine, STEP_INTERRUPT_INTERVAL_MICROS);
}

void loop()
{
  // elapsedMicros microSecSinceLastCycle = 0;
  
  /*
  1. Handle button presses (units changes, rapid configuration change, or direction change) done
  2. Handle any encoder movement (meaning pitch adjustments)
  3. Update TFT display done
  4. Check motor braking switch status, update backend
  5. Check feed clutch switch status, update backend
  6. Get spindle RPM from backend and write to display done
  7. Check for spindle overspeed OR leadscrew overspeed (light LED for either) done minus led function
  8. Call els.cycle() done
  */
  cPanel.updateInputs();

  // Update motor braking setting (the cPanel.brakingSwitch.enableMotorBraking bool is updated by updateInputs() )
  els.gearbox.enableMotorBraking = cPanel.brakingSwitch.enableMotorBraking;

  /* BUTTONS
  F1 = Units Toggle (TPI/mm/inches per rev)
  F2 = Rapid Toggle
  F3 = Unused
  */
  
  // Handle F1, units button
  if (cPanel.function1Btn.debouncedButton.fell()){
    // Toggle units
    // The order we use is TPI/mm/in_per_rev
    if (els.gearbox.configuredPitch.units == tpi){
      els.gearbox.configuredPitch.units = mm;
      // Default to 1.0mm pitch
      els.gearbox.configuredPitch.value = 1.0;
    }
    else if (els.gearbox.configuredPitch.units == mm) {
      els.gearbox.configuredPitch.units = in_per_rev;
      // Default to 0.001 feed pitch
      els.gearbox.configuredPitch.value = 0.001;
    }
    else if (els.gearbox.configuredPitch.units == in_per_rev){
      els.gearbox.configuredPitch.units = tpi;
      // Default to 20tpi pitch;
      els.gearbox.configuredPitch.value = 20;
    }
  }

  // Handle F2, Rapid Toggle
  if (cPanel.function2Btn.debouncedButton.fell()) {
    // TODO
  }

  // Poll feed switch again
  updateFeedSwitch();

  // ENCODER MOVEMENTS
  int encClicks;

  // This is sort of janky code which doesn't allow more than 1 encoder "tick" per cycle, in order to filter out some noise
  if (cPanel.encoder.read() > 0){
    encClicks = 1;
    cPanel.encoder.write(0);
  }
  else if (cPanel.encoder.read() < 0){
    encClicks = -1;
    cPanel.encoder.write(0);
  }
  else {
    encClicks = 0;
  }

  if (encClicks != 0) {
    // The increment amount of the encoder changes depending on the unit
    float increment;
    if (els.gearbox.configuredPitch.units == tpi) {
      increment = 1.0;
    }
    else if (els.gearbox.configuredPitch.units == mm) {
      increment = 0.05;
    }
    else if (els.gearbox.configuredPitch.units == in_per_rev) {
      increment = 0.001;
    }

    // Increment the configured pitch according to the encoder movement
    float change = increment * (float)encClicks;
    if ((els.gearbox.configuredPitch.value + change) < 0) {
      els.gearbox.configuredPitch.value = 0; // Prevent negative pitches
    }
    else {
      els.gearbox.configuredPitch.value = els.gearbox.configuredPitch.value + change;
    }
  }

  // Poll feed switch again
  updateFeedSwitch();
  
  // Spindle RPM Display
  int spindleRPM = els.spindleTach.getRPM();
  cPanel.alphanum_writeRPM(spindleRPM);

  // Handle overspeed indicator
  if (spindleRPM > MAX_SPINDLE_RPM || els.zStepper.speed() > (sysSpecs.maxStepRate-100)) {
    cPanel.writeOverspeedLED(true);
  }
  else {
    cPanel.writeOverspeedLED(false);
  }

  // Update TFT

  // Get the pitch in nice pretty format ignoring direction (e.g. "20TPI" or "1.75mm")
  String pitch_displayable = generateDisplayablePitch(els.gearbox.configuredPitch);
  // Testing
  // cPanel.TFT_writeGearboxInfo("", pitch_displayable, "TPI|mm|in/rev", "Rapid Right", String(microSecSinceLastCycle)+"us, "+String(cPanel.feedSwitch.currentState)+"Br " + String(cPanel.brakingSwitch.enableMotorBraking));
  cPanel.TFT_writeGearboxInfo("", pitch_displayable, "TPI|mm|in/rev", "Rapid Right", "");

  // Poll feed switch again
  updateFeedSwitch();
}

