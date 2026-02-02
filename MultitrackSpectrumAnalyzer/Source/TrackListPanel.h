#pragma once

#include <JuceHeader.h>
#include "TrackManager.h"

class TrackListPanel : public juce::Component,
                       private juce::Timer
{
public:
    TrackListPanel(TrackManager& trackManager);
    ~TrackListPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    void rebuildTrackList();

    TrackManager& trackManager;

    juce::Label titleLabel;
    juce::OwnedArray<juce::ToggleButton> trackButtons;
    std::vector<juce::String> currentTrackNames;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackListPanel)
};
