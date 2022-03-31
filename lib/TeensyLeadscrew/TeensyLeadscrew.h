#ifndef TeensyLeadscrew_h
#define TeensyLeadscrew_h

#include "Arduino.h"
#include "AccelStepper.h"
#include "QuadEncoder.h"
#include "EncoderTach.h"
#include "IntegerStepHelper.h"

// PITCH DATA TYPE
// Can describe a feed rate or a thread pitch (tpi, in/rev, or mm/rev)

// tpi or mm
enum PitchUnits {
    tpi,
    mm,
    in_per_rev,
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

struct DogClutch {
    float inputShaftAngle;
    bool engaged;
    bool locked;
};

// Necessary hardware specifications for any lathe outfitted with a TeensyLeadscrew
struct LatheHardwareInfo {
    float encoderPulleyMultiplier; // e.g. if the encoder runs at 2X spindle speed, make this 2
    unsigned int encoderTicksPerRev;
    unsigned int stepsPerRev;
    unsigned int maxStepRate;
    Pitch leadscrewPitch;
};

class TeensyLeadscrew {
    public:
    TeensyLeadscrew(QuadEncoder &spindleEncoder,
        AccelStepper &zStepper,
        LatheHardwareInfo hardwareSpecs,
        unsigned int tachometerSamplePeriod_micros);

    void init();

    // Feed controls (modeled after HLV-H leadscrew clutch lever)
    void engageZFeedLeft();
    void engageZFeedRight();
    void disengageZFeed();

    void cycle();

    // Spindle Tachometer
    EncoderTach spindleTach;

    // Hardware Control (not used in most situations but still made publicly accessible)
    QuadEncoder &spindleEncoder;
    AccelStepper &zStepper;

    // Hardware Info (pulleys, leadscrew pitch, stepper maximums, etc)
    LatheHardwareInfo sysInfo;

    // Gearbox Configuration Variables (with defaults for idiot proofing)
    Pitch gearbox_pitch;
    bool gearbox_rapidLeft = false;
    bool gearbox_rapidRight = false;
    unsigned int gearbox_rapidStepRate = 0;
    bool gearbox_enableMotorBraking = true;
    bool gearbox_suppress = false;

    float calculateMotorSteps(int encoderTicks); // Given encoder movement (and gearbox settings configured elsewhere) find number of steps to move, INDEPENDENT OF DIRECTION

    float stepsMoved; // temporary for debugging
    float encoderTicksRecorded;

    // Clutch state info
    DogClutch clutchState;
    DogClutch lastClutchState;

    //private:
    int zFeedDirection; // 0=neutral, 1=left, -1=right
    int zFeedDirection_previousCycle; // Still 0=neutral, 1=left, -1=right, but value from 1 cycle before

    IntegerStepHelper queuedMotorSteps; // This is a helper class to make sure we keep track of fractional steps across cycles
};

#endif