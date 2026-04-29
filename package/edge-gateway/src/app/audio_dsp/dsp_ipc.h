#pragma once

#include <cstdint>

namespace edge {

typedef struct {
    uint32_t seq;
    uint32_t timestamp_ms;
    float rms;
    float peak;
    float main_freq;
    float band_energy_low;
    float band_energy_mid;
    float band_energy_high;
    uint32_t alarm_flags;
    uint16_t crc16;
} dsp_audio_feature_t;

class DspIpc {
public:
    bool open();
    void close();
    bool readFeature(dsp_audio_feature_t &feature);

private:
    uint32_t seq_ = 0;
};

} // namespace edge
