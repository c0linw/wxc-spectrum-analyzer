#pragma once

#include <JuceHeader.h>
#include "TrackManager.h"
#include "TrackListPanel.h"

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

    juce::Label titleLabel;
    juce::Label statusLabel;

    juce::OSCReceiver oscReceiver;
    TrackManager trackManager;
    TrackListPanel trackListPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
