#pragma once

#include <JuceHeader.h>
#include "TrackManager.h"
#include "TrackListPanel.h"
#include "SpectrumDisplay.h"

class MainComponent : public juce::Component,
                      private juce::OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback>,
                      private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void oscMessageReceived(const juce::OSCMessage& message) override;
    void timerCallback() override;
    void updateStatusLabel();
    void setupDisplayControls();
    void onDisplayModeChanged();
    void onDbScalingChanged();

    juce::Label titleLabel;
    juce::Label statusLabel;

    juce::Label displayModeLabel;
    juce::ComboBox displayModeCombo;
    juce::Label dbScalingLabel;
    juce::ComboBox dbScalingCombo;

    juce::OSCReceiver oscReceiver;
    TrackManager trackManager;
    TrackListPanel trackListPanel;
    SpectrumDisplay spectrumDisplay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
