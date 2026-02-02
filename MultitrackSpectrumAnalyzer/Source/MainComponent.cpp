#include "MainComponent.h"
#include "../../Common/SpectrumData.h"

MainComponent::MainComponent()
    : trackListPanel(trackManager)
{
    // Title label
    titleLabel.setText("Multitrack Spectrum Analyzer", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    // Status label
    statusLabel.setFont(juce::FontOptions(14.0f));
    statusLabel.setJustificationType(juce::Justification::centredLeft);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(statusLabel);

    // Track list panel (left sidebar)
    addAndMakeVisible(trackListPanel);

    // Start OSC receiver
    if (oscReceiver.connect(SpectrumConstants::DEFAULT_OSC_PORT))
    {
        oscReceiver.addListener(this);
        updateStatusLabel();
    }
    else
    {
        statusLabel.setText("Failed to bind to port " + juce::String(SpectrumConstants::DEFAULT_OSC_PORT),
                           juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, juce::Colours::red);
    }

    // Start timer for stale track removal and status updates
    startTimerHz(2);

    setSize(800, 600);
}

MainComponent::~MainComponent()
{
    stopTimer();
    oscReceiver.removeListener(this);
    oscReceiver.disconnect();
}

void MainComponent::paint(juce::Graphics& g)
{
    // Dark background
    g.fillAll(juce::Colour(0xff1a1a1a));
}

void MainComponent::resized()
{
    auto area = getLocalBounds();

    // Title at top
    titleLabel.setBounds(area.removeFromTop(50));

    // Status bar at bottom
    statusLabel.setBounds(area.removeFromBottom(30).reduced(10, 0));

    // Left sidebar for track list
    trackListPanel.setBounds(area.removeFromLeft(180));

    // Remaining area is for spectrum display (Phase 5)
}

void MainComponent::oscMessageReceived(const juce::OSCMessage& message)
{
    juce::String address = message.getAddressPattern().toString();

    // Handle heartbeat messages: /wxc-tools/heartbeat/<trackName>
    juce::String heartbeatPrefix = SpectrumConstants::OSC_HEARTBEAT_PREFIX;
    if (address.startsWith(heartbeatPrefix))
    {
        juce::String trackName = address.substring(heartbeatPrefix.length());
        if (trackName.isEmpty() || message.size() < 1)
            return;

        if (!message[0].isFloat32())
            return;

        double sampleRate = static_cast<double>(message[0].getFloat32());
        trackManager.updateTrackPresence(trackName, sampleRate);
        return;
    }

    // Handle spectrum messages: /wxc-tools/spectrum/<trackName>
    juce::String spectrumPrefix = SpectrumConstants::OSC_ADDRESS_PREFIX;
    if (address.startsWith(spectrumPrefix))
    {
        juce::String trackName = address.substring(spectrumPrefix.length());
        if (trackName.isEmpty())
            return;

        // Expected format: [fftSize, sampleRate, magnitude[0], ..., magnitude[NUM_BINS-1]]
        if (message.size() < 3)
            return;

        if (!message[0].isFloat32() || !message[1].isFloat32())
            return;

        double sampleRate = static_cast<double>(message[1].getFloat32());

        // Parse spectrum data
        int numBins = message.size() - 2;
        std::vector<float> spectrumData(static_cast<size_t>(numBins));

        for (int i = 0; i < numBins; ++i)
        {
            if (message[i + 2].isFloat32())
                spectrumData[static_cast<size_t>(i)] = message[i + 2].getFloat32();
        }

        // Update track manager with spectrum data
        trackManager.updateTrack(trackName, spectrumData.data(), numBins, sampleRate);
    }
}

void MainComponent::timerCallback()
{
    trackManager.removeStaleTrack();
    updateStatusLabel();
}

void MainComponent::updateStatusLabel()
{
    int trackCount = trackManager.getTrackCount();
    juce::String statusText = "Listening on port " + juce::String(SpectrumConstants::DEFAULT_OSC_PORT);
    statusText += " | Active tracks: " + juce::String(trackCount);
    statusLabel.setText(statusText, juce::dontSendNotification);
}
