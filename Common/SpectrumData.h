#pragma once

// Shared constants for spectrum analysis
namespace SpectrumConstants
{
    // FFT configuration
    constexpr int FFT_ORDER = 11;              // 2^11 = 2048 samples
    constexpr int FFT_SIZE = 1 << FFT_ORDER;   // 2048
    constexpr int NUM_BINS = FFT_SIZE / 2;     // 1024 frequency bins
    constexpr int HOP_SIZE = FFT_SIZE / 4;     // 75% overlap

    // OSC configuration
    constexpr int DEFAULT_OSC_PORT = 58964;
    constexpr const char* OSC_ADDRESS_PREFIX = "/wxc-tools/spectrum/";
    constexpr const char* OSC_HEARTBEAT_PREFIX = "/wxc-tools/heartbeat/";
    constexpr int HEARTBEAT_INTERVAL_MS = 1000;  // Send heartbeat every 1 second

    // Display configuration
    constexpr float MIN_DB = -100.0f;
    constexpr float MAX_DB = 0.0f;
    constexpr int TRACK_TIMEOUT_MS = 1500;  // Mark track offline after 1.5 seconds of no data
}
