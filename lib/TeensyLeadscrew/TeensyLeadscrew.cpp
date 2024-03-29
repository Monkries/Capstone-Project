#include "Arduino.h"
#include "AccelStepper.h"
#include "QuadEncoder.h"
#include "EncoderTach.h"
#include "TeensyLeadscrew.h"
#include "SynchronousBidirectionalClutch.h"


TeensyLeadscrew::TeensyLeadscrew(QuadEncoder &arg_spindleEncoder,
AccelStepper &arg_zStepper,
LatheHardwareInfo arg_sysInfo,
unsigned int tachometerSamplePeriod_micros) :
spindleEncoder(arg_spindleEncoder), // This is reference stuff, which sets spindleEncoder to arg_spindleEncoder
zStepper(arg_zStepper) // Same as above. If you don't have this, it tries to copy the motor/encoder objects and hangs the whole teensy
{
        // Store hardware specs
        sysInfo = arg_sysInfo;
        // Set default gearbox state
        gearbox.rapidReturn = false;
        gearbox.enableMotorBraking = true;
        gearbox.disable = false;
        gearbox.rapidStepRate = 500; // TODO: how should we handle this?
        gearbox.configuredPitch.value = 0.001; // Default to 0.001"/REV feed rate, feeding left
        gearbox.configuredPitch.units = in_per_rev;
        gearbox.configuredPitch.direction = rightHandThread_feedLeft;
        
        clutch = SynchronousBidirectionalClutch(sysInfo.encoderTicksPerRev*sysInfo.encoderPulleyMultiplier, 5); // Create clutch object with SPINDLE ticks per rev and locking tolerance
}

void TeensyLeadscrew::init() {
    spindleTach = EncoderTach(100, sysInfo.encoderTicksPerRev, sysInfo.encoderPulleyMultiplier);
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

    // 1. Overspeed Safety Handling

    // Should we start a new leadscrewOverspeedAlarm ?
    if (zStepper.speed() > sysInfo.warningStepRate) {
        leadscrewOverspeedAlarmActive = true;
    }
    // Should we reset the alarm (if the spindle has stopped)?
    else if (leadscrewOverspeedAlarmActive == true && spindleTach.getRPM() == 0) {
        leadscrewOverspeedAlarmActive = false;
    }

    // 2. Actual Leadscrew Math
    if (leadscrewOverspeedAlarmActive == true) {
        /* Overspeed Condition:
        If the leadscrewOverspeed alarm has been tripped, we want to stop the carriage.
        Yes, this could mess up a part and maybe even break a tool, but it will prevent much more dangerous crashes/behavior.
        */
       clutch.disengage();
    }
    else {
        // Normal operating condition

        // Spindle --> Single-Tooth Clutch --> Threading Gearbox
        // Note that for the clutch, encoder ticks become spindle ticks automatically, because it knows how many encoder ticks make one spindle revolution
        // So even though we feed in encoder ticks, from an angular perspective, the clutch is looking at the spindle, not the encoder
        queuedMotorSteps.totalValue += calculateMotorSteps( clutch.moveInput(relativeEncoderMovement) );

        zStepper.move((long)queuedMotorSteps.popIntegerPart()+zStepper.distanceToGo());
    }

    // 3. Motor Movement

    // If the motor doesn't have any steps to go, AND motor braking at idle has been disabled, then disable the drive
    if ( gearbox.enableMotorBraking == false && zStepper.distanceToGo() == 0) {
        zStepper.disableOutputs();
    }
    else {
        // Otherwise, call zStepper.run()
        // (even if there are no steps needed, it will just quit and do nothing)
        zStepper.enableOutputs();
        zStepper.run();
    }
}

// Given the class's current gearbox config
// and the number of spindle encoder ticks on this cycle,
// find the number of motor steps the leadscrew should move, ignoring any clutch action or reversing
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

    float cutterMovement_inches; // positive means toward the headstock
    // If we're working in TPI
    if (gearbox.configuredPitch.units == tpi) {
        cutterMovement_inches = ((float)encoderTicks) * (1./sysInfo.encoderPulleyMultiplier) * (1./(float)sysInfo.encoderTicksPerRev) * (1./gearbox.configuredPitch.value);
    }
    else if (gearbox.configuredPitch.units == in_per_rev) {
        cutterMovement_inches = ((float)encoderTicks) * (1./sysInfo.encoderTicksPerRev) * (1./sysInfo.encoderPulleyMultiplier) * (gearbox.configuredPitch.value);
    }
    else if (gearbox.configuredPitch.units == mm) {
        cutterMovement_inches = ((float)encoderTicks) * (1./sysInfo.encoderTicksPerRev) * (1./sysInfo.encoderPulleyMultiplier) * (gearbox.configuredPitch.value) * (1./25.4);
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

    return stepsToMove;
}
