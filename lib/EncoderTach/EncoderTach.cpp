#include "Arduino.h"
#include "EncoderTach.h"

EncoderTach::EncoderTach(unsigned int samplePeriod_micros, unsigned int encoderTicksPerRev, float pulleyMultiplier)
{
    // Store configuration info
    _samplePeriod_micros = samplePeriod_micros;
    _encoderTicksPerRev = encoderTicksPerRev;
    _pulleyMultiplier = pulleyMultiplier;
    _timeSinceSample = 0;
    _lastSampledRPM = 0;
    _ticksSinceLastSample = 0;
}

// Default constructor, which does nothing
EncoderTach::EncoderTach(){};

void EncoderTach::recordTicks(int encoderTicks)
{
    // Store the movement (on top of any accumulated movement since the last RPM computation)
    _ticksSinceLastSample = _ticksSinceLastSample + encoderTicks;

    // If enough time has elapsed since the last sample, record the data and compute the RPM
    // Else if it's too soon after the last computation, store the encoder movement, but don't update the RPM
    if (_timeSinceSample > _samplePeriod_micros)
    {
            // Grab time stamp, reset the stopwatch
        int timestamp_micros = _timeSinceSample;
        _timeSinceSample = 0;

        float encoderRotations = abs( ((float)_ticksSinceLastSample) / ((float)_encoderTicksPerRev) );
        float minutes = ((float)timestamp_micros)/(1e6*60.0);

        // Encoder movement in rotations / time elapsed in minutes
        _lastSampledRPM = (encoderRotations / minutes) * _pulleyMultiplier;

        // Reset the encoder movement we've accumulated
        _ticksSinceLastSample = 0;
    }
}

float EncoderTach::getRPM()
{
    return _lastSampledRPM;
}