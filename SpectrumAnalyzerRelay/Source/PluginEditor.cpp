#include "PluginProcessor.h"
#include "PluginEditor.h"

SpectrumAnalyzerRelayAudioProcessorEditor::SpectrumAnalyzerRelayAudioProcessorEditor(SpectrumAnalyzerRelayAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Title label
    titleLabel.setText("Spectrum Relay", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(20.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    // DAW track name (read-only display)
    dawTrackNameLabel.setText("DAW Track:", juce::dontSendNotification);
    dawTrackNameLabel.setJustificationType(juce::Justification::left);
    addAndMakeVisible(dawTrackNameLabel);
    
    dawTrackNameValue.setText(audioProcessor.getDawTrackName(), juce::dontSendNotification);
    dawTrackNameValue.setJustificationType(juce::Justification::left);
    dawTrackNameValue.setColour(juce::Label::backgroundColourId, juce::Colour(0xff1a1a1a));
    dawTrackNameValue.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(dawTrackNameValue);

    // Custom track name checkbox
    useCustomNameCheckbox.setButtonText("Custom Track Name");
    useCustomNameCheckbox.setToggleState(audioProcessor.isUsingCustomTrackName(), juce::dontSendNotification);
    useCustomNameCheckbox.onClick = [this]()
    {
        bool useCustom = useCustomNameCheckbox.getToggleState();
        audioProcessor.setUsingCustomTrackName(useCustom);
        customTrackNameEditor.setEnabled(useCustom);
    };
    addAndMakeVisible(useCustomNameCheckbox);

    // Custom track name editor (indented under checkbox)
    customTrackNameEditor.setText(audioProcessor.getCustomTrackName());
    customTrackNameEditor.setEnabled(audioProcessor.isUsingCustomTrackName());
    customTrackNameEditor.onTextChange = [this]()
    {
        audioProcessor.setCustomTrackName(customTrackNameEditor.getText());
    };
    addAndMakeVisible(customTrackNameEditor);

    // OSC port label
    oscPortLabel.setText("OSC Port:", juce::dontSendNotification);
    addAndMakeVisible(oscPortLabel);

    // OSC port editor
    oscPortEditor.setText(juce::String(audioProcessor.getOscPort()));
    oscPortEditor.setInputRestrictions(5, "0123456789");
    oscPortEditor.onFocusLost = [this]()
    {
        int port = oscPortEditor.getText().getIntValue();
        if (port > 0 && port <= 65535)
            audioProcessor.setOscPort(port);
        else
            oscPortEditor.setText(juce::String(audioProcessor.getOscPort()));
    };
    oscPortEditor.onReturnKey = [this]()
    {
        int port = oscPortEditor.getText().getIntValue();
        if (port > 0 && port <= 65535)
            audioProcessor.setOscPort(port);
        else
            oscPortEditor.setText(juce::String(audioProcessor.getOscPort()));
    };
    addAndMakeVisible(oscPortEditor);

    // Enable button
    enableButton.setButtonText("Enabled");
    enableButton.setToggleState(audioProcessor.isRelayEnabled(), juce::dontSendNotification);
    enableButton.onClick = [this]()
    {
        audioProcessor.setRelayEnabled(enableButton.getToggleState());
    };
    addAndMakeVisible(enableButton);

    // Start timer to refresh DAW track name (in case DAW provides it after editor is created)
    startTimerHz(2);  // Check twice per second

    setSize(300, 240);
}

SpectrumAnalyzerRelayAudioProcessorEditor::~SpectrumAnalyzerRelayAudioProcessorEditor()
{
    stopTimer();
}

void SpectrumAnalyzerRelayAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2d2d2d));
}

void SpectrumAnalyzerRelayAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(10);

    titleLabel.setBounds(area.removeFromTop(30));
    area.removeFromTop(10);

    // DAW track name display
    auto row = area.removeFromTop(24);
    dawTrackNameLabel.setBounds(row.removeFromLeft(80));
    dawTrackNameValue.setBounds(row);

    area.removeFromTop(6);

    // Custom name checkbox
    useCustomNameCheckbox.setBounds(area.removeFromTop(24));
    
    area.removeFromTop(2);
    
    // Custom name editor (indented)
    row = area.removeFromTop(24);
    row.removeFromLeft(20);  // Indent
    customTrackNameEditor.setBounds(row);

    area.removeFromTop(6);

    // OSC port
    row = area.removeFromTop(24);
    oscPortLabel.setBounds(row.removeFromLeft(80));
    oscPortEditor.setBounds(row.removeFromLeft(70));

    area.removeFromTop(10);
    enableButton.setBounds(area.removeFromTop(24));
}

void SpectrumAnalyzerRelayAudioProcessorEditor::timerCallback()
{
    // Update DAW track name display if it changed in the processor (e.g., from DAW)
    juce::String currentDawName = audioProcessor.getDawTrackName();
    if (dawTrackNameValue.getText() != currentDawName)
        dawTrackNameValue.setText(currentDawName, juce::dontSendNotification);
}
