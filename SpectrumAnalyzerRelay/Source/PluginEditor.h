#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class SpectrumAnalyzerRelayAudioProcessorEditor : public juce::AudioProcessorEditor,
                                                   private juce::Timer
{
public:
    SpectrumAnalyzerRelayAudioProcessorEditor(SpectrumAnalyzerRelayAudioProcessor&);
    ~SpectrumAnalyzerRelayAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    SpectrumAnalyzerRelayAudioProcessor& audioProcessor;

    juce::Label titleLabel;
    
    juce::Label dawTrackNameLabel;
    juce::Label dawTrackNameValue;     // Shows actual DAW track name (read-only)
    
    juce::ToggleButton useCustomNameCheckbox;
    juce::Label customTrackNameLabel;
    juce::TextEditor customTrackNameEditor;
    
    juce::Label oscPortLabel;
    juce::TextEditor oscPortEditor;
    juce::ToggleButton enableButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzerRelayAudioProcessorEditor)
};
