#pragma once

#include <JuceHeader.h>
#include "TrackManager.h"
#include "TrackItem.h"

class TrackListPanel : public juce::Component,
                       public juce::DragAndDropContainer,
                       public juce::DragAndDropTarget,
                       private juce::Timer,
                       private juce::ComponentListener
{
public:
    TrackListPanel(TrackManager& trackManager);
    ~TrackListPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    int getPreferredWidth() const { return currentWidth; }

    void componentMovedOrResized(juce::Component& component,
                                bool wasMoved,
                                bool wasResized) override;

    // DragAndDropTarget interface
    bool isInterestedInDragSource(const SourceDetails& details) override;
    void itemDragMove(const SourceDetails& details) override;
    void itemDragExit(const SourceDetails& details) override;
    void itemDropped(const SourceDetails& details) override;

private:
    void timerCallback() override;
    void rebuildTrackList();
    int getInsertionIndexForY(int y) const;

    TrackManager& trackManager;

    juce::Label titleLabel;
    juce::OwnedArray<TrackItem> trackItems;
    std::vector<juce::String> currentTrackIds;  // Track IDs for change detection

    int dropInsertionIndex { -1 };  // -1 = no drop indicator

    juce::ResizableEdgeComponent resizer;
    juce::ComponentBoundsConstrainer constrainer;
    int currentWidth { 180 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackListPanel)
};
