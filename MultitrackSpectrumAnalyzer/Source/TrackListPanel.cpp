#include "TrackListPanel.h"

TrackListPanel::TrackListPanel(TrackManager& tm)
    : trackManager(tm)
{
    titleLabel.setText("Tracks", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    // Refresh track list at 4Hz
    startTimerHz(4);
}

TrackListPanel::~TrackListPanel()
{
    stopTimer();
}

void TrackListPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff252525));

    // Draw separator line on right edge
    g.setColour(juce::Colour(0xff404040));
    g.drawLine(static_cast<float>(getWidth() - 1), 0.0f,
               static_cast<float>(getWidth() - 1), static_cast<float>(getHeight()));
}

void TrackListPanel::resized()
{
    auto area = getLocalBounds().reduced(8);

    titleLabel.setBounds(area.removeFromTop(24));
    area.removeFromTop(8);

    // Layout track buttons
    for (auto* button : trackButtons)
    {
        button->setBounds(area.removeFromTop(28));
        area.removeFromTop(4);
    }
}

void TrackListPanel::timerCallback()
{
    rebuildTrackList();
}

void TrackListPanel::rebuildTrackList()
{
    auto tracks = trackManager.getActiveTracks();

    // Check if track list has changed
    std::vector<juce::String> newTrackNames;
    for (const auto& track : tracks)
        newTrackNames.push_back(track.name);

    // Sort for consistent ordering
    std::sort(newTrackNames.begin(), newTrackNames.end());

    if (newTrackNames == currentTrackNames)
    {
        // No change in track list, just update button states
        for (int i = 0; i < trackButtons.size() && i < static_cast<int>(tracks.size()); ++i)
        {
            // Find matching track and update toggle state without triggering callback
            for (const auto& track : tracks)
            {
                if (track.name == currentTrackNames[static_cast<size_t>(i)])
                {
                    trackButtons[i]->setToggleState(track.enabled, juce::dontSendNotification);
                    break;
                }
            }
        }
        return;
    }

    // Track list changed, rebuild buttons
    currentTrackNames = newTrackNames;
    trackButtons.clear();

    for (const auto& track : tracks)
    {
        auto* button = new juce::ToggleButton(track.name);
        button->setToggleState(track.enabled, juce::dontSendNotification);

        // Set colour indicator
        button->setColour(juce::ToggleButton::tickColourId, track.colour);
        button->setColour(juce::ToggleButton::textColourId, juce::Colours::lightgrey);

        // Capture track name for callback
        juce::String trackName = track.name;
        button->onClick = [this, trackName, button]()
        {
            trackManager.setTrackEnabled(trackName, button->getToggleState());
        };

        addAndMakeVisible(button);
        trackButtons.add(button);
    }

    resized();
}
