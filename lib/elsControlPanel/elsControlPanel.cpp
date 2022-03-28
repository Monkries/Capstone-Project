
#include "Arduino.h"
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "SPI.h"
#include "elsControlPanel.h"
#include "TeensyLeadscrew.h"
#include "Wire.h"

elsControlPanel::elsControlPanel(Adafruit_ILI9341 &tftObject, uint8_t arg_rpmReadouti2cAddress) :
tft(tftObject) // This weird syntax is required in order to pass in already-created display objects
{
    rpmReadouti2cAddress = arg_rpmReadouti2cAddress; // Store i2c address for alphanumeric RPM display
}

void elsControlPanel::init() {
    alpha4.begin(rpmReadouti2cAddress); // Initialize with whatever configured address (default is 0x70, per adafruit)
    tft.begin();
}

// Initial loading screen
void elsControlPanel::TFT_splashscreen() {
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(3);
    tft.println("  Electroturn\n");
    tft.print("  Electronic\n  Leadscrew\n");
    delay(3000);
    tft.setRotation(0);
    tft.fillScreen(ILI9341_WHITE);
}

// button1text controls units, button2text controls hardinge threading, button3text controls nothing currently
void elsControlPanel::TFT_writeGearboxInfo(enum ELSMode, Pitch currentPitch, String button1text, String button2text, String button3text) {
    //  tft.fillScreen(ILI9341_CYAN);
    //  delay(2000);
    tft.fillScreen(ILI9341_WHITE);
    tft.setCursor(0, 0);
    tft.setTextColor(ILI9341_BLACK);
    tft.setTextSize(3);
    tft.setTextWrap(false);

        if (ELSMode == "Power Feed") {
            tft.println("\n  Power Feed\n\n");
            if (currentPitch.units == tpi) {
                tft.print("  ");
                tft.setTextSize(4);
                tft.print(currentPitch.value);
                tft.println("in\n\n");
                tft.setTextSize(3);
            }
            else if (currentPitch.units == mm ) {
                tft.print("  ");
                tft.setTextSize(4);
                tft.print(currentPitch.value);
                tft.println("mm\n\n");
                tft.setTextSize(3);
            }
            tft.println("    mm/in");
        }
        else if (mode == "Threading") {
            if (button2text == "Rapid Right") {
                tft.println("\n  Threading\n\n");
                if (currentPitch.units == tpi) {
                    tft.print("  ");
                    tft.setTextSize(4);
                    tft.print(currentPitch.value);  
                    tft.println("TPI\n\n");
                    tft.setTextSize(3);
                    tft.println("    mm/in\n\n\n Rapid Right");
                }
                else if (currentPitch.units == mm) {
                    tft.print("  ");
                    tft.setTextSize(4);
                    tft.print(currentPitch.value);
                    tft.println("mm\n\n");
                    tft.setTextSize(3);
                    tft.println("    mm/in\n\n\n Rapid Right");
                }
            }
            else if (button2text == "Rapid Left") {
                tft.println("\n  Threading\n\n");
                if (currentPitch.units == tpi) {
                    tft.print("  ");
                    tft.setTextSize(4);
                    tft.print(currentPitch.value);  
                    tft.println("TPI\n\n");
                    tft.setTextSize(3);
                    tft.println("    mm/in\n\n\n  Rapid Left");
                }
                else if (currentPitch.units == mm) {
                    tft.print("  ");
                    tft.setTextSize(4);
                    tft.print(currentPitch.value);
                    tft.println("mm\n\n");
                    tft.setTextSize(3);
                    tft.println("   mm/in\n\n\n  Rapid Left");
                }

            }
            else {
                tft.println("\n  Threading\n\n");
                tft.print("  ");
                if (currentPitch.units == tpi) {
                    tft.setTextSize(4);
                    tft.print(currentPitch.value);  
                    tft.print("TPI\n\n");
                    tft.setTextSize(3);
                    tft.println("    mm/in\n\n\n  Rapid Off");
                }
                else if (currentPitch.units == mm) {
                    tft.setTextSize(4);
                    tft.print(currentPitch.value);
                    tft.println("mm\n\n");
                    tft.setTextSize(3);
                    tft.println("    mm/in\n\n\n  Rapid Off");
                }
            }
        }

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
