#ifndef SynchronousClutch_h
#define SynchronousClutch_h

#include "Arduino.h"
#include "AccelStepper.h"
#include "IntegerStepHelper.h"

class SynchronousClutch {
    public:
    SynchronousClutch(float stepsPerRev=2000.0);

    int move(float input_relativeSteps);

    void engageForward();
    void engageReverse();
    void disengage();

    private:
    // Helpers/internal stuff
    float stepsPerRev;
    IntegerStepHelper stepQueue;
    float constrainShaftAngle(float input);

    // Fwd clutch state
    float fwdInputAngle=0;
    bool fwdEngaged = false;
    bool fwdLocked = false;

    // Reverse clutch state
    float revInputAngle=0;
    bool revEngaged=false;
    bool revLocked=false;

    // Output shaft state
    float outputAngle=0;
};

#endif