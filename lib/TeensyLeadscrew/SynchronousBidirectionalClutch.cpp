#include "Arduino.h"
#include "AccelStepper.h"
#include "IntegerStepHelper.h"
#include "SynchronousBidirectionalClutch.h"

SynchronousBidirectionalClutch::SynchronousBidirectionalClutch(float stepsPerRev_arg, float lockTolerance_arg) {
    _stepsPerRev = stepsPerRev_arg;
    lockTolerance = lockTolerance_arg;
}

int SynchronousBidirectionalClutch::moveInput(float inputSteps) {
    // We'll return the relative shaft output movement (in steps, after using IntegerStepHelper stepsQueue), in this variable
    int outputShaftMovement=0;

    // Move input shafts to forward and reverse clutches
    _clutchState.forwardInputShaftAngle = constrainShaftAngle(_clutchState.forwardInputShaftAngle+inputSteps);
    _clutchState.reverseInputShaftAngle = constrainShaftAngle(_clutchState.reverseInputShaftAngle-inputSteps);

    if (_clutchState.forwardEngaged) {
        //We are using the forward clutch

        // Determine if it's locked or not
        if (_clutchState.forwardLocked) {
            Serial.print("Forward clutch locked, ");
            // The clutch is already locked, so simply move the output shaft forward
            _stepQueue.totalValue += inputSteps;
        }
        else {
            // The clutch is engaged, but not locked yet (we are in the "waiting for sync" mode)
            Serial.print("Forward clutch checking for lock, ");
            // See if we are hitting the lock point
            if (constrainShaftAngle(_clutchState.forwardInputShaftAngle - _clutchState.outputShaftAngle) < lockTolerance) {
                // Now we know we are within the acceptable range for locking the clutch

                // At this point, we need to jog the output shaft slightly for perfect alignment
                float alignmentSteps = constrainShaftAngle(_clutchState.forwardInputShaftAngle - _clutchState.outputShaftAngle);
                Serial.print(alignmentSteps);
                Serial.print(" steps to align, ");
                _stepQueue.totalValue += alignmentSteps;

                _clutchState.forwardLocked = true;
                Serial.print("locked!");
            }
        }
    }
    else if (_clutchState.reverseEngaged) {
        // We are using the reverse clutch

        // Determine if it's locked or not
        if (_clutchState.reverseLocked) {
            Serial.print("Reverse clutch locked, ");
            // The clutch is already locked, so simply move the output shaft backward
            _stepQueue.totalValue -= inputSteps;
        }
        else {
            // The clutch is engaged, but not locked yet (we are in the "waiting for sync" mode)
            Serial.print("Reverse clutch checking for lock, ");
            // See if we are hitting the lock point
            if (constrainShaftAngle(_clutchState.reverseInputShaftAngle - _clutchState.outputShaftAngle) < lockTolerance) {
                // Now we know we are within the acceptable range for locking the clutch

                // At this point, we need to jog the output shaft slightly for perfect alignment
                float alignmentSteps = constrainShaftAngle(_clutchState.forwardInputShaftAngle - _clutchState.outputShaftAngle);
                Serial.print(alignmentSteps);
                Serial.print(" steps to align, ");
                _stepQueue.totalValue -= alignmentSteps;
                
                _clutchState.reverseLocked = true;
                Serial.print("locked!");
            }
        }
    }

    // Accounting for non-integer step amounts (we only return the integer amount, and save "leftovers" in _stepQueue)
    outputShaftMovement = _stepQueue.popIntegerPart();

    // Adjust output shaft angle for outputShaftMovement
    _clutchState.outputShaftAngle = constrainShaftAngle(_clutchState.outputShaftAngle+(float)outputShaftMovement);

    // Print a shit ton of clutch stats
    Serial.print("fIn=");
    Serial.print(_clutchState.forwardInputShaftAngle);
    Serial.print(", rIn=");
    Serial.print(_clutchState.reverseInputShaftAngle);
    Serial.print(", Out=");
    Serial.println(_clutchState.outputShaftAngle);

    return outputShaftMovement;
}

void SynchronousBidirectionalClutch::disengage() {
    _clutchState.forwardEngaged = false;
    _clutchState.forwardLocked = false;
    _clutchState.reverseEngaged = false;
    _clutchState.reverseLocked = false;
}

void SynchronousBidirectionalClutch::engageForward() {
    if (_clutchState.forwardEngaged == false) {
        // Disengage everything else just to make sure
        disengage();
        // Now engage forward clutch
        _clutchState.forwardEngaged = true;
    }
}

void SynchronousBidirectionalClutch::engageReverse() {
    if (_clutchState.reverseEngaged == false) {
        // Disengage everything else just to make sure
        disengage();
        // Now engage forward clutch
        _clutchState.reverseEngaged = true;
    }
}

float SynchronousBidirectionalClutch::constrainShaftAngle(float input) {
    float output = fmod(input, _stepsPerRev); // This will get us part of the way there, but allows negative angles (meaning -90 and 270 are considered different)
    if (output<0) {
        output+=_stepsPerRev; // Force it to be positive
    }
    return output;
}