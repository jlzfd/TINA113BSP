# 08 面试讲解材料

## 1. 项目一句话介绍

这是一个基于全志 T113Pro 的工业边缘网关 BSP 项目，完成从 U-Boot、Kernel、Device Tree、RootFS 到摄像头、RS485、MQTT、SQLite、DSP 音频特征、OTA 和 Watchdog 的端到端系统设计。

## 2. 项目架构讲解

底层是 boot0/SPL 和 U-Boot，中间是 Linux 5.4 BSP 和设备树，上层是 Tina RootFS、OverlayFS、OTA 服务，再往上是 V4L2、Modbus、MQTT、SQLite、DSP IPC、Watchdog，最终由 LVGL HMI 展示设备状态、视频、传感器和报警。

## 3. 为什么这是 BSP 项目而不是普通应用项目

项目重点不只是写业务程序，而是要完成启动链、内核配置、设备树、外设 bring-up、RootFS、驱动与应用联调、可靠升级和系统稳定性设计。

## 4. U-Boot 做了什么

U-Boot 负责加载 kernel/dtb/rootfs，传递 bootargs，选择 A/B slot，维护 boot_count、boot_success、upgrade_available，并在升级失败时回滚。

## 5. 设备树适配了哪些外设

UART/RS485、USB Host、EMAC、RGB/LVDS LCD、PWM 背光、I2C touch、GPIO LED/KEY、Watchdog、Codec/DMIC/I2S 等。实际启用要结合原理图确认 pinctrl 和复用冲突。

## 6. LCD 花屏怎么排查

先确认接口模式 RGB/LVDS/MIPI，再查电源、reset、背光、pinctrl、时钟、像素格式、hsync/vsync/de 极性和屏时序；最后用示波器看 pclk、de、hsync、vsync。

## 7. 触摸无响应怎么排查

查 I2C 地址、INT/RST GPIO、电源、pinctrl、中断触发沿、驱动 compatible；用 `i2cdetect` 和 `dmesg` 验证。该板 CTP 与 CAN/KEY/USB 可能有复用，必须看原理图和跳线。

## 8. USB 摄像头没有 `/dev/video0` 怎么排查

查 USB VBUS、EHCI/OHCI、PHY、UVC 驱动、摄像头枚举日志；用 `dmesg`、`lsusb`、`v4l2-ctl --list-devices` 定位。

## 9. RS485 通信丢包怎么排查

查 A/B 线、终端电阻、GND、波特率、校验位、DE/RE 时序、收发切换延时、总线冲突、Modbus CRC 和超时重试。

## 10. MQTT 断线怎么处理

客户端做断线检测和指数退避重连，发布失败写 SQLite，网络恢复后按 id 顺序补传，状态主题可 retained。

## 11. 为什么用 OverlayFS

工业设备异常断电多，只读 rootfs 能避免系统文件损坏。OverlayFS 只保存少量可变配置，业务数据放 `/data`。

## 12. A/B OTA 怎么回滚

升级写 inactive slot，设置 `upgrade_available=1` 和 `boot_success=0`。新系统启动后健康检查通过才标记成功；若多次启动失败，U-Boot 切回旧 slot。

## 13. Watchdog 怎么设计才可靠

不能简单定时喂狗，而是由业务主循环上报心跳，watchdog daemon 只有在关键线程健康时喂狗。业务卡死就停止喂狗，让硬件复位。

## 14. HiFi4 DSP 在项目中做什么

做音频/振动预处理，包括 RMS、Peak、FFT、主频、频段能量和异常声学事件检测，再把特征值传给 Linux。

## 15. 为什么不把 HiFi4 DSP 当成 MCU 用

HiFi4 是面向 DSP 信号处理的核，不是通用实时控制 MCU。GPIO、总线、网络、文件系统这类控制面更适合由 Cortex-A7 Linux 管理。

## 16. H.264/H.265 是硬解码还是硬编码

在本项目中按 T113Pro 的 H.264/H.265 硬解码能力规划，用于远程流解码显示扩展；不默认宣称 H.264 硬编码。

## 17. JPEG/MJPEG 硬编码如何利用

USB 摄像头如果输出 YUYV，可调用 SDK 的 JPEG/MJPEG 硬件编码接口转成 JPEG 帧，用于抓拍和 MJPEG 推流。具体 API 需结合 Tina SDK sample 确认。

## 18. 项目最大难点和解决方案

难点是 BSP 与应用联调：引脚复用、设备树、驱动、RootFS、自启动、异常断电、OTA 回滚都互相关联。解决方法是按阶段推进，每个外设建立可重复测试清单和回退点。

## 19. 简历项目描述

基于全志 T113Pro 开发工业多协议边缘网关 BSP，完成 U-Boot 启动参数、Linux 5.4 Kernel 裁剪、Device Tree 外设适配、RootFS/OverlayFS、USB UVC 摄像头 V4L2 采集、MJPEG 推流、RS485 Modbus、MQTT、SQLite 断网缓存、HiFi4 DSP 音频特征抽象、A/B OTA 和 Watchdog 可靠性设计。

## 20. 面试 3 分钟讲解稿

我做的是一个 T113Pro 工业边缘网关 BSP 项目，不是单纯应用 demo。底层我基于 Tina SDK 做启动链、Kernel、设备树和 RootFS 定制，适配了串口、网口、USB、LCD、PWM 背光、GPIO、Watchdog 等外设。应用层用 LVGL 做本地 HMI，用 V4L2 采集 USB 摄像头，实现本地预览和 MJPEG HTTP 推流；工业侧通过 UART/RS485 跑 Modbus-RTU 采集传感器，数据通过 MQTT 上云，断网时写 SQLite，恢复后补传。可靠性方面设计了只读 rootfs + OverlayFS、A/B OTA 回滚、硬件 Watchdog 和健康检查。T113Pro 的 HiFi4 DSP 我没有当成 MCU 用，而是用于音频和振动信号预处理，比如 RMS、Peak、FFT 主频和频段能量，再把特征值传给 Linux 做显示和报警。这个项目能体现 BSP 岗位需要的启动、驱动、设备树、RootFS、外设 bring-up 和系统可靠性能力。
