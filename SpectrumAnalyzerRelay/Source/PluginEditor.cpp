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

    // Enable button
    enableButton.setButtonText("Enabled");
    enableButton.setToggleState(audioProcessor.isRelayEnabled(), juce::dontSendNotification);
    enableButton.onClick = [this]()
    {
        audioProcessor.setRelayEnabled(enableButton.getToggleState());
    };
    addAndMakeVisible(enableButton);

    setSize(280, 140);
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

    area.removeFromTop(10);
    enableButton.setBounds(area.removeFromTop(24));
}
