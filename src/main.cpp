
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


// Debugging stuff
/*
#include <TeensyDebug.h>
#pragma GCC optimize ("O0")
*/

// System Specs
LatheHardwareInfo sysSpecs = {
  -1.0, // encoderPulleyMultiplier : e.g. if the encoder runs at 2X spindle speed, make this 2
  8000, // encoderTicksPerRev
  2000, // stepsPerRev : usable steps per rev, including microsteps
  1000000, // maxStepRate : maximum allowable rate for stepper motor (steps per sec)
  {20, tpi, leftHandThread_feedRight}, // leadscrewPitch : a Pitch struct with leadscrew specifications
};

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

// Alphanumeric Display (for RPM) Pin Info
// SDA -> 18
// SCL -> 19
// We don't actually have to deal with this here, because the class is forced to use the hardware i2c pins

// Create control panel class
elsControlPanel cPanel(tftObject, panelEnc);

// // See https://github.com/adafruit/Adafruit-MCP23017-Arduino-Library for table on pin descriptions
// // #define INTB_PIN 19    // Interrupt pin
// #define INTA_PIN 20     // Interrupt pin
// #define BTTN_MODEINC 0  // Mode right scroll pin
// #define BTTN_MODEDEC 1  // Mode left scroll pin
// #define BTTN_UNITS 2   // Units toggle pin
// #define BTTN_RAPID 3   // Rapid scroll pin
// #define BTTN_X 4       // Future additions pin
  static unsigned int MAX_RPM = 3000; 

  int modenum = 0;
  int rapidnum = 0;
  bool Threading = true;
  bool PowerFeed = false;
  int spindleRpm = 1;
  int units = 1;

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

  // Test Config for screw, 20tpi, no rapids
  els.gearbox_enableMotorBraking = true;
  els.gearbox_pitch = {10, mm, rightHandThread_feedLeft};
  els.engageZFeedLeft();
  
  cPanel.TFT_splashscreen();
  delay(2000);

  // I2C chip testing
  


  // mcp.begin_I2C();
  // // MCP pin setup
  // // Interrupt A pin setup
  // pinMode(INTA_PIN, INPUT);
  // mcp.setupInterrupts(false, false, LOW);
  // mcp.pinMode(BTTN_MODEDEC, INPUT_PULLUP);
  // mcp.pinMode(BTTN_MODEINC, INPUT_PULLUP);
  // mcp.pinMode(BTTN_RAPID,INPUT_PULLUP);
  // mcp.pinMode(BTTN_X, INPUT_PULLUP);
  // mcp.pinMode(BTTN_UNITS, INPUT_PULLUP);
  // mcp.setupInterruptPin(BTTN_UNITS, LOW);
  // mcp.setupInterruptPin(BTTN_MODEDEC, LOW);
  // mcp.setupInterruptPin(BTTN_MODEINC, LOW);
  // mcp.setupInterruptPin(BTTN_RAPID, LOW);
  // mcp.setupInterruptPin(BTTN_X, LOW);

  // debounce.attach(mcp, BTTN_RAPID, 5);

  // Interrupt B pin setup
  // pinMode(INTB_PIN, INPUT);
  Serial.begin(9600);
}

void loop()
{
  // debounce.update();
  // Serial.println(units);
  // /////////////////////////////////////////////////////////////////////////////
  // //                              I2C Buttons                                //
  // if (!digitalRead(INTA_PIN)) {
  //   if (!mcp.digitalRead(BTTN_MODEDEC)) {
  //     modenum = modenum - 1;      
  //     if (modenum < 0) {
  //       modenum = 0;
  //     }
  //   }
  //   if (!mcp.digitalRead(BTTN_MODEINC)) {
  //     modenum = modenum + 1;
  //     if (modenum > 1) {
  //       modenum = 1;
  //     }
  //   }
  //   if (!mcp.digitalRead(BTTN_UNITS)) {
  //     units++;
  //     if ((units = 1)) {
  //       els.gearbox_pitch = {5, tpi, rightHandThread_feedLeft};
  //     }
  //     else {
  //       els.gearbox_pitch = {5, mm, rightHandThread_feedLeft};
  //     }
  //     if (units > 2) {
  //       units = 1;
  //     }
  //   }
  //   if (debounce.fell()) {
  //     if (!mcp.digitalRead(BTTN_RAPID)) {
  //       rapidnum = rapidnum + 1;
  //         if (rapidnum > 2) {
  //           rapidnum = 0;
  //         }
  //     }
  //   }
  //   else if (!mcp.digitalRead(BTTN_X)) {
  //     Serial.println("Hello world");
  //   }
  // }

  // // Mode Handling
  // switch (modenum) {
  //   case 0: 
  //     Threading = false;
  //     PowerFeed = true;
  //   break;
  //   case 1: 
  //     Threading = true;
  //     PowerFeed = false;
  //   break;
  // }

  // // // Rapid Handling
  // switch (rapidnum) {
  // case 0:
  //   els.gearbox_rapidLeft = false;
  //   els.gearbox_rapidRight = false;
  // break;
  // case 1:
  //   els.gearbox_rapidLeft = true;
  //   els.gearbox_rapidRight = false;
  // break;
  // case 2:
  //   els.gearbox_rapidLeft = false;
  //   els.gearbox_rapidRight = true;
  // break;
  // }
  // delay(500);
  ///////////////////////////////////////////////////////////////////////////////////

  // RPM display
  // int spindleRpm = (int)round(els.spindleTach.getRPM());
  spindleRpm++;

  cPanel.alphanum_writeRPM(spindleRpm);
  cPanel.writeOverspeedLED(spindleRpm);

  cPanel.TFT_writeGearboxInfo(Threading, PowerFeed, els.gearbox_pitch, els.gearbox_rapidLeft, els.gearbox_rapidRight, "3rd button");
  els.cycle();

  // LED 
  if (spindleRpm > MAX_RPM) {
    cPanel.writeOverspeedLED(true);
  }
  else {
    cPanel.writeOverspeedLED(false);
  }

}
