
#include "Arduino.h"
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "SPI.h"
#include "elsControlPanel.h"
#include "TeensyLeadscrew.h"
#include "Wire.h"
#include "Adafruit_MCP23X17.h"

static unsigned int MAX_RPM = 3000; 

elsControlPanel::elsControlPanel(Adafruit_ILI9341 &arg_tftObject, QuadEncoder &arg_encoder, uint8_t arg_rpmReadouti2cAddress) :
tft(arg_tftObject), // Properly pass in references to TFT object, encoder object
encoder(arg_encoder)
{
    rpmReadouti2cAddress = arg_rpmReadouti2cAddress; // Store i2c address for alphanumeric RPM display
}

void elsControlPanel::init() {
    alpha4.begin(rpmReadouti2cAddress); 
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
    tft.fillScreen(ILI9341_WHITE); // Clear screen
}

void elsControlPanel::TFT_writeGearboxInfo(bool Thread, bool Power, Pitch currentPitch, bool rapidLeftEnabled, bool rapidRightEnabled, String button3text) { // Button3text for future additions
    tft.setCursor(0, 0);
    tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
    tft.setTextSize(3);
    tft.setTextWrap(false);

        if (Power) {
            tft.println("\n  Power Feed\n\n");
            if (currentPitch.units == tpi) {
                tft.print("  ");
                tft.setTextSize(4);
                tft.print(currentPitch.value);
                tft.println("TPI   ");
                tft.print("\n");
                tft.setTextSize(3);
            }
            else if (currentPitch.units == mm ) {
                tft.print("  ");
                tft.setTextSize(4);
                tft.print(currentPitch.value);
                tft.println("mm   ");
                tft.print("\n");
                tft.setTextSize(3);
            }
            tft.println("    mm/in");
            tft.println("\n\n                   ");
        }
        else if (Thread) {

            // If rapid right is enabled
            if (rapidRightEnabled) {
                tft.println("\n  Threading  \n\n");
                if (currentPitch.units == tpi) {
                    tft.print("  ");
                    tft.setTextSize(4);
                    tft.print(currentPitch.value);  
                    tft.println("TPI   \n");
                    tft.setTextSize(3);
                    tft.println("    mm/in\n\n\n Rapid Right");
                }
                else if (currentPitch.units == mm) {
                    tft.print("  ");
                    tft.setTextSize(4);
                    tft.print(currentPitch.value);
                    tft.println("mm   \n");
                    tft.setTextSize(3);
                    tft.println("    mm/in\n\n\n Rapid Right");
                }
            }
            // If rapid left is enabled
            else if (rapidLeftEnabled) {
                tft.println("\n  Threading  \n\n");
                if (currentPitch.units == tpi) {
                    tft.print("  ");
                    tft.setTextSize(4);
                    tft.print(currentPitch.value);  
                    tft.println("TPI   \n");
                    tft.setTextSize(3);
                    tft.println("    mm/in\n\n\n  Rapid Left");
                }
                else if (currentPitch.units == mm) {
                    tft.print("  ");
                    tft.setTextSize(4);
                    tft.print(currentPitch.value);
                    tft.println("mm   \n");
                    tft.setTextSize(3);
                    tft.println("    mm/in\n\n\n  Rapid Left");
                }

            }
            // If no rapids are enabled
            else if (!rapidRightEnabled && !rapidLeftEnabled) {
                tft.println("\n  Threading     \n\n");
                tft.print("  ");
                if (currentPitch.units == tpi) {
                    tft.setTextSize(4);
                    tft.print(currentPitch.value);  
                    tft.print("TPI   \n\n");
                    tft.setTextSize(3);
                    tft.println("    mm/in\n\n\n  Rapid Off");
                }
                else if (currentPitch.units == mm) {
                    tft.setTextSize(4);
                    tft.print(currentPitch.value);
                    tft.println("mm   \n");
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

void elsControlPanel::writeOverspeedLED(unsigned int rpm){
    if (rpm > MAX_RPM) {
        //LED ON
    }
}
