#include "PluginProcessor.h"
#include "PluginEditor.h"

constexpr bool shouldUseGenericEditor = true;

NewPluginTemplateAudioProcessor::NewPluginTemplateAudioProcessor()
{
    parameters.add(*this);

    // neural network model
    buildModel();
}

const juce::String NewPluginTemplateAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NewPluginTemplateAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool NewPluginTemplateAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool NewPluginTemplateAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double NewPluginTemplateAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NewPluginTemplateAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
        // so this should be at least 1, even if you're not really implementing programs.
}

int NewPluginTemplateAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NewPluginTemplateAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String NewPluginTemplateAudioProcessor::getProgramName(int index)
{
    return {};
}

void NewPluginTemplateAudioProcessor::changeProgramName(int index,
                                                        const juce::String& newName)
{
}

//==============================================================================

void NewPluginTemplateAudioProcessor::setupBuffers()
{
    // compute the size of the buffer which will be passed to model
    membuflength = (int) (13333 - 1);
    procbuflength = (int) (13333 - 1 + blockSamples);

    std::cout << "membuflength " << membuflength << std::endl;
    std::cout << "procbuflength " << procbuflength << std::endl;

    // Initialize the to n channels
    nInputs = getTotalNumInputChannels();

    // and membuflength samples per channel
    membuf.setSize(1, membuflength);
    membuf.clear();

    procbuf.setSize(1, procbuflength);
    procbuf.clear();
}

void NewPluginTemplateAudioProcessor::prepareToPlay(double sampleRate_,
                                                    int samplesPerBlock_)
{
    // store the sample rate for future calculations
    sampleRate = sampleRate_;
    blockSamples = samplesPerBlock_;

    // setup high pass filter model
    double freq = 10.0;
    double q = 10.0;
    for (int channel = 0; channel < getTotalNumOutputChannels(); ++channel)
    {
        juce::IIRFilter filter;
        filter.setCoefficients(juce::IIRCoefficients::makeHighPass(sampleRate_, freq, q));
        highPassFilters.push_back(filter);
    }

    setupBuffers(); // setup the buffer for handling context
}

void NewPluginTemplateAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                                   juce::MidiBuffer& midiMessages)

{
    juce::ScopedNoDenormals noDenormals;
    auto inChannels = getTotalNumInputChannels();
    auto outChannels = getTotalNumOutputChannels();

    // we have to handle some buffer business first (this is somewhat inefficient)
    // 1. first we construct the process buffer which is [membuf, buffer]

    // first copy the past samples into the process buffer
    procbuf.copyFrom(0, 0, membuf, 0, 0, membuflength);

    // second copy the current buffer samples at the end
    procbuf.copyFrom(0, membuflength, buffer, 0, 0, blockSamples);

    // now we update membuf to reflect the last N samples in proccess buffer
    membuf.copyFrom(0, 0, procbuf, 0, procbuflength - membuflength, membuflength);

    // 3. now move the process buffer to a tensor  (size of the process buffer data)
    std::vector<int64_t> sizes = {procbuflength * 1}; // only supports mono
    auto* procbufptr = procbuf.getWritePointer(0); // get pointer of the first channel

    // load data from buffer into tensor type
    at::Tensor frame = torch::from_blob(procbufptr, sizes);

    frame = torch::mul(frame, parameters.inputGain->get()); // apply the input gain first

    // reshape so we have a batch and channel dimension
    frame = torch::reshape(frame, {1, 1, procbuflength});

    at::Tensor control_parameters = torch::empty({2});
    control_parameters.index_put_({0}, parameters.limit->get());
    control_parameters.index_put_({1}, parameters.peakReduction->get() / 100.0);

    // reshape so we have a batch and channel dimension
    control_parameters = torch::reshape(control_parameters, {1, 1, 2});

    // create special holder for model inputs
    std::vector<torch::jit::IValue> input_values;
    input_values.push_back(frame); // add the process buffer
    input_values.push_back(control_parameters); // add the parameter values (conditioning)

    at::Tensor output = model.forward(input_values).toTensor();

    // now load the output channels back into the buffer
    for (int channel = 0; channel < outChannels; ++channel)
    {
        // index the proper output channel
        auto outputData = output.index({0, 0, torch::indexing::Slice()});
        auto outputDataPtr = outputData.data_ptr<float>();

        // copy output data to buffer
        buffer.copyFrom(channel, 0, outputDataPtr, blockSamples);

        // remove the DC bias
        highPassFilters[channel].processSamples(buffer.getWritePointer(channel),
                                                buffer.getNumSamples());
    }
    buffer.applyGain(parameters.outputGain->get()); // apply the output gain
}

juce::AudioProcessorEditor* NewPluginTemplateAudioProcessor::createEditor()
{
    if (shouldUseGenericEditor)
        return new juce::GenericAudioProcessorEditor(*this);
    else
        return new NewPluginTemplateAudioProcessorEditor(*this);
}

void NewPluginTemplateAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    //Serializes your parameters, and any other potential data into an XML:

    juce::ValueTree params("Params");

    for (auto& param: getParameters())
    {
        juce::ValueTree paramTree(PluginHelpers::getParamID(param));
        paramTree.setProperty("Value", param->getValue(), nullptr);
        params.appendChild(paramTree, nullptr);
    }

    juce::ValueTree pluginPreset("MyPlugin");
    pluginPreset.appendChild(params, nullptr);
    //This a good place to add any non-parameters to your preset

    copyXmlToBinary(*pluginPreset.createXml(), destData);
}

void NewPluginTemplateAudioProcessor::setStateInformation(const void* data,
                                                          int sizeInBytes)
{
    //Loads your parameters, and any other potential data from an XML:

    auto xml = getXmlFromBinary(data, sizeInBytes);

    if (xml != nullptr)
    {
        auto preset = juce::ValueTree::fromXml(*xml);
        auto params = preset.getChildWithName("Params");

        for (auto& param: getParameters())
        {
            auto paramTree = params.getChildWithName(PluginHelpers::getParamID(param));

            if (paramTree.isValid())
                param->setValueNotifyingHost(paramTree["Value"]);
        }

        //Load your non-parameter data now
    }
}

void NewPluginTemplateAudioProcessor::buildModel()
{
    try
    {
        // Deserialize the ScriptModule from a file using torch::jit::load().
        model = torch::jit::load(
            "/Users/cjstein/Code/micro-tcn/models/traced_1-uTCN-300__causal__4-10-13__fraction-0.01-bs32.pt");
    }
    catch (const c10::Error& e)
    {
        std::cerr << "error loading the model\n";
        return -1;
    }

    std::cout << "ok\n";
    std::cout << "receptive field: " << receptiveFieldSamples << std::endl;
    setupBuffers();
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewPluginTemplateAudioProcessor();
}
