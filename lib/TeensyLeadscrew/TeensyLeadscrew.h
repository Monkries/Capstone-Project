#ifndef TeensyLeadscrew_h
#define TeensyLeadscrew_h

#include "Arduino.h"
#include "AccelStepper.h"
#include "QuadEncoder.h"
#include "EncoderTach.h"
#include "IntegerStepHelper.h"
#include "SynchronousBidirectionalClutch.h"

// PITCH DATA TYPE
// Can describe a feed rate or a thread pitch (tpi, in/rev, or mm/rev)

// Units for the Pitch struct (tpi, inches/revolution, or mm)
enum PitchUnits {
    tpi,
    mm,
    in_per_rev,
};

// Direction for the Pitch struct (leftHand)
enum PitchDirection {
    leftHandThread_feedRight, // In the case of threading, left hand thread. In the case of feeding, feeding right (towards the tailstock)
    rightHandThread_feedLeft, // In the case of threading, right hand thread. In the case of feeding, feeding left (toward the headstock)
};

// @brief Describes a thread pitch, or a feed rate (e.g. 0.003"/REV feeding left, or 1.5mm right hand thread)
struct Pitch {
    // @brief The number portion of the pitch (e.g. for a 1.5mm pitch, this would be 1.5)
    float value;
    // @brief Units for the Pitch (tpi, mm, or in_per_rev)
    PitchUnits units;
    // @brief Direction for the pitch (leftHandThread_feedRight or rightHandThread_feedLeft)
    PitchDirection direction;
};

// @brief Describes the current "gearbox" configuration (equivalent to the threading gearbox settings on a traditional lathe)
struct ELSGearboxConfig {
    Pitch configuredPitch;
    bool rapidReturn; // rapid in the "return" direction, based on the direction of the thread/feed set in configuredPitch. (e.g. if you're cutting a 10TPI-RH thread, setting this to true will rapid toward the tailstock)
    unsigned int rapidStepRate; // steps/sec that we use when rapid-ing
    bool enableMotorBraking; // Set to false to allow the motor to freewheel when not moving
    bool disable; // Disable gearbox logic entirely (for future expansion to non-standard movements)
};

// All necessary hardware specifications for any lathe outfitted with a TeensyLeadscrew
struct LatheHardwareInfo {
    float encoderPulleyMultiplier; // e.g. if the encoder runs at 2X spindle speed, make this 2
    unsigned int encoderTicksPerRev; // Ticks per revolution of the encoder (for a 2000PPR encoder, this is 8000)
    unsigned int stepsPerRev; // Steps per revolution on the stepper (including microstepping)
    unsigned int warningStepRate; // A "soft maximum" speed for the stepper (used for overspeed warning)
    Pitch leadscrewPitch; // The pitch of the Z leadscrew on the machine
};

/*
@brief Main, top-level electronic leadscrew class. When you want to control the system, you use this. 

Call TeensyLeadscrew::init() once in setup, then call TeensyLeadscrew::cycle() as often as possible. 
Configure pitch/feed rate in TeensyLeadscrew::gearbox, and operate the virtual feed clutch with TeensyLeadscrew::clutch.
 */
class TeensyLeadscrew {
    public:
    /*
    @brief Create an instance of TeensyLeadscrew, giving it access to the encoder and stepper, and all the hardware specs to do the threading math correctly.
    @param spindleEncoder Spindle encoder, using Teensy's hardware quadrature decoder (should already be initialized)
    @param zStepper Leadscrew stepper motor object (should already be initialized)
    @param hardwareSpecs Fixed hardware parameters of whatever lathe this is running on
    @param tachometerSamplePeriod_micros How long (in microseconds) to wait between update spindle RPMs.
    */
    TeensyLeadscrew(QuadEncoder &spindleEncoder,
        AccelStepper &zStepper,
        LatheHardwareInfo hardwareSpecs,
        unsigned int tachometerSamplePeriod_micros);

    // @brief Initialize spindleTach, and any other misc stuff that must wait until other hardware is fired up
    void init();

    // @brief Main leadscrew business logic. Reads the encoder, updates the tachometer, calculates any leadscrew movement, then actually moves the motor. Also updates motor braking pin.
    void cycle();

    // Spindle Tachometer Helper (use spindleTach.getRPM() to get the speed. No need to do anything else, we "feed" this object as part of the cycle() function)
    EncoderTach spindleTach;

    // HARDWARE CONTROL
    // @brief Spindle encoder object. For normal leadscrew operation, you probably don't need to access this directly, but it's here just in case.
    QuadEncoder &spindleEncoder;
    // @brief Leadscrew stepper motor. For normal leadscrew operation, you probably don't need to access this directly, but it's here just in case.
    AccelStepper &zStepper;

    // @brief Relevant lathe hardware specs. This should be set once in the constructor, DO NOT adjust after that.
    LatheHardwareInfo sysInfo;

    // @brief Gearbox configuration (most of the settings you'll want to adjust are in here).
    ELSGearboxConfig gearbox;

    // @brief Feed clutch, patterned after the HLV-H. Call .engageForward(), .engageReverse(), or .disengage() to control it.
    SynchronousBidirectionalClutch clutch;

    // @brief If true, it means the leadscrew speed has hit the governed limit (so synchronization is lost)
    bool leadscrewOverspeedAlarmActive = false;

    private:
    /*
    @brief Given encoder movement, and the current settings in TeensyLeadscrew::gearbox, find number of steps to move, IGNORING ANY CLUTCH ACTION/DIRECTION
    @param encoderTicks Encoder movement
    @return Steps to move based on the input encoder movement (this is not necessarily a neat integer!)
    */
    float calculateMotorSteps(int encoderTicks);

    // @brief This is a helper class to make sure we keep track of fractional steps across cycles
    IntegerStepHelper queuedMotorSteps;
};

#endif