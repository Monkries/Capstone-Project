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

// BEGIN REAL CODE

// Declare spindle encoder
// Hardware quadrature channel 1, phase A pin 3, phase B pin 2
QuadEncoder spindleEnc(1, 3, 2, 0);

// Declare Z axis stepper
AccelStepper zStepper(AccelStepper::DRIVER, 4, 6);

// Backend Electronic Leadscrew "Gearbox" lib setup
TeensyLeadscrew els(spindleEnc, zStepper, sysSpecs, 500);

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

void setup()
{
  // Initialize spindle encoder
  spindleEnc.setInitConfig();
  spindleEnc.init();

  // Initialize Z stepper
  zStepper.setMaxSpeed(100000.0);
  zStepper.setAcceleration(100000.0);

  // Initialize control panel hardware
  cPanel.init();

  // Initialize electronic leadscrew backend
  els.init();
  
  // Start the TFT splash screen
  cPanel.TFT_splashscreen();
  delay(2000);
}

void loop()
{
  cPanel.DebounceUpdate();
  cPanel.TFT_writeGearboxInfo("Power Feed", "20 MM", "TPI/MM", "Rapid Off", "");

  els.gearbox.configuredPitch = {20, mm, rightHandThread_feedLeft};
  els.gearbox.enableMotorBraking = true;
  els.gearbox.rapidReturn = left_towardHeadstock;

  // Feed Switch
  // if (feed switch left)
  // els.clutch.engageForward();
  // if (feed switch right)
  // els.clutch.engageReverse();
  // else
  // els.clutch.disengage();
  cPanel.alphanum_writeRPM(els.spindleTach.getRPM());
  if (els.spindleTach.getRPM() > MAX_SPINDLE_RPM) {
    cPanel.writeOverspeedLED(true);
  }
  else {
    cPanel.writeOverspeedLED(false);
  }
  
  els.cycle();
  /*
  1. Handle button presses (units changes, rapid configuration, or mode changes)
  2. Handle any encoder movement (meaning pitch adjustments)
  3. Update TFT display
  4. Check motor braking switch status, update backend
  5. Check feed clutch switch status, update backend
  6. Get spindle RPM from backend and write to display
  7. Check for spindle overspeed OR leadscrew overspeed (light LED for either)
  8. Call els.cycle()
  */
}
