#ifndef TeensyLeadscrew_h
#define TeensyLeadscrew_h

#include "Arduino.h"
#include "AccelStepper.h"
#include "QuadEncoder.h"
#include "EncoderTach.h"
#include "IntegerStepHelper.h"
#include "SynchronousClutch.h"

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

// For describing a thread pitch or a feed rate (e.g. "28TPI-RH" or "0.001in/REV, left")
struct Pitch {
    float value;
    PitchUnits units;
    PitchDirection direction;
};

// For keeping track of "virtual single tooth clutch" state easily
struct DogClutch {
    float inputShaftAngle;
    float outputShaftAngle;
    bool engaged;
    bool locked;
};

struct ELSGearboxConfig {
    Pitch configuredPitch;
    bool rapidReturn; // rapid in the "return" direction, based on the direction of the thread/feed set in configuredPitch. (e.g. if you're cutting a 10TPI-RH thread, setting this to true will rapid toward the tailstock)
    unsigned int rapidStepRate;
    bool enableMotorBraking; // Set to false to allow the motor to freewheel when not moving
    bool disable; // Disable gearbox logic entirely (for future expansion to non-standard movements)
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

    void init(); // Initialize spindleTach and other stuff that must wait until the hardware is ready

    void cycle();

    // Spindle Tachometer
    EncoderTach spindleTach;

    // Hardware Control (not used in most situations but still made publicly accessible)
    QuadEncoder &spindleEncoder;
    AccelStepper &zStepper;

    // Hardware Info (pulleys, leadscrew pitch, stepper maximums, etc)
    LatheHardwareInfo sysInfo;

    // Gearbox Configuration (default state is set in constructor)
    ELSGearboxConfig gearbox;

    float calculateMotorSteps(int encoderTicks); // Given encoder movement (and gearbox settings configured elsewhere) find number of steps to move, INDEPENDENT OF DIRECTION

    // Clutch state info
    SynchronousClutch clutch;

    IntegerStepHelper queuedMotorSteps; // This is a helper class to make sure we keep track of fractional steps across cycles
};

#endif