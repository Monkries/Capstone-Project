
#include "Arduino.h"
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "SPI.h"
#include "elsControlPanel.h"
#include "TeensyLeadscrew.h"
#include "Wire.h"
#include "Adafruit_MCP23X17.h"
#include "Bounce2mcp.h"


int modenum = 0;
int rapidnum = 0;
bool Threading = true;
bool PowerFeed = false;
int spindleRpm = 1;
int units = 1;
// See https://github.com/adafruit/Adafruit-MCP23017-Arduino-Library for table on pin descriptions
// #define INTB_PIN 19    // Interrupt pin
#define INTA_PIN 20     // Interrupt pin

elsControlPanel::elsControlPanel(Adafruit_ILI9341 &arg_tftObject, QuadEncoder &arg_encoder, uint8_t arg_rpmReadouti2cAddress) :
encoder(arg_encoder),
tft(arg_tftObject) // Properly pass in references to TFT object, encoder object
{
    rpmReadouti2cAddress = arg_rpmReadouti2cAddress; // Store i2c address for alphanumeric RPM display
}

void elsControlPanel::init() {
    alpha4.begin(rpmReadouti2cAddress); 
    tft.begin();
    i2cIO.begin_I2C();
    encoder.init();
    encoder.setInitConfig();
  
    // MCP pin setup
    // Interrupt A pin setup
    pinMode(INTA_PIN, INPUT);
    i2cIO.setupInterrupts(false, false, LOW);

    // Button pin setup
    i2cIO.pinMode((int)modeLeftBtn.mcpPin, INPUT_PULLUP);
    modeLeftBtn.debouncedButton.attach(i2cIO, (int)modeLeftBtn.mcpPin, 5);

    i2cIO.pinMode((int)modeRightBtn.mcpPin, INPUT_PULLUP);
    modeRightBtn.debouncedButton.attach(i2cIO, (int)modeRightBtn.mcpPin, 5);
    
    i2cIO.pinMode((int)function1Btn.mcpPin, INPUT_PULLUP);
    function1Btn.debouncedButton.attach(i2cIO, (int)function1Btn.mcpPin, 5);

    i2cIO.pinMode((int)function2Btn.mcpPin, INPUT_PULLUP);
    function2Btn.debouncedButton.attach(i2cIO, (int)function2Btn.mcpPin, 5);

    i2cIO.pinMode((int)function3Btn.mcpPin, INPUT_PULLUP);
    function3Btn.debouncedButton.attach(i2cIO, (int)function3Btn.mcpPin, 5);

    // Switch pin setup

    // Feed Switch
    i2cIO.pinMode((int)feedSwitch.leftPin.mcpPin, INPUT_PULLUP);
    feedSwitch.leftPin.debouncedInput.attach(i2cIO, (int)feedSwitch.leftPin.mcpPin, 5);

    i2cIO.pinMode((int)feedSwitch.rightPin.mcpPin, INPUT_PULLUP);
    feedSwitch.rightPin.debouncedInput.attach(i2cIO, (int)feedSwitch.rightPin.mcpPin, 5);

    // Braking Switch
    i2cIO.pinMode((int)brakingSwitch.disablePin.mcpPin, INPUT_PULLUP);
    brakingSwitch.disablePin.debouncedInput.attach(i2cIO, (int)brakingSwitch.disablePin.mcpPin, 5);

    i2cIO.pinMode((int)brakingSwitch.enablePin.mcpPin, INPUT_PULLUP);
    brakingSwitch.enablePin.debouncedInput.attach(i2cIO, (int)brakingSwitch.enablePin.mcpPin, 5);

    // i2cIO.setupInterruptPin(BTTN_UNITS, LOW);
    // i2cIO.setupInterruptPin(BTTN_MODEDEC, LOW);
    // i2cIO.setupInterruptPin(BTTN_MODEINC, LOW);
    // i2cIO.setupInterruptPin(BTTN_RAPID, LOW);
    // i2cIO.setupInterruptPin(BTTN_X, LOW);
}

// Button Updating
void elsControlPanel::updateInputs() {
    // Buttons
    modeRightBtn.debouncedButton.update();
    modeLeftBtn.debouncedButton.update();
    function1Btn.debouncedButton.update();
    function2Btn.debouncedButton.update();
    function3Btn.debouncedButton.update();
    
    // Switches
    feedSwitch.leftPin.debouncedInput.update();
    feedSwitch.rightPin.debouncedInput.update();
    brakingSwitch.disablePin.debouncedInput.update();
    brakingSwitch.enablePin.debouncedInput.update();

    // Feed switch logic
    if (feedSwitch.leftPin.debouncedInput.read() == LOW) {
        feedSwitch.currentState = left_towardHeadstock;
    }
    else if (feedSwitch.rightPin.debouncedInput.read() == LOW) {
        feedSwitch.currentState = right_towardTailstock;
    }
    else {
        feedSwitch.currentState = neutral;
    }

    // Braking switch logic
    if (brakingSwitch.disablePin.debouncedInput.read() == LOW) {
        brakingSwitch.enableMotorBraking = false;
    }
    else if (brakingSwitch.enablePin.debouncedInput.read() == LOW) {
        brakingSwitch.enableMotorBraking = true;
    }

}

// Initial loading screen
void elsControlPanel::TFT_splashscreen() {
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(0, 10);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(3);
    tft.println("  Electroturn\n");
    tft.print("  Electronic\n  Leadscrew\n");
    tft.setTextSize(2);
    tft.setCursor(10,120);
    tft.print("Beth Anne Burford, \n"); 
    tft.setCursor(10, 140);
    tft.print("Hampton Morgan, \n");
    tft.setCursor(10, 160);
    tft.print("Karson Shields \n");
    delay(3000);
    tft.setRotation(0);
    tft.fillScreen(ILI9341_WHITE); // Clear screen
}

// button1 = units, button2 = rapid, button3 = WIP
void elsControlPanel::TFT_writeGearboxInfo(String Mode, String pitch, String button1, String button2, String button3) { 
    tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
    tft.setTextSize(3.5);
    tft.setTextWrap(false);
    
    tft.setCursor(10, 5);
    tft.println(Mode);
    tft.setCursor(10, 60);
    tft.setTextSize(5);
    tft.println(pitch);
    tft.setCursor(10, 145);
    tft.setTextSize(3);
    tft.println(button1);
    tft.setCursor(10, 225);
    tft.println(button2);
    tft.setCursor(10, 295);
    tft.println(button3);

}

void elsControlPanel::alphanum_writeRPM(unsigned int rpm) {
    alpha4.clear();
    // Get int rpm into character array for printing (no leading 0s, but always 4 digits long)
    char buffer[4];
    sprintf(buffer, "%4d", rpm);

    for (int i=0;i<4;i++) {
        alpha4.writeDigitAscii(i,buffer[i]);
    }
    alpha4.writeDisplay();
}

void elsControlPanel::writeOverspeedLED(bool led){
        //LED On
}
