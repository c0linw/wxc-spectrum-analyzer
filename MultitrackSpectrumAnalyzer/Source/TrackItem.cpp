#include "TrackItem.h"

// Helper class to listen to colour selector changes
class ColourChangeListener : public juce::ChangeListener
{
public:
    ColourChangeListener(juce::Component::SafePointer<TrackItem> item,
                         juce::ColourSelector* selector)
        : safeItem(item), colourSelector(selector)
    {
    }

    void changeListenerCallback(juce::ChangeBroadcaster*) override
    {
        if (safeItem != nullptr && colourSelector != nullptr)
        {
            auto newColour = colourSelector->getCurrentColour();
            safeItem->setTrackColour(newColour);
            if (safeItem->onColourChanged)
                safeItem->onColourChanged(newColour);
        }
    }

private:
    juce::Component::SafePointer<TrackItem> safeItem;
    juce::ColourSelector* colourSelector;
};

TrackItem::TrackItem(const juce::String& id,
                     const juce::String& name,
                     const juce::Colour& colour)
    : trackId(id), trackName(name), trackColour(colour)
{
    toggleButton.setToggleState(true, juce::dontSendNotification);
    toggleButton.setColour(juce::ToggleButton::tickColourId, colour);
    toggleButton.onClick = [this]() {
        isEnabled = toggleButton.getToggleState();
        if (onToggleChanged)
            onToggleChanged(isEnabled);
    };
    addAndMakeVisible(toggleButton);
}

void TrackItem::resized()
{
    auto bounds = getLocalBounds();

    // Toggle button needs more width (24px to avoid clipping)
    auto toggleArea = bounds.removeFromLeft(24);
    toggleButton.setBounds(toggleArea);
    bounds.removeFromLeft(spacing);

    // Color swatch on the right (20x20, centered vertically)
    auto swatchArea = bounds.removeFromRight(colourSwatchSize);
    bounds.removeFromRight(spacing);
    colourSwatchBounds = swatchArea.withSizeKeepingCentre(colourSwatchSize, colourSwatchSize);

    // Track name label gets remaining space (handled in paint)
}

void TrackItem::paint(juce::Graphics& g)
{
    // Draw color swatch as rounded rectangle
    g.setColour(trackColour);
    g.fillRoundedRectangle(colourSwatchBounds.toFloat(), 4.0f);

    // Draw border around swatch
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.drawRoundedRectangle(colourSwatchBounds.toFloat(), 4.0f, 1.0f);

    // Draw track name (between toggle and color swatch)
    auto textBounds = getLocalBounds()
        .withTrimmedLeft(24 + spacing)
        .withTrimmedRight(colourSwatchSize + spacing);
    g.setColour(isOffline ? juce::Colours::grey : juce::Colours::lightgrey);
    g.setFont(juce::FontOptions(14.0f));
    g.drawText(trackName, textBounds, juce::Justification::centredLeft);
}

void TrackItem::mouseDown(const juce::MouseEvent& event)
{
    if (colourSwatchBounds.contains(event.getPosition()))
    {
        showColourSelector();
    }
}

void TrackItem::mouseDrag(const juce::MouseEvent& event)
{
    // Start drag after 10px movement threshold
    if (event.getDistanceFromDragStart() > 10)
    {
        if (auto* container = juce::DragAndDropContainer::findParentDragContainerFor(this))
        {
            // Use trackId as drag source description
            container->startDragging(trackId, this);
        }
    }
}

void TrackItem::showColourSelector()
{
    auto colourSelector = std::make_unique<juce::ColourSelector>(
        juce::ColourSelector::showColourspace | juce::ColourSelector::showSliders);

    colourSelector->setCurrentColour(trackColour);
    colourSelector->setSize(300, 300);  // Square dimensions

    auto* selectorPtr = colourSelector.get();

    // Add change listener (will be owned and deleted by the ColourSelector)
    selectorPtr->addChangeListener(
        new ColourChangeListener(juce::Component::SafePointer<TrackItem>(this), selectorPtr)
    );

    // Get screen bounds of the color swatch for positioning
    auto swatchScreenBounds = localAreaToGlobal(colourSwatchBounds);

    // Use top-level component as parent to avoid clipping
    if (auto* topLevel = getTopLevelComponent())
    {
        auto swatchInTopLevel = topLevel->getLocalArea(nullptr, swatchScreenBounds);

        juce::CallOutBox::launchAsynchronously(
            std::move(colourSelector),
            swatchInTopLevel,
            topLevel
        );
    }
}

void TrackItem::setToggleState(bool enabled)
{
    isEnabled = enabled;
    toggleButton.setToggleState(enabled, juce::dontSendNotification);
}

void TrackItem::setTrackColour(const juce::Colour& colour)
{
    trackColour = colour;
    toggleButton.setColour(juce::ToggleButton::tickColourId, colour);
    repaint();
}

void TrackItem::setTrackName(const juce::String& name)
{
    trackName = name;
    repaint();
}

void TrackItem::setOfflineStatus(bool offline)
{
    isOffline = offline;
    repaint();
}
