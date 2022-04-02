#include "Arduino.h"
#include "AccelStepper.h"
#include "IntegerStepHelper.h"
#include "SynchronousClutch.h"

SynchronousClutch::SynchronousClutch(float stepsPerRev_arg) {
    stepsPerRev = stepsPerRev_arg;
}

int SynchronousClutch::move(float inputSteps) {
    // We'll return the relative shaft output movement (in steps, after using IntegerStepHelper stepsQueue), in this variable
    int outputShaftMovement=0;

    // Move input shafts to forward and reverse clutches
    fwdInputAngle = constrainShaftAngle(fwdInputAngle+inputSteps);
    revInputAngle = constrainShaftAngle(revInputAngle-inputSteps);

    if (fwdEngaged) {
        //We are using the forward clutch

        // Determine if it's locked or not
        if (fwdLocked) {
            Serial.print("Forward clutch locked, ");
            // The clutch is already locked, so simply move the output shaft forward
            stepQueue.totalValue += inputSteps;
        }
        else {
            // The clutch is engaged, but not locked yet (we are in the "waiting for sync" mode)
            Serial.print("Forward clutch checking for lock, ");
            // See if we are hitting the lock point
            if (abs(fwdInputAngle - outputAngle) < 20) {
                fwdLocked = true;
                Serial.print("locked!");
            }
        }
    }
    else if (revEngaged) {
        // We are using the reverse clutch

        // Determine if it's locked or not
        if (revLocked) {
            Serial.print("Reverse clutch locked, ");
            // The clutch is already locked, so simply move the output shaft backward
            stepQueue.totalValue -= inputSteps;
        }
        else {
            // The clutch is engaged, but not locked yet (we are in the "waiting for sync" mode)
            Serial.print("Reverse clutch checking for lock, ");
            // See if we are hitting the lock point
            if (abs(revInputAngle - outputAngle) < 20) {
                revLocked = true;
                Serial.print("locked!");
            }
        }
    }

    // Accounting for non-integer step amounts (we only return the integer amount, and save "leftovers" in stepQueue)
    outputShaftMovement = stepQueue.popIntegerPart();

    // Adjust output shaft angle for outputShaftMovement
    outputAngle = constrainShaftAngle(outputAngle+(float)outputShaftMovement);

    // Print a shit ton of clutch stats
    Serial.print("fIn=");
    Serial.print(fwdInputAngle);
    Serial.print(", rIn=");
    Serial.print(revInputAngle);
    Serial.print(", Out=");
    Serial.println(outputAngle);

    return outputShaftMovement;
}

void SynchronousClutch::disengage(){
    fwdEngaged = false;
    fwdLocked = false;
    revEngaged = false;
    revLocked = false;
}

void SynchronousClutch::engageForward(){
    fwdEngaged = true;
}

void SynchronousClutch::engageReverse(){
    revEngaged = true;
}

float SynchronousClutch::constrainShaftAngle(float input) {
    float output = fmod(input, stepsPerRev); // This will get us part of the way there, but allows negative angles (meaning -90 and 270 are considered different)
    if (output<0) {
        output+=stepsPerRev; // Force it to be positive
    }
    return output;
}