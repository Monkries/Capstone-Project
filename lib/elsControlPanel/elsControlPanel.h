#ifndef elsControlPanel_h
#define elsControlPanel_h

#include "TeensyLeadscrew.h"
#include "Arduino.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h" // for TFT
#include "Adafruit_LEDBackpack.h" // for alphanumeric rpm display
#include "Adafruit_MCP23X17.h"

struct cPanelButton {
    bool pressedNow;
    unsigned int unhandledFells;
    int mcpPin; // The pin number corresponding to this button (IN THE MCP LIBRARY, NOT THE PHYSICAL PIN NUMBER ON THE CHIP) 
};

enum cPanelFeedSwitch {
    left_towardHeadstock,
    neutral,
    right_towardTailstock,
};

// This is meant to work with:
// - TFT display: https://www.adafruit.com/product/1480 (wiring info: https://learn.adafruit.com/2-2-tft-display/pinouts)
// - alphanumeric display: https://www.adafruit.com/product/2157

class elsControlPanel {
    public:
        elsControlPanel(Adafruit_ILI9341 &arg_tftObject, QuadEncoder &arg_encoder, uint8_t rpmReadouti2cAddress = 0x70);

        void init();

        // PANEL OUTPUTS

        // Main Utility Functions
        void TFT_writeGearboxInfo(bool Thread, bool Power, Pitch currentPitch, bool rapidLeftEnabled, bool rapidRightEnabled, String button3text);
        // Loading screen upon startup
        void TFT_splashscreen();
        // Update the alphanumeric RPM display
        void alphanum_writeRPM(unsigned int rpm);
        // LED for reaching max rpm 
        void writeOverspeedLED(unsigned int rpm); 
        
        // PANEL INPUTS

        // Encoder
        QuadEncoder &encoder;

        // Switches

        bool motorBrakingSwitch; // True to enable motor braking
        
        cPanelFeedSwitch feedSwitch = neutral; // left, neutral, or right
        // ^^ neutral is the initial state just for safety reasons

        // Buttons
        cPanelButton modeRightBtn = {false, 0, 9};
        cPanelButton modeLeftBtn;
        cPanelButton function1Btn;
        cPanelButton function2Btn;
        cPanelButton function3Btn;

        // Under-the-hood hardware objects
        private:
        Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4(); // RPM Display
        uint8_t rpmReadouti2cAddress; // Default of 0x70 stated by Adafruit

        // i2c I/O expander (MCP23017)
        Adafruit_MCP23X17 i2cIO = Adafruit_MCP23X17();
        // ^^^ Since there is nothing to configure for this device,
        // we just create the object inside this class instead of fighting with passing in a reference

        // Main TFT display
        Adafruit_ILI9341 &tft;
};
#endif