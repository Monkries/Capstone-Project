
#include "TeensyLeadscrew.h"
#include <Arduino.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9340.h"
#include "Adafruit_ILI9341.h"
#include <SPI.h>

unsigned long UIText(int mode, int button, Pitch currentPitch) {
    //tft.fillscreen(ILI9340_WHITE);
    //tft.setcursor(0, 0);
    //tft.setTextColor(ILI9340_BLACK);
    //tft.setTextSize(2);


    switch (mode) { //is in per rev needed for currentPitch in TeensyLeadscrew.h? based off of below
        case 0:
            Serial.println("Power Feed\n");
            if (currentPitch.units == tpi) {
                Serial.print(currentPitch.value);
                Serial.println("in/REV\n");
            }
            else if (currentPitch.units == mm ){
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

