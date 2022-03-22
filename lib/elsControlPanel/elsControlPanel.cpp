
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

// button1text controls units, button2text controls hardinge threading, button3text controls nothing currently
void elsControlPanel::TFT_writeGearboxInfo(String mode, Pitch currentPitch, String button1text, String button2text, String button3text) {
     tft.fillScreen(ILI9341_WHITE);
     tft.setCursor(0, 0);
     tft.setTextColor(ILI9341_BLACK);
     tft.setTextSize(2);

        if (mode == "Power Feed") {
            Serial.println("Power Feed\n");
            if (currentPitch.units == tpi) {
                Serial.print(currentPitch.value);
                Serial.println("in/REV\n");
            }
            else if (currentPitch.units == mm ) {
                Serial.print(currentPitch.value);
                Serial.println("mm/REV\n");
            }
            Serial.println("mm/in");
        }
        else if (mode == "Threading") {
            if (button2text == "Rapid Right") {
                Serial.println("Threading\n");
                if (currentPitch.units == tpi) {
                    Serial.print(currentPitch.value);  
                    Serial.println("TPI\n");
                }
                else if (currentPitch.units == mm) {
                    Serial.print(currentPitch.value);
                    Serial.println("mm\n");
                }
                Serial.print("Rapid Right");
            }
            else if (button2text == "Rapid Left") {
                Serial.println("Threading\n");
                if (currentPitch.units == tpi) {
                    Serial.print(currentPitch.value);  
                    Serial.println("TPI\n");
                }
                else if (currentPitch.units == mm) {
                    Serial.print(currentPitch.value);
                    Serial.println("mm\n");
                }
                Serial.print("Rapid Left");
            }
            else {
                Serial.println("Threading\n");
                if (currentPitch.units == tpi) {
                    Serial.print(currentPitch.value);  
                    Serial.println("TPI\n");
                }
                else if (currentPitch.units == mm) {
                    Serial.print(currentPitch.value);
                    Serial.println("mm\n");
                }
                Serial.print("Rapid Off"); 
            }
        }

}

void elsControlPanel::alphanum_writeRPM(unsigned int rpm) {
    
    rpmReadout.print(250, 10);
    rpmReadout.writeDisplay();
    delay(2000);
    rpmReadout.print(9999, 10);
    rpmReadout.writeDisplay();
    delay(2000);
    rpmReadout.print(0022, 10);
    rpmReadout.writeDisplay();
    delay(2000);
    rpmReadout.print(rpm, 10);

}