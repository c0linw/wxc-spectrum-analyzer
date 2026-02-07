#include "MainComponent.h"
#include "../../Common/SpectrumData.h"

MainComponent::MainComponent()
    : trackListPanel(trackManager),
      spectrumDisplay(trackManager)
{
    // Title label
    titleLabel.setText("Multitrack Spectrum Analyzer", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(24.0f, juce::Font::plain));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffc0c0c0));
    addAndMakeVisible(titleLabel);

    // Status label
    statusLabel.setFont(juce::FontOptions(14.0f));
    statusLabel.setJustificationType(juce::Justification::centredLeft);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(statusLabel);

    // Setup display control dropdowns
    setupDisplayControls();

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
    // 30Hz provides smooth visual updates and consistent decay with active tracks
    startTimerHz(30);

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

    // Control bar below title
    auto controlArea = area.removeFromTop(35);
    controlArea.reduce(10, 5);

    // Display mode dropdown (left side)
    displayModeLabel.setBounds(controlArea.removeFromLeft(90));
    displayModeCombo.setBounds(controlArea.removeFromLeft(120));
    controlArea.removeFromLeft(20); // Spacing

    // dB scaling dropdown (left side)
    dbScalingLabel.setBounds(controlArea.removeFromLeft(80));
    dbScalingCombo.setBounds(controlArea.removeFromLeft(120));

    // Status bar at bottom
    statusLabel.setBounds(area.removeFromBottom(30).reduced(10, 0));

    // Left sidebar for track list (resizable)
    trackListPanel.setBounds(area.removeFromLeft(trackListPanel.getPreferredWidth()));

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

void MainComponent::setupDisplayControls()
{
    // Display mode label and combo
    displayModeLabel.setText("Display Mode:", juce::dontSendNotification);
    displayModeLabel.setFont(juce::FontOptions(13.0f));
    displayModeLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    displayModeLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(displayModeLabel);

    displayModeCombo.addItem("Overlay", 1);
    displayModeCombo.addItem("Stacked", 2);
    displayModeCombo.setSelectedId(1, juce::dontSendNotification);
    displayModeCombo.onChange = [this]() { onDisplayModeChanged(); };
    addAndMakeVisible(displayModeCombo);

    // dB scaling label and combo
    dbScalingLabel.setText("dB Scale:", juce::dontSendNotification);
    dbScalingLabel.setFont(juce::FontOptions(13.0f));
    dbScalingLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    dbScalingLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(dbScalingLabel);

    dbScalingCombo.addItem("Linear", 1);
    dbScalingCombo.addItem("Compressed", 2);
    dbScalingCombo.setSelectedId(1, juce::dontSendNotification);
    dbScalingCombo.onChange = [this]() { onDbScalingChanged(); };
    addAndMakeVisible(dbScalingCombo);
}

void MainComponent::onDisplayModeChanged()
{
    int selectedId = displayModeCombo.getSelectedId();
    if (selectedId == 1)
        spectrumDisplay.setDisplayMode(DisplayMode::Overlay);
    else if (selectedId == 2)
        spectrumDisplay.setDisplayMode(DisplayMode::Stacked);
}

void MainComponent::onDbScalingChanged()
{
    int selectedId = dbScalingCombo.getSelectedId();
    if (selectedId == 1)
        spectrumDisplay.setDbScaling(DbScaling::Linear);
    else if (selectedId == 2)
        spectrumDisplay.setDbScaling(DbScaling::Compressed);
}
