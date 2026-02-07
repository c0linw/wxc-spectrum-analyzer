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

        // Add to insertion order list (always at the end)
        customTrackOrder.push_back(trackId);
    }
    else
    {
        // Update display name and timestamp (don't reset offline status on heartbeat)
        it->second.trackName = trackName;
        it->second.sampleRate = sampleRate;
        it->second.lastUpdateTime = juce::Time::currentTimeMillis();
        // Status will be reset to Active only when spectrum data arrives
    }
}

void TrackManager::updateTrack(const juce::String& trackId,
                                const juce::String& trackName,
                                const float* spectrumData,
                                int numBins,
                                double sampleRate)
{
    juce::ScopedLock sl(lock);

    // Smoothing factor for exponential moving average (0-1)
    // Lower = more smoothing, Higher = more responsive
    constexpr float smoothingFactor = 0.25f;

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

        // Copy spectrum data and initialize smoothed spectrum
        int copySize = juce::jmin(numBins, SpectrumConstants::NUM_BINS);
        for (int i = 0; i < copySize; ++i)
        {
            newTrack.spectrum[static_cast<size_t>(i)] = spectrumData[i];
            newTrack.smoothedSpectrum[static_cast<size_t>(i)] = spectrumData[i]; // Initialize with first value
        }

        tracks[trackId] = newTrack;

        // Add to insertion order list (always at the end)
        customTrackOrder.push_back(trackId);
    }
    else
    {
        // Update existing track and reset to Active if was offline
        it->second.trackName = trackName;
        it->second.sampleRate = sampleRate;
        it->second.lastUpdateTime = juce::Time::currentTimeMillis();
        it->second.lastSpectrumTime = juce::Time::currentTimeMillis();

        bool wasOffline = (it->second.status == TrackStatus::Offline);
        if (wasOffline)
        {
            it->second.status = TrackStatus::Active;
        }

        int copySize = juce::jmin(numBins, SpectrumConstants::NUM_BINS);
        for (int i = 0; i < copySize; ++i)
        {
            it->second.spectrum[static_cast<size_t>(i)] = spectrumData[i];

            // Apply exponential moving average smoothing
            // If just came back online, reset smoothed value to avoid jump from zero
            if (wasOffline)
                it->second.smoothedSpectrum[static_cast<size_t>(i)] = spectrumData[i];
            else
                it->second.smoothedSpectrum[static_cast<size_t>(i)] =
                    smoothingFactor * spectrumData[i] +
                    (1.0f - smoothingFactor) * it->second.smoothedSpectrum[static_cast<size_t>(i)];
        }
    }
}

void TrackManager::updateStaleTrack()
{
    juce::ScopedLock sl(lock);

    // Use same decay factor as active tracks for consistent visual appearance
    // This is 1.0 - smoothingFactor (0.25) from active track smoothing
    constexpr float decayFactor = 0.75f;

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
                // Zero out raw spectrum but let smoothed spectrum decay naturally
                track.spectrum.fill(0.0f);
            }
        }
        else if (track.status == TrackStatus::Offline)
        {
            // Apply exponential decay to smoothed spectrum while offline
            for (size_t i = 0; i < track.smoothedSpectrum.size(); ++i)
            {
                track.smoothedSpectrum[i] *= decayFactor;
            }
        }
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

std::vector<TrackData> TrackManager::getActiveTracksOrdered() const
{
    juce::ScopedLock sl(lock);

    std::vector<TrackData> result;
    result.reserve(tracks.size());

    // Always use customTrackOrder (which tracks insertion/manual order)
    for (const auto& trackId : customTrackOrder)
    {
        auto it = tracks.find(trackId);
        if (it != tracks.end())
            result.push_back(it->second);
    }

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

void TrackManager::setTrackColour(const juce::String& trackId, const juce::Colour& colour)
{
    juce::ScopedLock sl(lock);

    auto it = tracks.find(trackId);
    if (it != tracks.end())
        it->second.colour = colour;
}

void TrackManager::reorderTrack(const juce::String& trackId, int newIndex)
{
    juce::ScopedLock sl(lock);

    // Remove from current position
    auto it = std::find(customTrackOrder.begin(), customTrackOrder.end(), trackId);
    if (it != customTrackOrder.end())
        customTrackOrder.erase(it);

    // Insert at new position
    newIndex = juce::jlimit(0, static_cast<int>(customTrackOrder.size()), newIndex);
    customTrackOrder.insert(customTrackOrder.begin() + newIndex, trackId);
}

juce::Colour TrackManager::getNextColour()
{
    juce::Colour colour = trackColours[static_cast<size_t>(colourIndex % trackColours.size())];
    ++colourIndex;
    return colour;
}
