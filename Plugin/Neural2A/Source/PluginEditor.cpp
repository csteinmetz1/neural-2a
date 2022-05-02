#include "PluginProcessor.h"
#include "PluginEditor.h"

NewPluginTemplateAudioProcessorEditor::NewPluginTemplateAudioProcessorEditor(
    NewPluginTemplateAudioProcessor& p)
    : AudioProcessorEditor(&p)
{
    setSize(400, 450);
}

void NewPluginTemplateAudioProcessorEditor::paint(juce::Graphics& g)
{
    //g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.fillAll(juce::Colours::white);

    g.setColour(juce::Colours::grey);
    g.setFont(juce::Font("Source Sans Variable", 32.0f, juce::Font::plain)
                  .withTypefaceStyle("Light"));
    g.drawText("Neural-2A", 200, 0, 300, 70, juce::Justification::centred, true);

    g.setFont(juce::Font("Source Sans Variable", 18.0f, juce::Font::plain)
                  .withTypefaceStyle("Light"));
    g.drawText(
        "Christian J. Steinmetz", 200, 50, 300, 70, juce::Justification::centred, true);
}
