# 10 分阶段开发路线

## 阶段 1：基础 BSP Bring-up

目标：系统能启动，串口可交互，网络可用。

任务：

- SDK 编译：确认 `lunch t113_100ask-tina` 和 `.config`。
- U-Boot：确认 bootargs、console、rootfs 参数。
- Kernel：保留串口、网口、USB、V4L2、fbdev、watchdog、ALSA 基础配置。
- Device Tree：确认 UART console、SD/SPI NAND、EMAC、USB Host。
- RootFS：确认 BusyBox/Tina init、dropbear、网络工具。
- 串口 shell：验证登录和启动日志。
- EMAC：验证 DHCP/static IP、ping、iperf。

交付：

- 可启动镜像。
- Bring-up 日志。
- `02_bsp_bringup.md`。

## 阶段 2：显示与基础外设

目标：LCD、触摸、背光、GPIO、PWM 可用。

任务：

- RGB/LVDS/MIPI 选择一种真实屏。
- 适配 panel timing、pinctrl、电源、reset。
- 配置 PWM backlight。
- I2C touch 适配，确认 CTP_INT/CTP_RST 是否与 CAN/KEY/USB 复用。
- GPIO LED/KEY 验证。
- LVGL fbdev/evdev 骨架接入。

交付：

- 显示正常、背光可调。
- 触摸输入可读。
- LVGL HMI demo 可启动。

## 阶段 3：工业通信与边缘数据

目标：采集现场传感器数据并上传。

任务：

- UART + RS485 Modbus-RTU。
- DE/RE GPIO 或内核 RS485 自动方向。
- GPIO DI/DO。
- 可选 CAN/SocketCAN，前提是硬件或 SPI-CAN 确认。
- MQTT 上云。
- SQLite 断网缓存与重传。

交付：

- `modbus_rtu.*`
- `mqtt_client.*`
- `sqlite_cache.*`
- 断网补传测试记录。

## 阶段 4：视觉监控

目标：摄像头采集、本地预览、JPEG 抓拍、MJPEG 推流。

任务：

- USB Host 和 UVC Camera 验证。
- V4L2 mmap 采集。
- YUYV/MJPEG 格式处理。
- LCD/LVGL 本地预览。
- HTTP MJPEG Server。
- JPEG/MJPEG 硬件编码接口预留。

交付：

- `v4l2_capture.*`
- `mjpeg_server.*`
- 浏览器 MJPEG 预览。

## 阶段 5：DSP 音频/振动监测与企业可靠性

目标：形成企业级 BSP 亮点。

任务：

- HiFi4 DSP 音频/振动特征提取。
- Linux 与 DSP IPC 抽象。
- RMS/Peak/FFT/主频/频段能量。
- 异常报警。
- A/B OTA。
- OverlayFS 只读 RootFS。
- Watchdog daemon。
- 启动时间优化。
- 24h/72h 稳定性测试。

交付：

- `dsp_ipc.*`
- `watchdog_daemon.*`
- OTA/OverlayFS 设计文档。
- 面试材料和测试报告模板。
