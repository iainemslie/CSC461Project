/*
  ==============================================================================

    This file handles the SineWave Voice and Sound structs.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
struct SineWaveSound : public SynthesiserSound
{
    SineWaveSound() {}

    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

//==============================================================================
struct SineWaveVoice : public SynthesiserVoice
{
    SineWaveVoice() {}

    bool canPlaySound(SynthesiserSound* sound) override
    {
        return dynamic_cast<SineWaveSound*> (sound) != nullptr;
    }

    void startNote(int midiNoteNumber, float velocity,
        SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {

        currentAngle = 0.0;
        level = velocity * 0.15;
        tailOff = 0.0;

        auto cyclesPerSecond = MidiMessage::getMidiNoteInHertz(midiNoteNumber);
        auto cyclesPerSample = cyclesPerSecond / getSampleRate();

        angleDelta = cyclesPerSample * 2.0 * MathConstants<double>::pi;
    }

    void stopNote(float /*velocity*/, bool allowTailOff) override
    {
        if (allowTailOff) {
            if (tailOff == 0.0) {
                tailOff = 1.0;
            }
        } else {
            clearCurrentNote();
            angleDelta = 0.0;
        }
    }

    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}

    void renderNextBlock(AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override
    {
        if (angleDelta != 0.0) {
            if (tailOff > 0.0) {
                while (--numSamples >= 0) {
                    auto currentSample = (float)(std::sin(currentAngle) * level * tailOff);

                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;) {
                        outputBuffer.addSample(i, startSample, currentSample);
                    }

                    currentAngle += angleDelta;
                    ++startSample;

                    tailOff *= 0.99;

                    if (tailOff <= 0.005) {
                        clearCurrentNote();

                        angleDelta = 0.0;
                        break;
                    }
                }
            } else {
                while (--numSamples >= 0) {
                    auto currentSample = (float)(std::sin(currentAngle) * level);

                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;) {
                        outputBuffer.addSample(i, startSample, currentSample);
                    }

                    currentAngle += angleDelta;
                    ++startSample;
                }
            }
        }
    }

private:
    double currentAngle = 0.0;
    double angleDelta = 0.0;
    double level = 0.0;
    double tailOff = 0.0;
};
