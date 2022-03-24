
#include <Arduino.h>
//#include <Wire.h>
#include <SPI.h>
#include <AccelStepper.h>
#include <MultiStepper.h>
#include <QuadEncoder.h> // Teensy hardware quadrature decoder library
// "In-House" Libraries
#include <EncoderTach.h> // Our encoder RPM helper library
#include <TeensyLeadscrew.h> // Our main "virtual gearbox" backend lib
#include <elsControlPanel.h>
// I2C Libraries
#include "i2c_driver.h"
#include "i2c_register_slave.h"
#include "i2c_device.h"
#include "imx_rt1060/imx_rt1060_i2c_driver.h"
//#include "i2c_driver_wire.h"

// Debugging stuff
/*
#include <TeensyDebug.h>
#pragma GCC optimize ("O0")
*/

// System Specs
LatheHardwareInfo sysSpecs = {
  1.0, // encoderPulleyMultiplier : e.g. if the encoder runs at 2X spindle speed, make this 2
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
TeensyLeadscrew els(spindleEnc, zStepper, sysSpecs, 100);

/* CONTROL PANEL SETUP
// TFT Display Pin Info (3/21/2022)
// SCK -> 13
// MISO -> 12
// MOSI -> 11
// LCD_CS -> 10
// SD_CS -> n/c
// RESET -> 15
// D/C -> 14
*/
Adafruit_ILI9341 tftObject(10, 14, 11, 13, 15, 12);

// Alphanumeric Display (for RPM) Pin Info
// SDA -> 18
// SCL -> 19
// We don't actually have to deal with this here, because the class is forced to use the hardware i2c pins

// Create control panel class
elsControlPanel cPanel(tftObject);

  // Setup master-slave for Alphanumeric
  I2CMaster& master = Master; 
  const uint8_t slave_address = 0x70;
  I2CDevice sensor = I2CDevice(master, slave_address, __ORDER_BIG_ENDIAN__ );

// Setup function
void setup()
{
  // Initialize spindle encoder
  spindleEnc.setInitConfig();
  spindleEnc.init();

  // Setup pins for the teensy
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(17, OUTPUT);
  pinMode(16, OUTPUT);
  
  // Initalize Master
  master.begin(400 * 1000U);
  Serial.begin(9600);
  
  // Initialize Z stepper
  zStepper.setMaxSpeed(100000.0);
  zStepper.setAcceleration(500000.0);

  // Initialize control panel hardware
  cPanel.init();

  // Initialize electronic leadscrew backend
  els.init();

  // TEMPORARY: Configure for test screw, 20tpi, no rapids
  els.gearbox_enableMotorBraking = true;
  els.gearbox_pitch = {20, tpi, rightHandThread_feedLeft};
  els.engageZFeedLeft();
}

void loop()
{
  unsigned int rpm = 2000;
  byte val = cPanel.alphanum_writeRPM(rpm);
  sensor.write(1, val, true);
  // Wire.beginTransmission(address);
  // unsigned int &test = cPanel.alphanum_writeRPM(rpm);
  // digitalWrite(LED_BUILTIN, HIGH);
  // delay(1000);
  // digitalWrite(LED_BUILTIN, LOW);
  // delay(1000);
  els.gearbox_pitch = {13, tpi, rightHandThread_feedLeft};
  cPanel.TFT_writeGearboxInfo("Threading", els.gearbox_pitch, "tpi", "Rapid Left", "hello");
  els.gearbox_pitch = {15.6, mm, rightHandThread_feedLeft};
  delay(5000);
  cPanel.TFT_writeGearboxInfo("Threading", els.gearbox_pitch, "mm", "Rapid Right", "hello");
  delay(5000);
  cPanel.TFT_writeGearboxInfo("Threading", els.gearbox_pitch, "tpi", "Rapid Off", "hello");
  delay(5000);
  els.gearbox_pitch = {9, tpi, rightHandThread_feedLeft};
  cPanel.TFT_writeGearboxInfo("Power Feed", els.gearbox_pitch, "tpi", "Rapid Off", "hello");
  els.gearbox_pitch = {8, mm, rightHandThread_feedLeft};
  delay(5000);
  cPanel.TFT_writeGearboxInfo("Power Feed", els.gearbox_pitch, "mm", "Rapid Off", "hello");
  delay(5000);

  //  Wire.write(val);
  // cPanel.alphanum_writeRPM(rpm);
  // //els.cycle();
  // for (int i = 0; i <= 1; i++) {
  //   cPanel.alphanum_writeRPM(rpm);
  //   rpm = rpm + 3500;
  //   cPanel.alphanum_writeRPM(rpm);
  //   rpm = rpm - 4500;
  //   cPanel.alphanum_writeRPM(rpm);
  //   rpm = rpm - 1000;
  //   cPanel.alphanum_writeRPM(rpm);
  // }
  // cPanel.alphanum_writeRPM(rpm);
  // Wire.endTransmission();
}