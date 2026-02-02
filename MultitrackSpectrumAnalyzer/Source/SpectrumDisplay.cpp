#include "SpectrumDisplay.h"
#include "../../Common/SpectrumData.h"
#include <cmath>

SpectrumDisplay::SpectrumDisplay(TrackManager& tm)
    : trackManager(tm)
{
    startTimerHz(60);
}

SpectrumDisplay::~SpectrumDisplay()
{
    stopTimer();
}

void SpectrumDisplay::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));

    auto bounds = getLocalBounds().toFloat();
    auto plotArea = bounds.withTrimmedLeft(static_cast<float>(leftMargin))
                          .withTrimmedBottom(static_cast<float>(bottomMargin))
                          .withTrimmedTop(static_cast<float>(topMargin))
                          .withTrimmedRight(static_cast<float>(rightMargin));

    // Draw grid and axes
    drawAmplitudeAxis(g, plotArea);
    drawFrequencyAxis(g, plotArea);

    // Draw border around plot area
    g.setColour(juce::Colour(0xff404040));
    g.drawRect(plotArea, 1.0f);

    // Draw spectrums for enabled tracks
    for (const auto& track : cachedTracks)
    {
        if (track.enabled)
            drawSpectrum(g, track, plotArea);
    }
}

void SpectrumDisplay::resized()
{
}

void SpectrumDisplay::timerCallback()
{
    cachedTracks = trackManager.getEnabledTracks();
    repaint();
}

float SpectrumDisplay::binToX(int bin, float width, double sampleRate) const
{
    // Calculate frequency for this bin
    float frequency = static_cast<float>(bin) * static_cast<float>(sampleRate) / static_cast<float>(SpectrumConstants::FFT_SIZE);
    return frequencyToX(frequency, width);
}

float SpectrumDisplay::frequencyToX(float frequency, float width) const
{
    // Clamp frequency to display range
    frequency = juce::jlimit(minFrequency, maxFrequency, frequency);

    // Logarithmic scale mapping
    float logMin = std::log10(minFrequency);
    float logMax = std::log10(maxFrequency);
    float logFreq = std::log10(frequency);

    return width * (logFreq - logMin) / (logMax - logMin);
}

float SpectrumDisplay::magnitudeToY(float magnitude, float height) const
{
    // Convert to dB
    float db;
    if (magnitude <= 0.0f)
        db = minDb;
    else
        db = 20.0f * std::log10(magnitude);

    // Clamp to display range
    db = juce::jlimit(minDb, maxDb, db);

    // Map dB to y-coordinate (0 dB at top, minDb at bottom)
    float normalized = (db - minDb) / (maxDb - minDb);
    return height * (1.0f - normalized);
}

void SpectrumDisplay::drawSpectrum(juce::Graphics& g, const TrackData& track, juce::Rectangle<float> area)
{
    juce::Path spectrumPath;
    bool pathStarted = false;

    float plotWidth = area.getWidth();
    float plotHeight = area.getHeight();

    for (int bin = 0; bin < SpectrumConstants::NUM_BINS; ++bin)
    {
        float frequency = static_cast<float>(bin) * static_cast<float>(track.sampleRate) / static_cast<float>(SpectrumConstants::FFT_SIZE);

        // Skip bins outside display range
        if (frequency < minFrequency || frequency > maxFrequency)
            continue;

        float x = area.getX() + binToX(bin, plotWidth, track.sampleRate);
        float y = area.getY() + magnitudeToY(track.spectrum[static_cast<size_t>(bin)], plotHeight);

        if (!pathStarted)
        {
            spectrumPath.startNewSubPath(x, y);
            pathStarted = true;
        }
        else
        {
            spectrumPath.lineTo(x, y);
        }
    }

    if (pathStarted)
    {
        // Flat line naturally appears for offline tracks (zeroed spectrum)
        g.setColour(track.colour);
        g.strokePath(spectrumPath, juce::PathStrokeType(1.5f));
    }
}

void SpectrumDisplay::drawFrequencyAxis(juce::Graphics& g, juce::Rectangle<float> area)
{
    g.setColour(juce::Colour(0xff606060));
    g.setFont(juce::FontOptions(11.0f));

    // Frequency labels to display
    const float frequencies[] = { 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 };
    const char* labels[] = { "20", "50", "100", "200", "500", "1k", "2k", "5k", "10k", "20k" };

    for (int i = 0; i < 10; ++i)
    {
        float x = area.getX() + frequencyToX(frequencies[i], area.getWidth());

        // Draw grid line
        g.setColour(juce::Colour(0xff303030));
        g.drawVerticalLine(static_cast<int>(x), area.getY(), area.getBottom());

        // Draw label
        g.setColour(juce::Colour(0xff808080));
        g.drawText(labels[i],
                   static_cast<int>(x) - 20, static_cast<int>(area.getBottom()) + 4,
                   40, 16,
                   juce::Justification::centredTop);
    }
}

void SpectrumDisplay::drawAmplitudeAxis(juce::Graphics& g, juce::Rectangle<float> area)
{
    g.setFont(juce::FontOptions(11.0f));

    // dB levels to display
    const float dbLevels[] = { 0, -12, -24, -36, -48, -60, -72, -84 };

    for (float db : dbLevels)
    {
        float y = area.getY() + magnitudeToY(std::pow(10.0f, db / 20.0f), area.getHeight());

        // Draw grid line
        g.setColour(juce::Colour(0xff303030));
        g.drawHorizontalLine(static_cast<int>(y), area.getX(), area.getRight());

        // Draw label
        g.setColour(juce::Colour(0xff808080));
        juce::String label = juce::String(static_cast<int>(db));
        g.drawText(label,
                   2, static_cast<int>(y) - 8,
                   leftMargin - 6, 16,
                   juce::Justification::centredRight);
    }
}
