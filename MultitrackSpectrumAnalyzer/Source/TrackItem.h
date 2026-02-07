#pragma once
#include <JuceHeader.h>

class TrackItem : public juce::Component
{
public:
    TrackItem(const juce::String& trackId,
              const juce::String& trackName,
              const juce::Colour& colour);

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;

    // Callbacks
    std::function<void(bool enabled)> onToggleChanged;
    std::function<void(juce::Colour colour)> onColourChanged;

    // Setters
    void setToggleState(bool enabled);
    void setTrackColour(const juce::Colour& colour);
    void setTrackName(const juce::String& name);
    void setOfflineStatus(bool isOffline);

    juce::String getTrackId() const { return trackId; }

private:
    void showColourSelector();

    juce::String trackId;
    juce::String trackName;
    juce::Colour trackColour;
    bool isEnabled { true };
    bool isOffline { false };

    juce::ToggleButton toggleButton;
    juce::Rectangle<int> colourSwatchBounds;

    static constexpr int colourSwatchSize = 20;
    static constexpr int spacing = 4;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackItem)
};
