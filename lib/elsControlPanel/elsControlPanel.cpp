
#include "Arduino.h"
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "SPI.h"
#include "elsControlPanel.h"
#include "TeensyLeadscrew.h"

elsControlPanel::elsControlPanel(Adafruit_ILI9341 &tftObject, uint8_t arg_rpmReadouti2cAddress = 0x70) :
tft(tftObject) // This weird syntax is required in order to pass in already-created display objects
{
    rpmReadouti2cAddress = arg_rpmReadouti2cAddress; // Store i2c address for alphanumeric RPM display
}

void elsControlPanel::init() {
    rpmReadout.begin(rpmReadouti2cAddress); // Initialize with whatever configured address (default is 0x70, per adafruit)
}

void elsControlPanel::TFT_writeGearboxInfo(String mode, Pitch currentPitch, String button1text, String button2text, String button3text) {

}

void alphanum_writeRPM(unsigned int rpm) {
    
}