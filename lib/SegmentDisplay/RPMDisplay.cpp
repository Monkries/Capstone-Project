#include Adafruit LED Backpack Library


void RPMDisplay() {
Adafruit_7segment numdisplay = Adafruit_7segment();

//Write Digit x with number
numdisplay.writeDigitnum(0,1);
numdisplay.writeDigitnum(1,1);
numdisplay.writeDigitnum(2,1);
numdisplay.writeDigitnum(3,1);

}
