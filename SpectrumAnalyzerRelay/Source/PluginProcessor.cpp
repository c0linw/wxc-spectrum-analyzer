#include "PluginProcessor.h"
#include "PluginEditor.h"

SpectrumAnalyzerRelayAudioProcessor::SpectrumAnalyzerRelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
                         )
#endif
{
}

SpectrumAnalyzerRelayAudioProcessor::~SpectrumAnalyzerRelayAudioProcessor()
{
}

const juce::String SpectrumAnalyzerRelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SpectrumAnalyzerRelayAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool SpectrumAnalyzerRelayAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool SpectrumAnalyzerRelayAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double SpectrumAnalyzerRelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SpectrumAnalyzerRelayAudioProcessor::getNumPrograms()
{
    return 1;
}

int SpectrumAnalyzerRelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SpectrumAnalyzerRelayAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String SpectrumAnalyzerRelayAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void SpectrumAnalyzerRelayAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void SpectrumAnalyzerRelayAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    spectrumProcessor.prepare(sampleRate);
}

void SpectrumAnalyzerRelayAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SpectrumAnalyzerRelayAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void SpectrumAnalyzerRelayAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't have corresponding input
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Process FFT if relay is enabled (using first channel only for analysis)
    if (relayEnabled.load() && totalNumInputChannels > 0)
    {
        const float* channelData = buffer.getReadPointer(0);
        spectrumProcessor.process(channelData, buffer.getNumSamples());
    }

    // Audio passes through unchanged - no modification needed since we only read
}

bool SpectrumAnalyzerRelayAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* SpectrumAnalyzerRelayAudioProcessor::createEditor()
{
    return new SpectrumAnalyzerRelayAudioProcessorEditor(*this);
}

void SpectrumAnalyzerRelayAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Save state to XML
    juce::XmlElement xml("SpectrumRelayState");
    xml.setAttribute("trackName", trackName);
    xml.setAttribute("relayEnabled", relayEnabled.load());
    copyXmlToBinary(xml, destData);
}

void SpectrumAnalyzerRelayAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Restore state from XML
    auto xml = getXmlFromBinary(data, sizeInBytes);
    if (xml != nullptr && xml->hasTagName("SpectrumRelayState"))
    {
        trackName = xml->getStringAttribute("trackName", "Track");
        relayEnabled = xml->getBoolAttribute("relayEnabled", true);
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SpectrumAnalyzerRelayAudioProcessor();
}
