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

    // Track name label
    trackNameLabel.setText("Track Name:", juce::dontSendNotification);
    addAndMakeVisible(trackNameLabel);

    // Track name editor
    trackNameEditor.setText(audioProcessor.getTrackName());
    trackNameEditor.onTextChange = [this]()
    {
        audioProcessor.setTrackName(trackNameEditor.getText());
    };
    addAndMakeVisible(trackNameEditor);

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

    setSize(280, 175);
}

SpectrumAnalyzerRelayAudioProcessorEditor::~SpectrumAnalyzerRelayAudioProcessorEditor()
{
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

    auto row = area.removeFromTop(24);
    trackNameLabel.setBounds(row.removeFromLeft(80));
    trackNameEditor.setBounds(row);

    area.removeFromTop(6);

    row = area.removeFromTop(24);
    oscPortLabel.setBounds(row.removeFromLeft(80));
    oscPortEditor.setBounds(row.removeFromLeft(70));

    area.removeFromTop(10);
    enableButton.setBounds(area.removeFromTop(24));
}
