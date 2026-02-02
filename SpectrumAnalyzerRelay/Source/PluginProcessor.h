#pragma once

#include <JuceHeader.h>
#include "SpectrumProcessor.h"
#include "../../Common/SpectrumData.h"

class SpectrumAnalyzerRelayAudioProcessor : public juce::AudioProcessor,
                                           private juce::Timer
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

    // Called by DAW to provide track info (name, colour)
    void updateTrackProperties(const TrackProperties& properties) override;

    bool isRelayEnabled() const { return relayEnabled.load(); }
    void setRelayEnabled(bool enabled) { relayEnabled = enabled; }

    int getOscPort() const { return oscPort; }
    void setOscPort(int port);

    bool isOscConnected() const { return oscConnected.load(); }

    // Track naming accessors
    juce::String getDawTrackName() const { return dawTrackName; }
    juce::String getCustomTrackName() const { return customTrackName; }
    void setCustomTrackName(const juce::String& name) { customTrackName = name; }
    
    bool isUsingCustomTrackName() const { return useCustomTrackName; }
    void setUsingCustomTrackName(bool useCustom) { useCustomTrackName = useCustom; }
    
    /// Returns the track name to send via OSC (DAW name or custom name based on flag)
    juce::String getEffectiveTrackName() const 
    { 
        return useCustomTrackName ? customTrackName : dawTrackName; 
    }

private:
    void timerCallback() override;
    void connectOSC();
    void sendHeartbeat();
    void sendSpectrumViaOSC();
    SpectrumProcessor spectrumProcessor;

    juce::String trackId;              // Unique identifier (UUID)
    juce::String dawTrackName;         // Track name from DAW (via updateTrackProperties)
    juce::String customTrackName;      // User-defined custom name
    bool useCustomTrackName { false }; // If true, send customTrackName; otherwise send dawTrackName
    std::atomic<bool> relayEnabled { true };

    juce::OSCSender oscSender;
    int oscPort { SpectrumConstants::DEFAULT_OSC_PORT };
    std::atomic<bool> oscConnected { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzerRelayAudioProcessor)
};
