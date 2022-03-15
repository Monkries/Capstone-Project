#ifndef TeensyLeadscrew_h
#define TeensyLeadscrew_h

#include "Arduino.h"
#include "AccelStepper.h"
#include "QuadEncoder.h"
#include "EncoderTach.h"

class TeensyLeadscrew {
    public:
    TeensyLeadscrew(QuadEncoder spindleEncoder,
        AccelStepper zStepper, LatheHardwareInfo hardwareSpecs);
    
    // Gearbox setup functions
    void configureGearing(float pitch, String units);
    void configureClutch(bool rapidLeft, bool rapidRight, unsigned int rapidStepRate);
    void configureMotorBraking(bool enable);
    void suppress(bool suppressGearbox);

    // Feed controls
    void engageZFeedLeft();
    void engageZFeedRight();
    void disengageZFeed();

    int cycle();

    void getSpindleRPM();

    // Hardware Control (not used in most situations but still made publicly accessible)
    QuadEncoder spindleEncoder;
    AccelStepper zStepper;

    // Hardware Info (pulleys, leadscrew pitch, stepper maximums, etc)
    LatheHardwareInfo sysInfo;

    // Gearbox Configuration Variables
    Pitch gearbox_pitch;
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

    float calculateMotorSteps(int encoderTicks); // Given encoder movement (and gearbox settings configured elsewhere) find number of steps to move, INDEPENDENT OF DIRECTION

};

// Necessary hardware specifications for any lathe outfitted with a TeensyLeadscrew
struct LatheHardwareInfo {
    const float encoderPulleyMultiplier; // e.g. if the encoder runs at 2X spindle speed, make this 2
    const unsigned int encoderTicksPerRev;
    const unsigned int stepsPerRev;
    const unsigned int maxStepRate;
    Pitch leadscrewPitch;
};

// PITCH DATA TYPE
// Can describe a feed rate or a thread pitch (tpi, in/rev, or mm/rev)

// tpi or mm
enum PitchUnits {
    tpi,
    mm,
};

enum PitchDirection {
    leftHandThread_feedRight,
    rightHandThread_feedLeft,
};

struct Pitch {
    float value;
    PitchUnits units;
    PitchDirection direction;
};

#endif