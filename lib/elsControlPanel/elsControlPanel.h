#ifndef elsControlPanel_h
#define elsControlPanel_h

#include "TeensyLeadscrew.h"
#include "Arduino.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h" // for TFT
#include "Adafruit_LEDBackpack.h" // for alphanumeric rpm display

// This is meant to work with:
// - TFT display: https://www.adafruit.com/product/1480 (wiring info: https://learn.adafruit.com/2-2-tft-display/pinouts)
// - alphanumeric display: https://www.adafruit.com/product/2157

class elsControlPanel {
    public:
        elsControlPanel(Adafruit_ILI9341 &tftObject, uint8_t rpmReadouti2cAddress = 0x70);

        void init(); // Clear everything, call tft.begin() and similar hardware initialization stuff here

        // Main Utility Functions
        void TFT_writeGearboxInfo(String mode, Pitch currentPitch, String button1text, String button2text, String button3text);
        // Update the alphanumeric RPM display
        void alphanum_writeRPM(unsigned int rpm);
        void writeOverspeedLED(bool overspeed); // Call with "true" to illuminate overspeed LED
        // TODO: function to return a struct with states of all the buttons since we last checked

        // Hardware Objects
        Adafruit_AlphaNum4 rpmReadout;
        uint8_t rpmReadouti2cAddress;
        Adafruit_ILI9341 &tft; // This object gets created outside the class, then passed in (so we don't have to deal with all the pin assignments)
};
#endif