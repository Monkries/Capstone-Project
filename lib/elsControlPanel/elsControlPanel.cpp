
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
// #define BTTN_MODEINC 0  // Mode right scroll pin
// #define BTTN_MODEDEC 1  // Mode left scroll pin
// #define BTTN_UNITS 2   // Units toggle pin
// #define BTTN_RAPID 3   // Rapid scroll pin
// #define BTTN_X 4       // Future additions pin

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
  
    // MCP pin setup
    // Interrupt A pin setup
    pinMode(INTA_PIN, INPUT);
    i2cIO.setupInterrupts(false, false, LOW);
    i2cIO.pinMode((int)modeLeftBtn.mcpPin, INPUT_PULLUP);
    modeRightBtn.debouncedButton.attach(i2cIO, (int)modeLeftBtn.mcpPin, 5);

    i2cIO.pinMode((int)modeRightBtn.mcpPin, INPUT_PULLUP);
    modeRightBtn.debouncedButton.attach(i2cIO, (int)modeRightBtn.mcpPin, 5);
    
    i2cIO.pinMode((int)function1Btn.mcpPin, INPUT_PULLUP);
    modeRightBtn.debouncedButton.attach(i2cIO, (int)function1Btn.mcpPin, 5);

    i2cIO.pinMode((int)function2Btn.mcpPin,INPUT_PULLUP);
    modeRightBtn.debouncedButton.attach(i2cIO, (int)function2Btn.mcpPin, 5);

    i2cIO.pinMode((int)function3Btn.mcpPin, INPUT_PULLUP);
    modeRightBtn.debouncedButton.attach(i2cIO, (int)function3Btn.mcpPin, 5);

    // i2cIO.setupInterruptPin(BTTN_UNITS, LOW);
    // i2cIO.setupInterruptPin(BTTN_MODEDEC, LOW);
    // i2cIO.setupInterruptPin(BTTN_MODEINC, LOW);
    // i2cIO.setupInterruptPin(BTTN_RAPID, LOW);
    // i2cIO.setupInterruptPin(BTTN_X, LOW);

    // Set up debounce objects for each button and switch


}

// Button Updating
void elsControlPanel::DebounceUpdate() {
    modeRightBtn.debouncedButton.update();
    modeLeftBtn.debouncedButton.update();
    function1Btn.debouncedButton.update();
    function2Btn.debouncedButton.update();
    function3Btn.debouncedButton.update();
    // Switch
}

//       if (!digitalRead(INTA_PIN)) {
//     if (!i2cIO.digitalRead(modeRightBtn)) {
//       modenum = modenum - 1;      
//       if (modenum < 0) {
//         modenum = 0;
//       }
//     }
//     if (!i2cIO.digitalRead(BTTN_MODEINC)) {
//       modenum = modenum + 1;
//       if (modenum > 1) {
//         modenum = 1;
//       }
//     }
//     if (!i2cIO.digitalRead(BTTN_UNITS)) {
//       units++;
//       if ((units = 1)) {
//         // els.gearbox_pitch = {5, tpi, rightHandThread_feedLeft};
//       }
//       else {
//         // els.gearbox_pitch = {5, mm, rightHandThread_feedLeft};
//       }
//       if (units > 2) {
//         units = 1;
//       }
//     }
//     if (debounce.fell()) {
//       if (!i2cIO.digitalRead(BTTN_RAPID)) {
//         rapidnum = rapidnum + 1;
//           if (rapidnum > 2) {
//             rapidnum = 0;
//           }
//       }
//     }
//     else if (!i2cIO.digitalRead(BTTN_X)) {
//       Serial.println("Hello world");
//     }
//   }

//   // Mode Handling
//   switch (modenum) {
//     case 0: 
//       Threading = false;
//       PowerFeed = true;
//     break;
//     case 1: 
//       Threading = true;
//       PowerFeed = false;
//     break;
//   }

//   // // Rapid Handling
//   switch (rapidnum) {
//   case 0:
//     // els.gearbox_rapidLeft = false;
//     // els.gearbox_rapidRight = false;
//   break;
//   case 1:
//     // els.gearbox_rapidLeft = true;
//     // els.gearbox_rapidRight = false;
//   break;
//   case 2:
//     // els.gearbox_rapidLeft = false;
//     // els.gearbox_rapidRight = true;
//   break;

//   }

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


        // if (Power) {
        //     tft.println("\n  Power Feed\n\n");
        //     if (currentPitch.units == tpi) {
        //         tft.print("  ");
        //         tft.setTextSize(4);
        //         tft.print(currentPitch.value);
        //         tft.println("TPI   ");
        //         tft.print("\n");
        //         tft.setTextSize(3);
        //     }
        //     else if (currentPitch.units == mm ) {
        //         tft.print("  ");
        //         tft.setTextSize(4);
        //         tft.print(currentPitch.value);
        //         tft.println("mm   ");
        //         tft.print("\n");
        //         tft.setTextSize(3);
        //     }
        //     tft.println("    mm/in");
        //     tft.println("\n\n                   ");
        // }
        // else if (Thread) {

        //     // If rapid right is enabled
        //     if (rapidRightEnabled) {
        //         tft.println("\n  Threading  \n\n");
        //         if (currentPitch.units == tpi) {
        //             tft.print("  ");
        //             tft.setTextSize(4);
        //             tft.print(currentPitch.value);  
        //             tft.println("TPI   \n");
        //             tft.setTextSize(3);
        //             tft.println("    mm/in\n\n\n Rapid Right");
        //         }
        //         else if (currentPitch.units == mm) {
        //             tft.print("  ");
        //             tft.setTextSize(4);
        //             tft.print(currentPitch.value);
        //             tft.println("mm   \n");
        //             tft.setTextSize(3);
        //             tft.println("    mm/in\n\n\n Rapid Right");
        //         }
        //     }
        //     // If rapid left is enabled
        //     else if (rapidLeftEnabled) {
        //         tft.println("\n  Threading  \n\n");
        //         if (currentPitch.units == tpi) {
        //             tft.print("  ");
        //             tft.setTextSize(4);
        //             tft.print(currentPitch.value);  
        //             tft.println("TPI   \n");
        //             tft.setTextSize(3);
        //             tft.println("    mm/in\n\n\n  Rapid Left");
        //         }
        //         else if (currentPitch.units == mm) {
        //             tft.print("  ");
        //             tft.setTextSize(4);
        //             tft.print(currentPitch.value);
        //             tft.println("mm   \n");
        //             tft.setTextSize(3);
        //             tft.println("    mm/in\n\n\n  Rapid Left");
        //         }

        //     }
        //     // If no rapids are enabled
        //     else if (!rapidRightEnabled && !rapidLeftEnabled) {
        //         tft.println("\n  Threading     \n\n");
        //         tft.print("  ");
        //         if (currentPitch.units == tpi) {
        //             tft.setTextSize(4);
        //             tft.print(currentPitch.value);  
        //             tft.print("TPI   \n\n");
        //             tft.setTextSize(3);
        //             tft.println("    mm/in\n\n\n  Rapid Off");
        //         }
        //         else if (currentPitch.units == mm) {
        //             tft.setTextSize(4);
        //             tft.print(currentPitch.value);
        //             tft.println("mm   \n");
        //             tft.setTextSize(3);
        //             tft.println("    mm/in\n\n\n  Rapid Off");
        //         }
        //     }
        // }

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
