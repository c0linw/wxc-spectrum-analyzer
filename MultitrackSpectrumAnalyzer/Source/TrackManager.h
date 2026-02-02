#pragma once

#include <JuceHeader.h>
#include "../../Common/SpectrumData.h"

struct TrackData
{
    juce::String trackId;      // Unique identifier (UUID from plugin)
    juce::String trackName;    // Display name
    double sampleRate { 0.0 };
    juce::Colour colour;
    juce::int64 lastUpdateTime { 0 };
    std::array<float, SpectrumConstants::NUM_BINS> spectrum { 0.0f };
    bool enabled { true };
};

class TrackManager
{
public:
    TrackManager();

    /// Update track presence (called on heartbeat)
    void updateTrackPresence(const juce::String& trackId, 
                            const juce::String& trackName, 
                            double sampleRate);

    /// Update track with new spectrum data
    void updateTrack(const juce::String& trackId,
                    const juce::String& trackName,
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

    /// Enable or disable a track
    void setTrackEnabled(const juce::String& trackId, bool enabled);

private:
    juce::Colour getNextColour();

    std::map<juce::String, TrackData> tracks;  // Key is trackId (UUID)
    mutable juce::CriticalSection lock;
    int colourIndex { 0 };

    // Predefined colour palette for tracks
    static const std::array<juce::Colour, 8> trackColours;
};
