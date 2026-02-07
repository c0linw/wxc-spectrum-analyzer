#include "TrackListPanel.h"

TrackListPanel::TrackListPanel(TrackManager& tm)
    : trackManager(tm),
      resizer(this, &constrainer, juce::ResizableEdgeComponent::rightEdge)
{
    titleLabel.setText("Tracks", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(16.0f, juce::Font::plain));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffc0c0c0));
    addAndMakeVisible(titleLabel);

    // Set up resizer
    constrainer.setMinimumWidth(100);
    constrainer.setMaximumWidth(400);
    addAndMakeVisible(resizer);
    addComponentListener(this);

    // Refresh track list at 4Hz
    startTimerHz(4);
}

TrackListPanel::~TrackListPanel()
{
    removeComponentListener(this);
    stopTimer();
}

void TrackListPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff252525));

    // Draw separator line on right edge
    g.setColour(juce::Colour(0xff404040));
    g.drawLine(static_cast<float>(getWidth() - 1), 0.0f,
               static_cast<float>(getWidth() - 1), static_cast<float>(getHeight()));

    // Draw drop indicator line
    if (dropInsertionIndex >= 0)
    {
        int titleHeight = 24 + 8;  // Title height + spacing
        int y = titleHeight + dropInsertionIndex * (28 + 4);

        g.setColour(juce::Colours::white);
        g.fillRect(0, y - 1, getWidth(), 2);
    }
}

void TrackListPanel::resized()
{
    auto area = getLocalBounds();

    // Position the resizer on the right edge
    resizer.setBounds(area.removeFromRight(4));

    area.reduce(8, 8);

    titleLabel.setBounds(area.removeFromTop(24));
    area.removeFromTop(8);

    // Layout track items
    for (auto* item : trackItems)
    {
        item->setBounds(area.removeFromTop(28));
        area.removeFromTop(4);
    }
}

void TrackListPanel::componentMovedOrResized(juce::Component& component,
                                              bool wasMoved,
                                              bool wasResized)
{
    if (wasResized && &component == this)
    {
        currentWidth = getWidth();
        if (auto* parent = getParentComponent())
            parent->resized();
    }
}

void TrackListPanel::timerCallback()
{
    rebuildTrackList();
}

bool TrackListPanel::isInterestedInDragSource(const SourceDetails& details)
{
    // Accept drags that contain a track ID string
    return details.description.toString().isNotEmpty();
}

void TrackListPanel::itemDragMove(const SourceDetails& details)
{
    dropInsertionIndex = getInsertionIndexForY(details.localPosition.y);
    repaint();
}

void TrackListPanel::itemDragExit(const SourceDetails& details)
{
    dropInsertionIndex = -1;
    repaint();
}

void TrackListPanel::itemDropped(const SourceDetails& details)
{
    juce::String draggedTrackId = details.description.toString();
    trackManager.reorderTrack(draggedTrackId, dropInsertionIndex);

    dropInsertionIndex = -1;
    repaint();
}

int TrackListPanel::getInsertionIndexForY(int y) const
{
    // Calculate which track position the Y coordinate corresponds to
    // Account for title label height (24px + 8px spacing) and track spacing (28px + 4px per item)
    int titleHeight = 24 + 8;
    int yRelative = y - titleHeight;

    if (yRelative < 0)
        return 0;

    int trackHeight = 28 + 4;  // height + spacing
    int index = (yRelative + trackHeight / 2) / trackHeight;

    return juce::jlimit(0, trackItems.size(), index);
}

void TrackListPanel::rebuildTrackList()
{
    auto tracks = trackManager.getActiveTracksOrdered();

    // Check if track list has changed (by track IDs)
    std::vector<juce::String> newTrackIds;
    for (const auto& track : tracks)
        newTrackIds.push_back(track.trackId);

    if (newTrackIds == currentTrackIds)
    {
        // No change in track list, just update item states
        for (int i = 0; i < trackItems.size() && i < static_cast<int>(tracks.size()); ++i)
        {
            // Find matching track and update state without triggering callback
            for (const auto& track : tracks)
            {
                if (track.trackId == currentTrackIds[static_cast<size_t>(i)])
                {
                    trackItems[i]->setToggleState(track.enabled);
                    trackItems[i]->setTrackName(track.trackName);
                    trackItems[i]->setTrackColour(track.colour);
                    trackItems[i]->setOfflineStatus(track.status == TrackStatus::Offline);
                    break;
                }
            }
        }
        return;
    }

    // Track list changed, rebuild items
    currentTrackIds = newTrackIds;
    trackItems.clear();

    for (const auto& track : tracks)
    {
        auto* item = trackItems.add(new TrackItem(
            track.trackId,
            track.trackName,
            track.colour
        ));

        item->setToggleState(track.enabled);
        item->setOfflineStatus(track.status == TrackStatus::Offline);

        // Wire up callbacks
        juce::String trackId = track.trackId;
        item->onToggleChanged = [this, trackId](bool enabled) {
            trackManager.setTrackEnabled(trackId, enabled);
        };

        item->onColourChanged = [this, trackId](juce::Colour colour) {
            trackManager.setTrackColour(trackId, colour);
        };

        addAndMakeVisible(item);
    }

    resized();
}
