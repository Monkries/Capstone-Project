#ifndef elsControlPanel_h
#define elsControlPanel_h

#include "TeensyLeadscrew.h"
#include "Arduino.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h" // for TFT
#include "Adafruit_LEDBackpack.h" // for alphanumeric rpm display
#include "Adafruit_MCP23X17.h"
#include "Bounce2mcp.h"

// Misc helpers for control panel class

// Maps physical pin labels on chip (GPA0, GPB3, etc) to the correct integer pin numbers for the Adafruit library
enum MCP23017_GPIO_Mapping {
    GPA0,
    GPA1,
    GPA2,
    GPA3,
    GPA4,
    GPA5,
    GPA6,
    GPA7,
    GPB0,
    GPB1,
    GPB2,
    GPB3,
    GPB4,
    GPB5,
    GPB6,
    GPB7,
};

// Possible states for cPanelFeedSwitch
enum cPanelFeedSwitchState {
    left_towardHeadstock,
    neutral,
    right_towardTailstock,
};

// Structs for control panel devices

struct cPanelButton {
    BounceMcp debouncedButton;
    int fells;
    MCP23017_GPIO_Mapping mcpPin;
};

struct cPanelFeedSwitch {
    cPanelFeedSwitchState currentState;
    MCP23017_GPIO_Mapping leftPin; // the pin that goes LOW when it's set to feed left
    MCP23017_GPIO_Mapping rightPin; // the pin that goes LOW when it's set to feed right
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
        void writeOverspeedLED(bool led); 
        
        // TODO: function to return a struct with states of all the buttons since we last checked
        
        // Function for button handling
        void MCPButtons();

        // PANEL INPUTS

        // Encoder
        QuadEncoder &encoder;

        // Switches

        bool motorBrakingSwitch; // True to enable motor braking
        
        cPanelFeedSwitch feedSwitch = {neutral, GPB0, GPB1};
        // ^^ neutral is the initial state just for safety reasons

        // Buttons
        cPanelButton modeRightBtn = {false, 0, GPA0};
        cPanelButton modeLeftBtn = {false, 0, GPA1};
        cPanelButton function1Btn = {false, 0, GPA2};
        cPanelButton function2Btn = {false, 0, GPA3};
        cPanelButton function3Btn = {false, 0, GPA4};

        // Under-the-hood hardware objects
        private:
        Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4(); // RPM Display

        uint8_t rpmReadouti2cAddress; // Default of 0x70 stated by Adafruit

        // i2c I/O expander (MCP23017)
        Adafruit_MCP23X17 i2cIO = Adafruit_MCP23X17();
        // ^^^ Since there is nothing to configure for this device,
        // we just create the object inside this class instead of fighting with passing in a reference
        BounceMcp debounce = BounceMcp();   //Something?

        // Main TFT display
        Adafruit_ILI9341 &tft;
};
#endif