#include "MainComponent.h"
#include "../../Common/SpectrumData.h"

MainComponent::MainComponent()
    : trackListPanel(trackManager),
      spectrumDisplay(trackManager)
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

    // Spectrum display
    addAndMakeVisible(spectrumDisplay);

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

    // Spectrum display fills remaining area
    spectrumDisplay.setBounds(area);
}

void MainComponent::oscMessageReceived(const juce::OSCMessage& message)
{
    juce::String address = message.getAddressPattern().toString();

    // Handle heartbeat messages: /wxc-tools/heartbeat/<trackId>
    juce::String heartbeatPrefix = SpectrumConstants::OSC_HEARTBEAT_PREFIX;
    if (address.startsWith(heartbeatPrefix))
    {
        juce::String trackId = address.substring(heartbeatPrefix.length());
        if (trackId.isEmpty() || message.size() < 2)
            return;

        if (!message[0].isString() || !message[1].isFloat32())
            return;

        juce::String trackName = message[0].getString();
        double sampleRate = static_cast<double>(message[1].getFloat32());
        trackManager.updateTrackPresence(trackId, trackName, sampleRate);
        return;
    }

    // Handle spectrum messages: /wxc-tools/spectrum/<trackId>
    juce::String spectrumPrefix = SpectrumConstants::OSC_ADDRESS_PREFIX;
    if (address.startsWith(spectrumPrefix))
    {
        juce::String trackId = address.substring(spectrumPrefix.length());
        if (trackId.isEmpty())
            return;

        // Expected format: [trackName, fftSize, sampleRate, magnitude[0], ..., magnitude[NUM_BINS-1]]
        if (message.size() < 4)
            return;

        if (!message[0].isString() || !message[1].isFloat32() || !message[2].isFloat32())
            return;

        juce::String trackName = message[0].getString();
        double sampleRate = static_cast<double>(message[2].getFloat32());

        // Parse spectrum data (starts at index 3)
        int numBins = message.size() - 3;
        std::vector<float> spectrumData(static_cast<size_t>(numBins));

        for (int i = 0; i < numBins; ++i)
        {
            if (message[i + 3].isFloat32())
                spectrumData[static_cast<size_t>(i)] = message[i + 3].getFloat32();
        }

        // Update track manager with spectrum data
        trackManager.updateTrack(trackId, trackName, spectrumData.data(), numBins, sampleRate);
    }
}

void MainComponent::timerCallback()
{
    trackManager.updateStaleTrack();
    updateStatusLabel();
}

void MainComponent::updateStatusLabel()
{
    int trackCount = trackManager.getTrackCount();
    juce::String statusText = "Listening on port " + juce::String(SpectrumConstants::DEFAULT_OSC_PORT);
    statusText += " | Active tracks: " + juce::String(trackCount);
    statusLabel.setText(statusText, juce::dontSendNotification);
}
