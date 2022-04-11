#include "Arduino.h"
#include "IntegerStepHelper.h"

IntegerStepHelper::IntegerStepHelper(float startingValue = 0) {
    totalValue = startingValue;
}

int IntegerStepHelper::getIntegerPart() {
    double integerComponent;
    modf(totalValue, &integerComponent);
    return (int)integerComponent;
}

int IntegerStepHelper::popIntegerPart() {
    int integerPart = getIntegerPart();
    totalValue -= (float)integerPart;
    return integerPart;
}

float IntegerStepHelper::getFractionalPart() {
    double dummyIntegerComponentVariable;
    return (float)modf(totalValue, &dummyIntegerComponentVariable);
}

float IntegerStepHelper::popFractionalPart() {
    float fractionalPart = getFractionalPart();
    totalValue -= fractionalPart;
    return fractionalPart;
}