#ifndef IntegerStepHelper_h
#define IntegerStepHelper_h

#include <Arduino.h>

class IntegerStepHelper {
    public:
    IntegerStepHelper(float startingValue = 0);

    // The actual value we're storing
    float totalValue;

    // Returns just the integer portion of the value (without editing the value itself)
    int getIntegerPart();

    // Returns just the fractional portion of the value (without editing the value itself)
    float getFractionalPart();

    // Returns the integer portion of the value, and also subtracts it from the value (leaving only the fractional portion)
    int popIntegerPart();

    // Returns the fractional portion of the value, and also subtracts it from the value (leaving only the integer portion)
    float popFractionalPart();
};

#endif