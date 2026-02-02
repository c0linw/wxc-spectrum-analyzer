#pragma once

#include <JuceHeader.h>
#include "TrackManager.h"

class SpectrumDisplay : public juce::Component,
                        private juce::Timer
{
public:
    SpectrumDisplay(TrackManager& trackManager);
    ~SpectrumDisplay() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;

    /// Convert frequency bin index to x-coordinate (logarithmic scale).
    float binToX(int bin, float width, double sampleRate) const;

    /// Convert frequency to x-coordinate (logarithmic scale).
    float frequencyToX(float frequency, float width) const;

    /// Convert normalized magnitude (0-1) to y-coordinate (dB scale).
    float magnitudeToY(float magnitude, float height) const;

    /// Draw single track's spectrum curve.
    void drawSpectrum(juce::Graphics& g, const TrackData& track, juce::Rectangle<float> area);

    /// Draw frequency axis labels and grid lines.
    void drawFrequencyAxis(juce::Graphics& g, juce::Rectangle<float> area);

    /// Draw amplitude axis labels and grid lines.
    void drawAmplitudeAxis(juce::Graphics& g, juce::Rectangle<float> area);

    TrackManager& trackManager;
    std::vector<TrackData> cachedTracks;

    // Display range
    static constexpr float minFrequency = 20.0f;
    static constexpr float maxFrequency = 20000.0f;
    static constexpr float minDb = -90.0f;
    static constexpr float maxDb = 0.0f;

    // Layout margins
    static constexpr int leftMargin = 45;
    static constexpr int bottomMargin = 25;
    static constexpr int topMargin = 10;
    static constexpr int rightMargin = 10;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumDisplay)
};
