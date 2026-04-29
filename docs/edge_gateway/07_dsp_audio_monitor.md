# 07 HiFi4 DSP 音频/振动监测

## 定位

HiFi4 DSP 用于音频、振动和信号处理预处理，不作为通用 MCU，不负责 Linux 外围 GPIO、RS485、MQTT 等控制面。

## DSP 侧任务

1. 接收音频或振动数据。
2. 滑动窗口缓存，例如 1024/2048 点。
3. 高通/低通/带通滤波。
4. 计算 RMS。
5. 计算 Peak。
6. FFT 提取主频。
7. 统计低/中/高频段能量。
8. 根据阈值或模型输出异常事件。
9. 将特征值传给 Linux。

## Linux 侧接口

当前骨架：

```c
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
```

当前 `DspIpc` 使用模拟数据，后续替换为：

- vendor IPC
- mailbox
- shared memory
- rpmsg
- char device

具体机制需结合全志 SDK 和 HiFi4 固件框架确认。

## 报警策略

- Peak 超阈值：冲击、敲击、异常声。
- RMS 长时间升高：设备运行状态异常。
- 主频偏移：轴承、电机类设备可能异常。
- 高频能量升高：摩擦、气泄漏等场景。

## HMI/MQTT 展示

HMI 显示 RMS、Peak、主频、频段能量条和报警状态。MQTT 上报 `edge/{device_id}/dsp_feature` 和 `edge/{device_id}/alarm`。
