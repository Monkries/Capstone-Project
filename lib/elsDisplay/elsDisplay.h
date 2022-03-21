#ifndef elsDisplay_h
#define elseDisplay_h

#include "TeensyLeadscrew.h"
#include "Arduino.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9340.h"
#include "Adafruit_ILI9341.h"
#include "SPI.h"

class elsDisplay {
    public:
        elsDisplay();
        void TextDisplay(unsigned int mode, int button, Pitch currentPitch);
        void RPMDisplay(unsigned int rpm);
};
#endif