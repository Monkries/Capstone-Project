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

// BEGIN CONFIG SECTION

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
#define BACKEND_INTERRUPT_INTERVAL_MICROS 5 // How often (in microseconds) to call TeensyLeadscrew.cycle() ( which also calls AccelStepper.run() )

#define STEPPER_DRIVE_ENABLE_TEENSYPIN 5
#define STEPPER_DRIVE_STEP_TEENSYPIN 4
#define STEPPER_DRIVE_DIRECTION_TEENSYPIN 6

#define SPINDLE_ENCODER_PHASEA_TEENSYPIN 3
#define SPINDLE_ENCODER_PHASEB_TEENSYPIN 2
#define SPINDLE_TACH_SAMPLEPERIOD_MICROS 100000

/*
TFT Display Pin Info (3/21/2022)
SCK -> 13
MISO -> 12
MOSI -> 11
LCD_CS -> 10
SD_CS -> n/c
RESET -> 15
D/C -> 14
*/
#define TFT_SCK_TEENSYPIN 13
#define TFT_MISO_TEENSYPIN 12
#define TFT_MOSI_TEENSYPIN 11
#define TFT_DISPLAYCS_TEENSYPIN 10
#define TFT_RESET_TEENSYPIN 15
#define TFT_DC_TEENSYPIN 14

#define PANEL_ENCODER_PHASEA_TEENSYPIN 0
#define PANEL_ENCODER_PHASEB_TEENSYPIN 1

// END CONFIG, BEGIN REAL CODE

// Declare spindle encoder
// Hardware quadrature channel 1, phase A pin 3, phase B pin 2
QuadEncoder spindleEnc(1, SPINDLE_ENCODER_PHASEA_TEENSYPIN, SPINDLE_ENCODER_PHASEB_TEENSYPIN, 0);

// Declare Z axis stepper
AccelStepper zStepper(AccelStepper::DRIVER, STEPPER_DRIVE_STEP_TEENSYPIN, STEPPER_DRIVE_DIRECTION_TEENSYPIN);

// Backend Electronic Leadscrew "Gearbox" lib setup
TeensyLeadscrew els(spindleEnc, zStepper, sysSpecs, SPINDLE_TACH_SAMPLEPERIOD_MICROS);

// CONTROL PANEL HARDWARE OBJECTS

// TFT Display
Adafruit_ILI9341 tftObject(TFT_DISPLAYCS_TEENSYPIN, TFT_DC_TEENSYPIN, TFT_MOSI_TEENSYPIN, TFT_SCK_TEENSYPIN, TFT_RESET_TEENSYPIN, TFT_MISO_TEENSYPIN);

// Control Panel Encoder
// Hardware quadrature channel 2, phase A, phase B
QuadEncoder panelEnc(2, PANEL_ENCODER_PHASEA_TEENSYPIN, PANEL_ENCODER_PHASEB_TEENSYPIN, 0);

// Create control panel class
elsControlPanel cPanel(tftObject, panelEnc);

// Mechanism for ensuring the stepper's run() function gets called often enough
IntervalTimer backendTimer;

void backendISR() {
  els.cycle();
};

// @brief Helper function that checks the feed switch state, and updates the backend accordingly
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

// @brief Helper function for converting our Pitch structs to pretty human-readable strings
// @param Input pitch to be string-ified
// @returns e.g. "20TPI", "1.25mm", or "0.128" "
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
    outputUnits = "\"";
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
  zStepper.setEnablePin(STEPPER_DRIVE_ENABLE_TEENSYPIN);

  // Initialize control panel hardware
  cPanel.init();

  // Initialize electronic leadscrew backend
  els.init();
  
  // Start the TFT splash screen
  cPanel.TFT_splashscreen();
  delay(2000);

  // Start the timer interrupt which calls els.cycle()
  // This "arms" the system
  backendTimer.begin(backendISR, BACKEND_INTERRUPT_INTERVAL_MICROS);
}

// Note that while this loop is handling input from the user, the backend is getting called with a timer interrupt every 5us
// (so any updates we make to the backend here take virtually immediate effect)
void loop()
{ 
  // STEP 1: HANDLE BUTTON PRESSES (UNITS, RAPIDS, MOTOR BRAKING SWITCH)
  cPanel.updateInputs();

  // Update motor braking setting (the cPanel.brakingSwitch.enableMotorBraking bool is updated by updateInputs() )
  els.gearbox.enableMotorBraking = cPanel.brakingSwitch.enableMotorBraking;

  /*
  Button Assignments:
  F1 = Units Toggle (TPI/mm per rev/inches per rev)
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

  // STEP 2: POLL FEED SWITCH AND UPDATE BACKEND
  updateFeedSwitch();

  // STEP 3: CHECK FOR PANEL ENCODER MOVEMENT (PITCH ADJUSTMENT)
  int encClicks = (cPanel.encoder.read() / 4);
  if (encClicks != 0) {
    cPanel.encoder.write(0);

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

  // STEP 4: POLL FEED SWITCH AND UPDATE BACKEND
  updateFeedSwitch();
  
  // STEP 5: GET SPINDLE RPM FROM BACKEND, WRITE TO ALPHANUMERIC DISPLAY
  int spindleRPM = els.spindleTach.getRPM();
  cPanel.alphanum_writeRPM(spindleRPM);

  // Handle overspeed indicator
  if (spindleRPM > MAX_SPINDLE_RPM || els.zStepper.speed() > (sysSpecs.maxStepRate-100)) {
    cPanel.writeOverspeedLED(true);
  }
  else {
    cPanel.writeOverspeedLED(false);
  }

  // STEP 6: UPDATE TFT WITH CURRENT PITCH SETTINGS

  // Get the pitch in nice pretty format ignoring direction (e.g. "20TPI" or "1.75mm")
  String pitch_displayable = generateDisplayablePitch(els.gearbox.configuredPitch);
  cPanel.TFT_writeGearboxInfo("", pitch_displayable, "TPI/mm/in", "", "");

  // STEP 7: POLL FEED SWITCH AND UPDATE BACKEND
  updateFeedSwitch();
}

