#include "Arduino.h"
#include "AccelStepper.h"
#include "QuadEncoder.h"
#include "EncoderTach.h"
#include "TeensyLeadscrew.h"


TeensyLeadscrew::TeensyLeadscrew(QuadEncoder &arg_spindleEncoder,
AccelStepper &arg_zStepper,
LatheHardwareInfo arg_sysInfo,
unsigned int tachometerSamplePeriod_micros) :
spindleEncoder(arg_spindleEncoder), // This is reference stuff, which sets spindleEncoder to arg_spindleEncoder
zStepper(arg_zStepper) // Same as above. If you don't have this, it tries to copy the motor/encoder objects and hangs the whole teensy
{
        // Store hardware specs
        sysInfo = arg_sysInfo;
}

void TeensyLeadscrew::init() {
    spindleTach = EncoderTach(100, sysInfo.encoderTicksPerRev, sysInfo.encoderPulleyMultiplier);
}

// Z Feed "Lever" Control (modeled after HLV-H leadscrew clutch lever)

void TeensyLeadscrew::disengageZFeed() {
    zFeedDirection = 0;
}

void TeensyLeadscrew::engageZFeedLeft() {
    zFeedDirection = 1;
}

void TeensyLeadscrew::engageZFeedRight() {
    zFeedDirection = -1;
}

// Handles one "cycle" of actual leadscrew movement
// Reads the encoder object stored in the class, and moves the stepper motor accordingly
// Call as often as possible
void TeensyLeadscrew::cycle() {
    // Read encoder movement since last cycle
    int relativeEncoderMovement = spindleEncoder.read();
    // Then re-zero encoder
    if (relativeEncoderMovement!=0) {
        // This whole IF statement seems silly, but this is REALLY important
        // If you just zero the encoder every time, you lose whatever fractional step is hanging in the balance
        // (due to the rising and falling quadrature pulses)
        // Don't get rid of this!!
        spindleEncoder.write(0);
    }
    // Cycle RPM calculator
    spindleTach.recordTicks(relativeEncoderMovement);

    // BUSINESS

    // Move the clutch input shaft (and normalize the value as an angle in steps)
    clutchState.inputShaftAngle = fmod( (clutchState.inputShaftAngle + calculateMotorSteps(relativeEncoderMovement)), sysInfo.stepsPerRev); // TODO: clarify this by moving to a function

    // If the clutch is already locked, just go ahead and move the motor
    if (clutchState.engaged && clutchState.locked) {
        // Move motor as normal
        queuedMotorSteps.totalValue += calculateMotorSteps(relativeEncoderMovement);
    }
    // If the clutch isn't locked, but is engaged, see if it lines up on this cycle (meaning we start movement on the next one)
    else if (clutchState.engaged && !clutchState.locked) {
        // We're waiting for the clutch to align
        if (clutchState.inputShaftAngle == 0) {
            // Clutch is aligned
            clutchState.locked = true;
        }
    }

    // Store this clutch state for next cycle
    lastClutchState = clutchState;

    // stats for debugging
    encoderTicksRecorded = encoderTicksRecorded + relativeEncoderMovement;

    zStepper.move((long)queuedMotorSteps.popIntegerPart()+zStepper.distanceToGo());

    zStepper.run();
}

// Given the class's current gearbox config
// and the number of spindle encoder ticks on this cycle,
// find the number of motor steps the leadscrew should move, ignoring any clutch action
float TeensyLeadscrew::calculateMotorSteps(int encoderTicks) {
    /*
    Math Example: Cutting a 13TPI thread on a 20TPI leadscrew

    (Encoder Counts) | 1 spindle rev | 1in cutter movement | 20 leadscrew revs | 200 steps
    ---------------------------------------------------------------------------------------------
                     | 1000 counts   | 13 spindle revs     | 1 in. tool travel | 1 leadscrew rev

    For good coding structure, we split the math HERE -----^

    1. Find desired cutter movement, in inches
    2. Use cutter movement to calculate leadscrew rotation, in steps
    */

    // So first, we'll find the desired cutter movement (in inches)
    float cutterMovement_inches;
    // If we're working in TPI
    if (gearbox_pitch.units == tpi) {
        cutterMovement_inches = ((float)encoderTicks) * (1./sysInfo.encoderPulleyMultiplier) * (1./(float)sysInfo.encoderTicksPerRev) * (1./gearbox_pitch.value);
    }
    else if (gearbox_pitch.units == in_per_rev) {
        cutterMovement_inches = ((float)encoderTicks) * (1./sysInfo.encoderTicksPerRev) * (1./sysInfo.encoderPulleyMultiplier) * (gearbox_pitch.value);
    }
    else if (gearbox_pitch.units == mm) {
        cutterMovement_inches = ((float)encoderTicks) * (1./sysInfo.encoderTicksPerRev) * (1./sysInfo.encoderPulleyMultiplier) * (gearbox_pitch.value) * (1./25.4);
    }

    // Now go from cutter movement to leadscrew motor steps
    float stepsToMove;
    
    // If the leadscrew is imperial
    if (sysInfo.leadscrewPitch.units == tpi) {
        stepsToMove = cutterMovement_inches * (sysInfo.leadscrewPitch.value) * ((float)sysInfo.stepsPerRev);
    }
    // If the leadscrew is metric
    else if (sysInfo.leadscrewPitch.units == mm) { 
        stepsToMove = cutterMovement_inches * (25.4) * (1./sysInfo.leadscrewPitch.value) * ((float)sysInfo.stepsPerRev);
    }
    // If the leadscrew is being given in inches-per-revolution, which shouldn't ever happen
    else {
        // Just in case leadscrewPitch.units gets set to in_per_rev (which is nonsense),
        // then do nothing (because clearly something's not right)
        return 0;
    }

    return stepsToMove; // TODO: Handle direction
}
