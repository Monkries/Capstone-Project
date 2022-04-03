#ifndef SynchronousClutch_h
#define SynchronousClutch_h

#include "Arduino.h"
#include "AccelStepper.h"
#include "IntegerStepHelper.h"

struct BidirectionalClutchStatus {
    float forwardInputShaftAngle;
    bool forwardEngaged;
    bool forwardLocked;

    float reverseInputShaftAngle;
    bool reverseEngaged;
    bool reverseLocked;

    float outputShaftAngle;
};

class SynchronousBidirectionalClutch {
    public:
    SynchronousBidirectionalClutch(float stepsPerRev=2000.0);

    // Move the "input shaft" on the virtual clutch
    int moveInput(float relativeSteps);

    // Move the output shaft separately from the input shaft, return true if successful (this will fail if the clutch is locked)
    bool moveOutputShaft(float relativeSteps);

    BidirectionalClutchStatus getClutchStatus();

    void engageForward();
    void engageReverse();
    void disengage();

    private:
    // Helpers/internal stuff
    float _stepsPerRev;
    IntegerStepHelper _stepQueue;
    float constrainShaftAngle(float input);

    // Stores all the shaft angles and engagement/lock status for both the forward and reverse "sides" of the clutch
    BidirectionalClutchStatus _clutchState = {
        0, false, false, // Forward Input Shaft Initial State
        0, false, false, // Reverse Input Shaft Initial State
        0, // Output Shaft Initial State
    };
};

#endif