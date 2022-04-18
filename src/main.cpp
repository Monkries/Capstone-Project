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

// For debugging
void printAllInputs() {
  Serial.println("---------------");
  Serial.println("--- BUTTONS ---");

  Serial.print("Mode Left: ");
  Serial.println(cPanel.modeLeftBtn.debouncedButton.read());

  Serial.print("Mode Right: ");
  Serial.println(cPanel.modeRightBtn.debouncedButton.read());

  Serial.print("F1: ");
  Serial.println(cPanel.function1Btn.debouncedButton.read());

  Serial.print("F2: ");
  Serial.println(cPanel.function2Btn.debouncedButton.read());

  Serial.print("F3: ");
  Serial.println(cPanel.function3Btn.debouncedButton.read());

  Serial.println("--- SWITCHES ---");

  Serial.print("Feed Switch: ");
  Serial.println(cPanel.feedSwitch.currentState);

  Serial.print("Motor Braking Switch: ");
  Serial.println(cPanel.brakingSwitch.enableMotorBraking);
}

void loop()
{
  cPanel.updateInputs();
  //cPanel.TFT_writeGearboxInfo("Power Feed", "20 MM", "TPI/MM", "Rapid Off", "");

  els.gearbox.configuredPitch = {20, mm, rightHandThread_feedLeft};
  els.gearbox.enableMotorBraking = true;

  if (cPanel.feedSwitch.currentState == neutral) {
    els.clutch.disengage();
  }
  else if (cPanel.feedSwitch.currentState == left_towardHeadstock) {
    els.clutch.engageForward();
  }
  else if (cPanel.feedSwitch.currentState == right_towardTailstock) {
    els.clutch.engageReverse();
  }

  cPanel.alphanum_writeRPM(els.spindleTach.getRPM());
  if (els.spindleTach.getRPM() > MAX_SPINDLE_RPM) {
    cPanel.writeOverspeedLED(true);
  }
  else {
    cPanel.writeOverspeedLED(false);
  }

  els.cycle();
  for(int i=0;i<2000;i++){
    els.zStepper.run();
  }

  /*
  1. Handle button presses (units changes, rapid configuration, or mode changes) done
  2. Handle any encoder movement (meaning pitch adjustments)
  3. Update TFT display done
  4. Check motor braking switch status, update backend
  5. Check feed clutch switch status, update backend
  6. Get spindle RPM from backend and write to display done
  7. Check for spindle overspeed OR leadscrew overspeed (light LED for either) done minus led function
  8. Call els.cycle() done
  */
}

