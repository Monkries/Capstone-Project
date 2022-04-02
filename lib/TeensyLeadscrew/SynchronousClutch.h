#ifndef SynchronousClutch_h
#define SynchronousClutch_h

#include "Arduino.h"
#include "AccelStepper.h"
#include "IntegerStepHelper.h"

class SynchronousClutch {
    public:
    SynchronousClutch(float stepsPerRev=2000.0);

    float stepsPerRev;
    IntegerStepHelper stepQueue;

    int move(float input_relativeSteps);

    void engageForward();
    void engageReverse();
    void disengage();

    float fwdInputAngle=0;
    bool fwdEngaged = false;
    bool fwdLocked = false;

    float revInputAngle=0;
    bool revEngaged=false;
    bool revLocked=false;
    float outputAngle=0;
};

#endif