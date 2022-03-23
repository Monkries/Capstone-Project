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
    // Define major variables
    int encoderTicks;
    float stepsToMoveNow;

    // Read encoder movement since last cycle
    encoderTicks = spindleEncoder.read();
    // Then re-zero encoder
    spindleEncoder.write(0);
    // Cycle RPM calculator
    spindleTach.recordTicks(encoderTicks);
    
    // Look at feed lever and gearbox config, and determine stepper movement (if any)
    
    // If the feed lever is in center neutral position
    if (zFeedDirection == 0) {
        // If it was JUST put into neutral
        if (zFeedDirection_previousCycle != 0) {
            // Call the current leadscrew position our zero point
            hypotheticalLeadscrewPosition = 0;
            // TODO: Check if motor target position is way off in the distance compared to current position
        }
        // Calculate hypothetical position
        hypotheticalLeadscrewPosition += calculateMotorSteps(encoderTicks);
        // Handle zero rollover (this is an angle in steps)
        if (hypotheticalLeadscrewPosition >= (int)sysInfo.stepsPerRev) {
            hypotheticalLeadscrewPosition -= (int)sysInfo.stepsPerRev;
        }
    }
    else if (zFeedDirection == 1) {
        // If the operator has just engaged the Z feed, then assume virtual clutch needs time to align
        if (zFeedDirection_previousCycle == 0) {
            waitingForClutch = true;
        }

        // TODO: determine acceptable margin of error from 0
        if (hypotheticalLeadscrewPosition == 0) {
            waitingForClutch = false;
        }

        if (!waitingForClutch) {
            // Movement Calculation
            stepsToMoveNow = calculateMotorSteps(encoderTicks)*(float)zFeedDirection;
        }
    }
    else if (zFeedDirection == -1) {
        // If the operator has just engaged the Z feed, then assume virtual clutch needs time to align
        if (zFeedDirection_previousCycle == 0) {
            waitingForClutch = true;
        }

        // TODO: determine acceptable margin of error from 0
        if (hypotheticalLeadscrewPosition == 0) {
            waitingForClutch = false;
        }

        if (!waitingForClutch) {
            // Movement Calculation
            stepsToMoveNow = calculateMotorSteps(encoderTicks)*(float)zFeedDirection;
        }
    }

    // Store this cycle's feed direction (so we can check for state change on next cycle)
    zFeedDirection_previousCycle = zFeedDirection;

    // Deal with fractional "remainder" steps
    stepsToMoveNow = stepsToMove_accumulator + stepsToMoveNow; // Bring in remainder steps from previous cycles
    stepsToMove_accumulator = stepsToMoveNow - floor(stepsToMoveNow); // Shave "remainder steps" off this number, for later cycles
    stepsToMoveNow = floor(stepsToMoveNow); // Now permanently store this, so we can move an integer number of steps

    zStepper.move(stepsToMoveNow + zStepper.distanceToGo());
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
        cutterMovement_inches = ((float)encoderTicks) * (1./sysInfo.encoderPulleyMultiplier) * (1./(float)sysInfo.encoderTicksPerRev) * (1./gearbox_pitch.value); // TODO: pulley multiplier needed?
        
        if (false) {
        Serial.print("encoderTicks = ");
        Serial.println(encoderTicks);
        Serial.print("encoderTicksPerRev = ");
        Serial.println(sysInfo.encoderTicksPerRev);
        Serial.print("gearbox_pitch.value = ");
        Serial.println(gearbox_pitch.value);
        }

    }
    else if (gearbox_pitch.units == in_per_rev) {
        cutterMovement_inches = 0; // TODO
    }
    else if (gearbox_pitch.units == mm) {
        cutterMovement_inches = 0; // TODO
    }

    // Now go from cutter movement to leadscrew motor steps
    float stepsToMove;
    
    // If the leadscrew is imperial
    if (sysInfo.leadscrewPitch.units == tpi) {
        stepsToMove = cutterMovement_inches * (sysInfo.leadscrewPitch.value) * ((float)sysInfo.stepsPerRev);
    }
    // If the leadscrew is metric
    else if (sysInfo.leadscrewPitch.units == mm) { 
        stepsToMove = 0; //TODO
    }
    // If the leadscrew is being given in inches-per-revolution, which shouldn't ever happen
    else {
        // Just in case leadscrewPitch.units gets set to in_per_rev (which is nonsense),
        // then do nothing (because clearly something's not right)
        return 0;
    }

    if (stepsToMove != 0) {
        Serial.print("stepsToMove = ");
        Serial.println(stepsToMove);
    }
    return stepsToMove; // TODO: Handle direction
}
