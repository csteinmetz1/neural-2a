#pragma once

#include <shared_plugin_helpers/shared_plugin_helpers.h>

struct Parameters
{
    void add(juce::AudioProcessor& processor) const
    {
        processor.addParameter(inputGain);
        processor.addParameter(outputGain);
        processor.addParameter(limit);
        processor.addParameter(peakReduction);
    }

    //Raw pointers. They will be owned by either the processor or the APVTS (if you use it)
    juce::AudioParameterFloat* inputGain =
        new juce::AudioParameterFloat("inputGain", "Input Gain", 0.f, 1.f, 0.5f);

    juce::AudioParameterFloat* outputGain =
        new juce::AudioParameterFloat("outputGain", "Output Gain", 0.f, 1.f, 0.5f);

    juce::AudioParameterFloat* limit =
        new juce::AudioParameterFloat("limit", "Limit", 0.f, 1.f, 1.0f);

    juce::AudioParameterFloat* peakReduction = new juce::AudioParameterFloat(
        "peakReduction", "Peak Reduction", 0.f, 100.f, 50.0f);
};