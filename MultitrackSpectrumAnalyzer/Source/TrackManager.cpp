#include "TrackManager.h"

const std::array<juce::Colour, 8> TrackManager::trackColours = {
    juce::Colour(0xff4fc3f7),  // Light blue
    juce::Colour(0xffef5350),  // Red
    juce::Colour(0xff66bb6a),  // Green
    juce::Colour(0xffffca28),  // Yellow
    juce::Colour(0xffab47bc),  // Purple
    juce::Colour(0xffff7043),  // Orange
    juce::Colour(0xff26c6da),  // Cyan
    juce::Colour(0xffec407a)   // Pink
};

TrackManager::TrackManager()
{
}

void TrackManager::updateTrackPresence(const juce::String& trackId, 
                                       const juce::String& trackName, 
                                       double sampleRate)
{
    juce::ScopedLock sl(lock);

    auto it = tracks.find(trackId);
    if (it == tracks.end())
    {
        // Create new track (spectrum starts empty)
        TrackData newTrack;
        newTrack.trackId = trackId;
        newTrack.trackName = trackName;
        newTrack.sampleRate = sampleRate;
        newTrack.colour = getNextColour();
        newTrack.lastUpdateTime = juce::Time::currentTimeMillis();
        newTrack.lastSpectrumTime = 0;  // No spectrum data yet
        newTrack.status = TrackStatus::Active;
        newTrack.enabled = true;

        tracks[trackId] = newTrack;
    }
    else
    {
        // Update display name, timestamp, and reset to Active if was offline
        it->second.trackName = trackName;
        it->second.sampleRate = sampleRate;
        it->second.lastUpdateTime = juce::Time::currentTimeMillis();
        if (it->second.status == TrackStatus::Offline)
        {
            it->second.status = TrackStatus::Active;
        }
    }
}

void TrackManager::updateTrack(const juce::String& trackId,
                                const juce::String& trackName,
                                const float* spectrumData,
                                int numBins,
                                double sampleRate)
{
    juce::ScopedLock sl(lock);

    auto it = tracks.find(trackId);
    if (it == tracks.end())
    {
        // Create new track
        TrackData newTrack;
        newTrack.trackId = trackId;
        newTrack.trackName = trackName;
        newTrack.sampleRate = sampleRate;
        newTrack.colour = getNextColour();
        newTrack.lastUpdateTime = juce::Time::currentTimeMillis();
        newTrack.lastSpectrumTime = juce::Time::currentTimeMillis();
        newTrack.status = TrackStatus::Active;
        newTrack.enabled = true;

        // Copy spectrum data
        int copySize = juce::jmin(numBins, SpectrumConstants::NUM_BINS);
        for (int i = 0; i < copySize; ++i)
            newTrack.spectrum[static_cast<size_t>(i)] = spectrumData[i];

        tracks[trackId] = newTrack;
    }
    else
    {
        // Update existing track and reset to Active if was offline
        it->second.trackName = trackName;
        it->second.sampleRate = sampleRate;
        it->second.lastUpdateTime = juce::Time::currentTimeMillis();
        it->second.lastSpectrumTime = juce::Time::currentTimeMillis();
        if (it->second.status == TrackStatus::Offline)
        {
            it->second.status = TrackStatus::Active;
        }

        int copySize = juce::jmin(numBins, SpectrumConstants::NUM_BINS);
        for (int i = 0; i < copySize; ++i)
            it->second.spectrum[static_cast<size_t>(i)] = spectrumData[i];
    }
}

void TrackManager::updateStaleTrack()
{
    juce::ScopedLock sl(lock);

    juce::int64 now = juce::Time::currentTimeMillis();

    for (auto& pair : tracks)
    {
        auto& track = pair.second;
        
        // Check time since last SPECTRUM data (not heartbeat)
        juce::int64 timeSinceSpectrum = (track.lastSpectrumTime > 0) ? (now - track.lastSpectrumTime) : 0;

        if (track.status == TrackStatus::Active)
        {
            // Mark as Offline if no spectrum data received within timeout
            if (track.lastSpectrumTime > 0 && timeSinceSpectrum > SpectrumConstants::TRACK_TIMEOUT_MS)
            {
                track.status = TrackStatus::Offline;
                // Zero out spectrum to show flat line
                track.spectrum.fill(0.0f);
            }
        }
        // Offline tracks stay offline (and zeroed) until spectrum data received
    }
}

std::vector<TrackData> TrackManager::getActiveTracks() const
{
    juce::ScopedLock sl(lock);

    std::vector<TrackData> result;
    result.reserve(tracks.size());

    for (const auto& pair : tracks)
        result.push_back(pair.second);

    return result;
}

std::vector<TrackData> TrackManager::getEnabledTracks() const
{
    juce::ScopedLock sl(lock);

    std::vector<TrackData> result;
    result.reserve(tracks.size());

    for (const auto& pair : tracks)
    {
        if (pair.second.enabled)
            result.push_back(pair.second);
    }

    return result;
}

int TrackManager::getTrackCount() const
{
    juce::ScopedLock sl(lock);
    return static_cast<int>(tracks.size());
}

void TrackManager::setTrackEnabled(const juce::String& trackId, bool enabled)
{
    juce::ScopedLock sl(lock);

    auto it = tracks.find(trackId);
    if (it != tracks.end())
        it->second.enabled = enabled;
}

juce::Colour TrackManager::getNextColour()
{
    juce::Colour colour = trackColours[static_cast<size_t>(colourIndex % trackColours.size())];
    ++colourIndex;
    return colour;
}
