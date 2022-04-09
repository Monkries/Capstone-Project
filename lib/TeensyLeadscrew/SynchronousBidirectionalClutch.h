#ifndef SynchronousClutch_h
#define SynchronousClutch_h

#include "Arduino.h"
#include "AccelStepper.h"
#include "IntegerStepHelper.h"

// @brief Simple struct to hold the state of the mechanical model behind the SynchronousBidirectionalClutch class
struct BidirectionalClutchStatus {
    // @brief Angle of forward side input shaft (see README), in steps, constrained between 0 and 1 revolution of steps
    float forwardInputShaftAngle;
    // @brief True if forward clutch is engaged (either locked already or attempting to lock)
    bool forwardEngaged;
    // @brief True if forward clutch is locked
    bool forwardLocked;

    // @brief Angle of the reverse side input shaft (see README), in steps, constrained between 0 and 1 revolution of steps
    float reverseInputShaftAngle;
    // @brief True if reverse clutch is engaged (either locked already or attempting to lock)
    bool reverseEngaged;
    // @brief True if reverse clutch is locked
    bool reverseLocked;

    // @brief Angle of the output shaft (see README), in steps
    float outputShaftAngle;
};

/*
A synchronous clutch, usable in FWD and REV, meant for use with stepper motors. Controls are based on the dog clutch found on Hardinge HLV-H lathes.

It's important to understand the underlying mechanical model, so see the README.
*/
class SynchronousBidirectionalClutch {
    public:

    /*
    @brief Create a new SynchronousBidirectionalClutch, with shaft angles measured according to the given stepsPerRev, and the given locking tolerance
    @param stepsPerRev The number of steps in one revolution of the motor. Shaft angles are constrained between 0 and stepsPerRev.
    @param lockTolerance How many steps away from perfect alignment is considered "close enough" to lock the clutch
    */
    SynchronousBidirectionalClutch(float stepsPerRev=2000.0, float lockTolerance=20.0);

    /*
    @brief Take in the movement of the input shaft, and return the necessary movement for the output shaft (based on clutch configuration/state).
    @param relativeSteps Movement (in steps) of the input shaft since the last call
    @return Steps to move the output shaft (always a neat integer, with accumulated rounding errors handled internally)
    */
    int moveInput(float relativeSteps);

    /*
    @brief Move the output shaft separately from the input shaft
    @param relativeSteps Amount of movement of output shaft
    @return True on success, false on fail (for instance, if the clutch is locked, and we can't move the output shaft separately)
    */
    bool moveOutputShaft(float relativeSteps);

    /*
    @brief Return the state of the clutch model
    */
    BidirectionalClutchStatus getClutchStatus();

    // @brief Engage the output shaft to the forward clutch (the output shaft won't move until sync/lock happens)
    void engageForward();
    // @brief Engage the output shaft to the reverse clutch (the output shaft won't move until sync/lock happens)
    void engageReverse();
    // @brief Disengage the output shaft from both clutches (the output shaft will immediately stop)
    void disengage();

    // When waiting for the clutch to lock, this determines how close to perfect alignment is considered "close enough"
    // 0 requires perfect alignment (not recommended)
    // The system will jog the motor to correct misalignment to avoid accumulated error over time
    float lockTolerance = 0;

    // Helpers/internal stuff
    private:
    // The steps/revolution we use to constrain shaft angles
    float _stepsPerRev;

    // Little helper class for handling fractional steps (prevents us from constantly "losing" 0.25 or 0.5 steps to rounding)
    IntegerStepHelper _stepQueue;

    /*
    @brief Force the input to be an angle between 0 steps and stepsPerRev steps
    @param input Un-constrained input steps
    @return Angle equivalent to the input, constrained between 0 and stepsPerRev (set in the constructor)
    */
    float constrainShaftAngle(float input);

    // Stores all the shaft angles and engagement/lock status for both the forward and reverse "sides" of the clutch
    BidirectionalClutchStatus _clutchState = {
        0, false, false, // Forward Input Shaft Initial State
        0, false, false, // Reverse Input Shaft Initial State
        0, // Output Shaft Initial State
    };
};

#endif