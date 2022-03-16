#ifndef EncoderTach_h
#define EncoderTach_h

#include "Arduino.h"

class EncoderTach {
    public:
    EncoderTach(unsigned int samplePeriod_micros, unsigned int encoderTicksPerRev, float pulleyMultiplier);
    EncoderTach();
    void recordTicks(int encoderTicks);
    float getRPM();

    private:
    unsigned int _encoderTicksPerRev;
    unsigned int _samplePeriod_micros;
    float _pulleyMultiplier;
    elapsedMicros _timeSinceSample;
    int _ticksSinceLastSample;
    float _lastSampledRPM;
};

#endif