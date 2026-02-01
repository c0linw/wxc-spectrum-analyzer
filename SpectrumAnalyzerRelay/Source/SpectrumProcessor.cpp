#include "SpectrumProcessor.h"

SpectrumProcessor::SpectrumProcessor()
{
    inputBuffer.fill(0.0f);
    fftData.fill(0.0f);
    magnitudeSpectrum.fill(0.0f);
}

void SpectrumProcessor::prepare(double sampleRate)
{
    currentSampleRate = sampleRate;
    inputBufferIndex = 0;
    samplesSinceLastFFT = 0;
    inputBuffer.fill(0.0f);
    spectrumReady = false;
}

void SpectrumProcessor::process(const float* inputData, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        // Add sample to circular buffer
        inputBuffer[inputBufferIndex] = inputData[i];
        inputBufferIndex = (inputBufferIndex + 1) % SpectrumConstants::FFT_SIZE;
        ++samplesSinceLastFFT;

        // Perform FFT when we've accumulated enough new samples (hop size)
        if (samplesSinceLastFFT >= SpectrumConstants::HOP_SIZE)
        {
            processFFT();
            samplesSinceLastFFT = 0;
        }
    }
}

void SpectrumProcessor::processFFT()
{
    // Copy samples from circular buffer to FFT buffer in correct order
    const int firstPartSize = SpectrumConstants::FFT_SIZE - inputBufferIndex;
    std::copy(inputBuffer.begin() + inputBufferIndex,
              inputBuffer.begin() + inputBufferIndex + firstPartSize,
              fftData.begin());
    std::copy(inputBuffer.begin(),
              inputBuffer.begin() + inputBufferIndex,
              fftData.begin() + firstPartSize);

    // Apply window function
    window.multiplyWithWindowingTable(fftData.data(), SpectrumConstants::FFT_SIZE);

    // Perform forward FFT (in-place, real input)
    fft.performFrequencyOnlyForwardTransform(fftData.data());

    // Convert to magnitude spectrum (normalized)
    const float maxMagnitude = static_cast<float>(SpectrumConstants::FFT_SIZE);
    for (int i = 0; i < SpectrumConstants::NUM_BINS; ++i)
    {
        // fftData now contains magnitudes after performFrequencyOnlyForwardTransform
        float magnitude = fftData[i] / maxMagnitude;
        magnitudeSpectrum[i] = magnitude;
    }

    spectrumReady = true;
}

bool SpectrumProcessor::isSpectrumReady() const
{
    return spectrumReady.load();
}

void SpectrumProcessor::getSpectrum(std::array<float, SpectrumConstants::NUM_BINS>& output)
{
    output = magnitudeSpectrum;
    spectrumReady = false;
}
