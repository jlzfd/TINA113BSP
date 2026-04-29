#include "audio_dsp/dsp_ipc.h"

#include "common/log.h"

#include <cmath>
#include <ctime>

namespace edge {

bool DspIpc::open()
{
    // 初期使用模拟数据。量产时替换为 vendor IPC、mailbox、shared memory 或 rpmsg。
    EG_INFO("HiFi4 DSP IPC placeholder opened");
    return true;
}

void DspIpc::close() {}

bool DspIpc::readFeature(dsp_audio_feature_t &feature)
{
    ++seq_;
    feature.seq = seq_;
    feature.timestamp_ms = static_cast<uint32_t>(std::time(nullptr) * 1000U);
    feature.rms = 0.12f + 0.03f * std::sin(seq_ * 0.1f);
    feature.peak = 0.35f + 0.05f * std::sin(seq_ * 0.07f);
    feature.main_freq = 120.0f + 10.0f * std::sin(seq_ * 0.03f);
    feature.band_energy_low = 0.2f;
    feature.band_energy_mid = 0.5f;
    feature.band_energy_high = 0.1f;
    feature.alarm_flags = feature.peak > 0.39f ? 0x1 : 0x0;
    feature.crc16 = 0;
    return true;
}

} // namespace edge
