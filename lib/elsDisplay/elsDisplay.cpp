
#include "Arduino.h"
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9340.h"
#include "Adafruit_ILI9341.h"
#include "SPI.h"
#include "elsDisplay.h"
#include "TeensyLeadscrew.h"

elsDisplay::elsDisplay() {
    //temporary blank constructor
}

// Display text menu for user selectin of different settings
void elsDisplay::TextDisplay(unsigned int mode, int button, Pitch currentPitch) {
    //Adafruit_ILI9340 tft = Adafruit_ILI9340()
    // tft.fillscreen(ILI9340_WHITE);
    // tft.setcursor(0, 0);
    // tft.setTextColor(ILI9340_BLACK);
    // tft.setTextSize(2);

    switch (mode) {
        case 0:
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
            break;
        case 1:
            if (button == 1) {
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
                break;
            }
            else if (button == -1) {
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
                break;
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
                break;  
            }
    }

}

// Display RPM from encoder on the 7 segment display
void elsDisplay::RPMDisplay(unsigned int rpm) {
    Adafruit_7segment numdisplay = Adafruit_7segment();

    //Write Digit x with number
    numdisplay.print(250, DEC);
    numdisplay.writeDisplay();
    delay(2000);
    numdisplay.writeDigitNum(0,1);
    numdisplay.writeDigitNum(1,1);
    numdisplay.writeDigitNum(2,1);
    numdisplay.writeDigitNum(3,1);
    numdisplay.writeDisplay();
    delay(2000);
    numdisplay.writeDigitNum(0,5);
    numdisplay.writeDigitNum(1,2);
    numdisplay.writeDigitNum(2,9);
    numdisplay.writeDigitNum(3,6);
    numdisplay.writeDisplay();
}

