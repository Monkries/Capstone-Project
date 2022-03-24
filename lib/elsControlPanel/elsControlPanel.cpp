
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
    rpmReadout.begin(rpmReadouti2cAddress); // Initialize with whatever configured address (default is 0x70, per adafruit)
    tft.begin(100);
}

// button1text controls units, button2text controls hardinge threading, button3text controls nothing currently
void elsControlPanel::TFT_writeGearboxInfo(String mode, Pitch currentPitch, String button1text, String button2text, String button3text) {
    //  tft.fillScreen(ILI9341_CYAN);
    //  delay(2000);
     tft.fillScreen(ILI9341_WHITE);
     tft.setCursor(0, 0);
     tft.setTextColor(ILI9341_BLACK);
     tft.setTextSize(3);

        if (mode == "Power Feed") {
            tft.println("  Power Feed\n\n");
            if (currentPitch.units == tpi) {
                tft.print(" ");
                tft.print(currentPitch.value);
                tft.println("in/REV\n\n");
            }
            else if (currentPitch.units == mm ) {
                tft.print(" ");
                tft.print(currentPitch.value);
                tft.println("mm/REV\n\n");
            }
            tft.println("   mm/in");
        }
        else if (mode == "Threading") {
            if (button2text == "Rapid Right") {
                tft.println("  Threading\n\n");
                if (currentPitch.units == tpi) {
                    tft.print("  ");
                    tft.print(currentPitch.value);  
                    tft.println("TPI\n\n");
                    tft.println("   mm/in\n\n Rapid Right");
                }
                else if (currentPitch.units == mm) {
                    tft.print("  ");
                    tft.print(currentPitch.value);
                    tft.println("mm\n\n");
                    tft.println("   mm/in\n\n Rapid Right");
                }
            }
            else if (button2text == "Rapid Left") {
                tft.println("  Threading\n\n");
                if (currentPitch.units == tpi) {
                    tft.print("  ");
                    tft.print(currentPitch.value);  
                    tft.println("TPI\n\n");
                    tft.println("   mm/in\n\n  Rapid Left");
                }
                else if (currentPitch.units == mm) {
                    tft.print("  ");
                    tft.print(currentPitch.value);
                    tft.println("mm\n\n");
                    tft.println("   mm/in\n\n  Rapid Left");
                }

            }
            else {
                tft.println("  Threading\n\n");
                tft.print("  ");
                if (currentPitch.units == tpi) {
                    tft.print(currentPitch.value);  
                    tft.println("TPI\n\n");
                    tft.println("   mm/in\n\n  Rapid Off");
                }
                else if (currentPitch.units == mm) {
                    tft.print(currentPitch.value);
                    tft.println("mm\n\n");
                    tft.println("   mm/in\n\n  Rapid Off");
                }
            }
        }

}

byte elsControlPanel::alphanum_writeRPM(unsigned int rpm) {
    //rpmReadout.blinkRate(2);
    rpmReadout.writeDigitRaw(0, 0x3fff);
    rpmReadout.writeDisplay();
    delay(2000);
    // rpmReadout.println(9999, 10);
    // rpmReadout.writeDisplay();
    // delay(2000);
    // rpmReadout.println(0022, 10);
    // rpmReadout.writeDisplay();
    // delay(2000);
    // Wire.beginTransmission(0x70);
    // Wire.write(rpm);
    // rpmReadout.println(rpm, 10);
    // rpmReadout.writeDisplay();
    // Wire.endTransmission();
    //return rpmReadout.blinkRate(2);
    //return digitalWrite(18, rpm);

}
