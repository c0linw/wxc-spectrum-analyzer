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
    // Generate persistent unique ID for this plugin instance
    trackId = juce::Uuid().toString();
    
    // Default names
    dawTrackName = "Track " + trackId.substring(0, 8);
    customTrackName = "Track " + trackId.substring(0, 8);
    
    // Start heartbeat timer
    startTimer(SpectrumConstants::HEARTBEAT_INTERVAL_MS);
}

SpectrumAnalyzerRelayAudioProcessor::~SpectrumAnalyzerRelayAudioProcessor()
{
    stopTimer();
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
    connectOSC();
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

    // Process FFT if relay is enabled
    if (relayEnabled.load() && totalNumInputChannels > 0)
    {
        // For stereo, sum both channels to mono for analysis
        // For mono, just use the single channel
        if (totalNumInputChannels == 1)
        {
            // Mono input
            const float* channelData = buffer.getReadPointer(0);
            spectrumProcessor.process(channelData, buffer.getNumSamples());
        }
        else
        {
            // Stereo input - average to mono
            monoBuffer.resize(buffer.getNumSamples());
            const float* leftChannel = buffer.getReadPointer(0);
            const float* rightChannel = buffer.getReadPointer(1);

            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                monoBuffer[i] = (leftChannel[i] + rightChannel[i]) * 0.5f;
            }

            spectrumProcessor.process(monoBuffer.data(), buffer.getNumSamples());
        }

        // Send spectrum data via OSC when new FFT data is ready
        if (spectrumProcessor.isSpectrumReady())
            sendSpectrumViaOSC();
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

void SpectrumAnalyzerRelayAudioProcessor::connectOSC()
{
    oscSender.disconnect();
    oscConnected = oscSender.connect("127.0.0.1", oscPort);
}

void SpectrumAnalyzerRelayAudioProcessor::timerCallback()
{
    sendHeartbeat();
}

void SpectrumAnalyzerRelayAudioProcessor::sendHeartbeat()
{
    if (!oscConnected.load())
        return;

    // Build OSC address: /wxc-tools/heartbeat/<trackId>
    juce::String address = SpectrumConstants::OSC_HEARTBEAT_PREFIX + trackId;

    // Create heartbeat message with effective track name and sample rate
    juce::OSCMessage message(address.toRawUTF8());
    message.addString(getEffectiveTrackName());
    message.addFloat32(static_cast<float>(spectrumProcessor.getSampleRate()));

    oscSender.send(message);
}

void SpectrumAnalyzerRelayAudioProcessor::setOscPort(int port)
{
    if (port != oscPort && port > 0 && port <= 65535)
    {
        oscPort = port;
        connectOSC();
    }
}

void SpectrumAnalyzerRelayAudioProcessor::sendSpectrumViaOSC()
{
    if (!oscConnected.load())
        return;

    std::array<float, SpectrumConstants::NUM_BINS> spectrum;
    spectrumProcessor.getSpectrum(spectrum);

    // Build OSC address: /wxc-tools/spectrum/<trackId>
    juce::String address = SpectrumConstants::OSC_ADDRESS_PREFIX + trackId;

    // Create OSC message with spectrum data
    // Format: [trackName, fftSize, sampleRate, magnitude[0], magnitude[1], ..., magnitude[NUM_BINS-1]]
    juce::OSCMessage message(address.toRawUTF8());
    message.addString(getEffectiveTrackName());
    message.addFloat32(static_cast<float>(SpectrumConstants::FFT_SIZE));
    message.addFloat32(static_cast<float>(spectrumProcessor.getSampleRate()));

    for (int i = 0; i < SpectrumConstants::NUM_BINS; ++i)
        message.addFloat32(spectrum[static_cast<size_t>(i)]);

    oscSender.send(message);
}

void SpectrumAnalyzerRelayAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Save state to XML
    juce::XmlElement xml("SpectrumRelayState");
    xml.setAttribute("trackId", trackId);
    xml.setAttribute("dawTrackName", dawTrackName);
    xml.setAttribute("customTrackName", customTrackName);
    xml.setAttribute("useCustomTrackName", useCustomTrackName);
    xml.setAttribute("relayEnabled", relayEnabled.load());
    xml.setAttribute("oscPort", oscPort);
    copyXmlToBinary(xml, destData);
}

void SpectrumAnalyzerRelayAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Restore state from XML
    auto xml = getXmlFromBinary(data, sizeInBytes);
    if (xml != nullptr && xml->hasTagName("SpectrumRelayState"))
    {
        trackId = xml->getStringAttribute("trackId");
        if (trackId.isEmpty())  // Generate if missing (backward compatibility)
            trackId = juce::Uuid().toString();
        
        // Load names with backward compatibility
        juce::String legacyTrackName = xml->getStringAttribute("trackName", "");
        if (!legacyTrackName.isEmpty())
        {
            // Old format: migrate to new system
            dawTrackName = legacyTrackName;
            customTrackName = legacyTrackName;
        }
        else
        {
            dawTrackName = xml->getStringAttribute("dawTrackName", "Track " + trackId.substring(0, 8));
            customTrackName = xml->getStringAttribute("customTrackName", "Track " + trackId.substring(0, 8));
        }
        
        useCustomTrackName = xml->getBoolAttribute("useCustomTrackName", false);
        relayEnabled = xml->getBoolAttribute("relayEnabled", true);
        oscPort = xml->getIntAttribute("oscPort", SpectrumConstants::DEFAULT_OSC_PORT);
    }
}

void SpectrumAnalyzerRelayAudioProcessor::updateTrackProperties(const TrackProperties& properties)
{
    // Auto-populate DAW track name if provided
    if (properties.name.has_value())
        dawTrackName = *properties.name;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SpectrumAnalyzerRelayAudioProcessor();
}
