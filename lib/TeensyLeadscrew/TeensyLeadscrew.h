#ifndef TeensyLeadscrew_h
#define TeensyLeadscrew_h

#include "Arduino.h"
#include "AccelStepper.h"
#include "QuadEncoder.h"
#include "EncoderTach.h"

class TeensyLeadscrew {
    public:
    TeensyLeadscrew();

    // System configuration
    void setup(QuadEncoder spindleEncoder, int ticksPerRev, float spindlePulleyRatio, AccelStepper zStepper, int stepsPerRev, float leadscrewPitch, String leadscrewPitch_units);
    
    // Gearbox setup functions
    void configureGearing(float pitch, String units);
    void configureClutch(bool enableSynchronousClutch, bool rapidLeft, bool rapidRight, int rapidStepRate);
    void configureMotorBraking(bool enable);
    void suppress(bool suppressGearbox);

    // Feed controls
    void engageZFeedLeft();
    void engageZFeedRight();
    void disengageZFeed();

    void cycle();

    void getSpindleRPM();

    // Hardware Control (not used in most situations but still made publicly accessible)
    QuadEncoder spindleEncoder;
    AccelStepper zStepper;
}