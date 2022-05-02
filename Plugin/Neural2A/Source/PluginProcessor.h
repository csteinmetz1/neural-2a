#pragma once

#include "Parameters.h"
#include <torch/script.h>
#include <torch/torch.h>

class NewPluginTemplateAudioProcessor : public PluginHelpers::ProcessorBase
{
public:
    NewPluginTemplateAudioProcessor();

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    juce::AudioProcessorEditor* createEditor() override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    void buildModel();
    void setupBuffers();

    int seed = 42;
    int receptiveFieldSamples = 13333; // in samples
    int blockSamples = 0; // in/out samples
    double sampleRate = 0; // in Hz

    // holder for the linear gain values
    // (don't want to convert dB -> linear on audio thread)
    float inputGainLn, outputGainLn;

private:
    Parameters parameters;

    juce::AudioBuffer<float> membuf, procbuf; // circular buffer to store input data
    int mbr, mbw; // read and write pointers

    int nInputs;
    int membuflength; // number of samples in the memory (past samples) buffer (rf - 1)
    int procbuflength; // number of samples in the process buffer (rf + block - 1)

    // high pass filters for the left and right channels
    std::vector<juce::IIRFilter> highPassFilters;

    torch::jit::script::Module model;
};
