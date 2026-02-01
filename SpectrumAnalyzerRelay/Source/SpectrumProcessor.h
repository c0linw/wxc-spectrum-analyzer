#pragma once

#include <JuceHeader.h>
#include "../../Common/SpectrumData.h"

class SpectrumProcessor
{
public:
    SpectrumProcessor();

    /// Called when sample rate changes. Resets internal buffers.
    void prepare(double sampleRate);

    /// Process incoming audio samples. Accumulates into circular buffer
    /// and triggers FFT when enough samples collected.
    void process(const float* inputData, int numSamples);

    /// Returns true if new spectrum data is available since last getSpectrum() call.
    bool isSpectrumReady() const;

    /// Copies current magnitude spectrum (normalized 0-1) to output array.
    /// Clears the ready flag.
    void getSpectrum(std::array<float, SpectrumConstants::NUM_BINS>& output);

    double getSampleRate() const { return currentSampleRate; }

private:
    void processFFT();

    // JUCE FFT
    juce::dsp::FFT fft { SpectrumConstants::FFT_ORDER };
    juce::dsp::WindowingFunction<float> window {
        SpectrumConstants::FFT_SIZE,
        juce::dsp::WindowingFunction<float>::hann
    };

    // Circular input buffer
    std::array<float, SpectrumConstants::FFT_SIZE> inputBuffer {};
    int inputBufferIndex { 0 };
    int samplesSinceLastFFT { 0 };

    // FFT working buffer (needs 2x size for real FFT)
    std::array<float, SpectrumConstants::FFT_SIZE * 2> fftData {};

    // Output magnitude spectrum
    std::array<float, SpectrumConstants::NUM_BINS> magnitudeSpectrum {};

    // Thread-safe ready flag
    std::atomic<bool> spectrumReady { false };

    double currentSampleRate { 44100.0 };
};
