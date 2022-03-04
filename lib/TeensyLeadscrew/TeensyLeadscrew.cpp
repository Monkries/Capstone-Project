#include "Arduino.h"
#include "AccelStepper.h"
#include "QuadEncoder.h"
#include "TeensyLeadscrew.h"

TeensyLeadscrew::TeensyLeadscrew(QuadEncoder arg_spindleEncoder, unsigned int ticksPerRev,
    float spindlePulleyRatio, AccelStepper arg_zStepper, unsigned int stepsPerRev,
    float leadscrewPitch, String leadscrewPitch_units) {
        // Store the stepper and encoder objects
        spindleEncoder = arg_spindleEncoder;
        zStepper = arg_zStepper;
        // Store hardware specs
        cfg_encTicksPerRev = ticksPerRev;
        cfg_spindlePulleyRatio = spindlePulleyRatio;
        cfg_stepsPerRev = stepsPerRev;
        cfg_leadscrewPitch = leadscrewPitch;
        cfg_leadscrewPitchUnits = leadscrewPitch_units;
}

int TeensyLeadscrew::cycle() {
    // Define major variables
    int encoderTicks;
    int stepsToMove;

    // Read encoder movement since last cycle
    encoderTicks = 10;
    // Cycle RPM calculator
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
        if (hypotheticalLeadscrewPosition >= cfg_stepsPerRev) {
            hypotheticalLeadscrewPosition -= cfg_stepsPerRev;
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
            stepsToMove = calculateMotorSteps(encoderTicks)*zFeedDirection;
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
            stepsToMove = calculateMotorSteps(encoderTicks)*zFeedDirection;
        }
    }

    // Store this cycle's feed direction
    zFeedDirection_previousCycle = zFeedDirection;


    // Tell stepper to move
    return stepsToMove;
}

float TeensyLeadscrew::calculateMotorSteps(int encoderTicks) {
    float ratio;    // Gear ratio
    float num;      // Numerator
    float denum;    // Denominator
    
    num = cfg_leadscrewPitch * cfg_stepsPerRev;     // Leadscrew pitch times the steps per revolution
    denum = cfg_encTicksPerRev * gearbox_pitch;     // Encoder ticks per revolution times the thread pitch either Imperial or Metric
    ratio = encoderTicks * (num / denum);           // The ratio is equal to the encoder ticks times the numerator/denominator
    
    return ratio;
}
