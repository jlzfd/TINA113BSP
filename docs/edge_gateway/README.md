# 基于全志 T113Pro 的工业多协议边缘网关与视觉音频监控 BSP 系统

英文名称：Industrial Multi-Protocol Edge Gateway with Vision-Audio Monitoring Based on Allwinner T113Pro

## 项目介绍

本项目基于当前 Tina SDK 工作区构建，目标平台为 100ASK T113-Pro / T113Pro V1.3 开发板。系统以 BSP Bring-up 为核心，覆盖 U-Boot、Linux Kernel、Device Tree、RootFS、驱动联调、外设适配、应用自启动、OverlayFS、A/B OTA、Watchdog、视觉采集、工业通信和 HiFi4 DSP 音频/振动特征处理。

本项目不采用 A7 + RISC-V 架构。T113Pro 当前资源按双核 Cortex-A7 + 单核 HiFi4 DSP 设计，HiFi4 DSP 用于音频、振动和信号预处理，不包装成通用 MCU。

## 硬件平台

- CPU：双核 ARM Cortex-A7
- DSP：单核 HiFi4 DSP
- 视频：H.265/H.264 硬解码，JPEG/MJPEG 硬件编码能力需结合 SDK 接口确认
- 显示：RGB、LVDS、MIPI DSI、CVBS，实际使用需结合原理图和 SDK 确认
- 通信：USB2.0、EMAC、UART、SPI、TWI/I2C、GPIO、PWM
- 音频：Codec、MICIN3、I2S/PCM、DMIC，具体通路需结合原理图确认

原理图线索：

- Base V1.3 图纸中存在 “CAMERA 接口或者 RMII 网络接口二选一” 的 DVP/RMII 复用说明。
- CTP_RST / CTP_INT 与 CAN0_TX / CAN0_RX、USER KEY 等存在复用线索，启用前必须按原理图确认跳线和功能选择。
- USB0/USB1、USB HUB/4G 模块、MICIN3、HPOUT、TVIN/TVOUT 等信号在图纸中可见。

## 软件依赖

- Tina SDK / OpenWrt 风格构建系统
- Linux 5.4
- C++11
- sqlite3
- pthread
- LVGL：作为 HMI 选型，当前代码保留集成点，实际启用需加入 Tina LVGL 包或 SDK GUI 包
- MQTT：当前为抽象骨架，量产时建议接入 mosquitto 或 paho-mqtt-c

## 编译方法

```sh
source build/envsetup.sh
lunch t113_100ask-tina
make menuconfig
```

在 `Utilities` 中选中 `edge-gateway` 后编译：

```sh
make package/edge-gateway/compile V=s
make -j$(nproc)
pack
```

## 烧录方法

使用当前 SDK 支持的 PhoenixSuit、LiveSuit、Tina pack 输出镜像或板卡既有烧录流程。烧录前确认 `device/config/chips/t113/configs/100ask/sys_partition.fex` 与存储介质匹配。

## 运行方法

```sh
/etc/init.d/edge-gateway enable
/etc/init.d/edge-gateway start
```

MJPEG 预览：

```text
http://<board-ip>:8080/
```

## 模块说明

- `app/camera`：V4L2 摄像头采集、MJPEG HTTP 推流
- `app/modbus`：UART/RS485 Modbus-RTU
- `app/network`：MQTT 抽象客户端
- `app/storage`：SQLite 断网缓存
- `app/audio_dsp`：Linux 侧 HiFi4 DSP 特征数据 IPC 抽象
- `app/watchdog`：硬件 Watchdog 守护
- `app/hmi`：LVGL 风格 HMI 主程序框架

## 测试方法

详见 `09_test_plan.md`。分阶段开发路线见 `10_development_plan.md`。建议先做串口、网络、USB 摄像头、LCD、GPIO、Watchdog，再做 Modbus、MQTT、SQLite、DSP 模拟特征和 OTA 回滚测试。

## 常见问题

- 没有 `/dev/video0`：检查 USB Host、UVC 驱动、摄像头供电、`dmesg`。
- LCD 花屏：检查 RGB/LVDS/MIPI 模式、时序、pinctrl、电源、背光 PWM。
- MQTT 无法连接：先确认 `ping`、路由、broker 地址、端口、防火墙。
- RS485 丢包：检查波特率、终端电阻、DE/RE 时序、收发方向 GPIO。

## 后续计划

- 接入真实 LVGL UI
- 接入真实 MQTT 库
- 接入 SDK JPEG/MJPEG 硬编码接口
- 接入 HiFi4 DSP vendor IPC
- 完成 A/B OTA 与 OverlayFS 实机验证
