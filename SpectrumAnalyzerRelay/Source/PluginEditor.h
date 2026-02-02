#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class SpectrumAnalyzerRelayAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    SpectrumAnalyzerRelayAudioProcessorEditor(SpectrumAnalyzerRelayAudioProcessor&);
    ~SpectrumAnalyzerRelayAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    SpectrumAnalyzerRelayAudioProcessor& audioProcessor;

    juce::Label titleLabel;
    juce::Label trackNameLabel;
    juce::TextEditor trackNameEditor;
    juce::Label oscPortLabel;
    juce::TextEditor oscPortEditor;
    juce::ToggleButton enableButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzerRelayAudioProcessorEditor)
};
