#ifndef elsControlPanel_h
#define elsControlPanel_h

#include "TeensyLeadscrew.h"
#include "Arduino.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h" // for TFT
#include "Adafruit_LEDBackpack.h" // for alphanumeric rpm display
#include "Adafruit_MCP23X17.h"

// This is meant to work with:
// - TFT display: https://www.adafruit.com/product/1480 (wiring info: https://learn.adafruit.com/2-2-tft-display/pinouts)
// - alphanumeric display: https://www.adafruit.com/product/2157

class elsControlPanel {
    public:
        elsControlPanel(Adafruit_ILI9341 &tftObject, uint8_t rpmReadouti2cAddress = 0x70);

        void init();

        // Main Utility Functions
        void TFT_writeGearboxInfo(bool Thread, bool Power, Pitch currentPitch, bool rapidLeftEnabled, bool rapidRightEnabled, String button3text);
        // Loading screen upon startup
        void TFT_splashscreen();
        // Update the alphanumeric RPM display
        void alphanum_writeRPM(unsigned int rpm);
        // LED for reaching max rpm 
        void writeOverspeedLED(unsigned int rpm); 
        // TODO: function to return a struct with states of all the buttons since we last checked

        // Hardware Objects
        Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();
        uint8_t rpmReadouti2cAddress; // Default of 0x70 stated by Adafruit
        Adafruit_ILI9341 tft = Adafruit_ILI9341(9, 10); // This object gets created outside the class
};
#endif