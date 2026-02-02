#pragma once

#include <JuceHeader.h>
#include "../../Common/SpectrumData.h"

struct TrackData
{
    juce::String name;
    std::array<float, SpectrumConstants::NUM_BINS> spectrum {};
    double sampleRate { 44100.0 };
    juce::int64 lastUpdateTime { 0 };
    juce::Colour colour;
    bool enabled { true };  // Whether to display this track's spectrum
};

class TrackManager
{
public:
    TrackManager();

    /// Called when heartbeat received. Creates track if new, updates timestamp.
    void updateTrackPresence(const juce::String& trackName, double sampleRate);

    /// Called when spectrum OSC message received. Updates spectrum data.
    void updateTrack(const juce::String& trackName,
                     const float* spectrumData,
                     int numBins,
                     double sampleRate);

    /// Removes tracks that haven't been updated within timeout period.
    void removeStaleTrack();

    /// Returns list of currently active tracks (thread-safe copy).
    std::vector<TrackData> getActiveTracks() const;

    /// Returns list of enabled tracks only (for spectrum display).
    std::vector<TrackData> getEnabledTracks() const;

    int getTrackCount() const;

    /// Enable or disable a track's spectrum display.
    void setTrackEnabled(const juce::String& trackName, bool enabled);

private:
    juce::Colour getNextColour();

    std::map<juce::String, TrackData> tracks;
    mutable juce::CriticalSection lock;
    int colourIndex { 0 };

    // Predefined colour palette for tracks
    static const std::array<juce::Colour, 8> trackColours;
};
