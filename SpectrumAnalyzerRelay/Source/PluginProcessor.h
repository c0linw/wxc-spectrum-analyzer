#pragma once

#include <JuceHeader.h>
#include "SpectrumProcessor.h"

class SpectrumAnalyzerRelayAudioProcessor : public juce::AudioProcessor
{
public:
    SpectrumAnalyzerRelayAudioProcessor();
    ~SpectrumAnalyzerRelayAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Track configuration accessors
    juce::String getTrackName() const { return trackName; }
    void setTrackName(const juce::String& name) { trackName = name; }

    bool isRelayEnabled() const { return relayEnabled.load(); }
    void setRelayEnabled(bool enabled) { relayEnabled = enabled; }

private:
    SpectrumProcessor spectrumProcessor;

    juce::String trackName { "Track" };
    std::atomic<bool> relayEnabled { true };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzerRelayAudioProcessor)
};
