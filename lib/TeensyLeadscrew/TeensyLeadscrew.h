#ifndef TeensyLeadscrew_h
#define TeensyLeadscrew_h

#include "Arduino.h"
#include "AccelStepper.h"
#include "QuadEncoder.h"
#include "EncoderTach.h"

class TeensyLeadscrew {
    public:
    TeensyLeadscrew(QuadEncoder spindleEncoder, unsigned int encTicksPerRev, float spindlePulleyRatio,
        AccelStepper zStepper, unsigned int stepsPerRev, float leadscrewPitch, String leadscrewPitch_units);
    
    // Gearbox setup functions
    void configureGearing(float pitch, String units);
    void configureClutch(bool rapidLeft, bool rapidRight, unsigned int rapidStepRate);
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

    // Hardware Info
    unsigned int cfg_encTicksPerRev;
    float cfg_spindlePulleyRatio;
    unsigned int cfg_stepsPerRev;
    float cfg_leadscrewPitch;
    String cfg_leadscrewPitchUnits; // Could be "tpi", or "mm"

    // Gearbox Configuration Variables
    float gearbox_pitch;
    String gearbox_pitchUnits; // Could be "tpi", "in/rev", or "mm"
    bool gearbox_rapidLeft;
    bool gearbox_rapidRight;
    unsigned int gearbox_rapidStepRate;
    bool gearbox_enableMotorBraking;
    bool gearbox_suppress;

    private:
    int hypotheticalLeadscrewPosition; // measured in motor steps, relative to real leadscrew position (our zero point)
    int zFeedDirection; // TODO: rewrite so 0=neutral, 1=left, -1=right
    int zFeedDirection_previousCycle; // Same variable as above, but value from 1 cycle before
    bool waitingForClutch;

    int calculateMotorSteps(int encoderTicks); // Given encoder movement (and gearbox settings configured elsewhere) find number of steps to move, INDEPENDENT OF DIRECTION

}

#endif