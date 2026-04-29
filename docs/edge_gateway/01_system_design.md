# 01 系统总体设计

## 项目背景

本项目面向工业现场边缘网关场景，在 T113Pro 开发板上实现 Linux BSP 定制、现场总线采集、摄像头监控、本地 HMI、MQTT 上云、SQLite 断网缓存、HiFi4 DSP 音频/振动特征处理和可靠 OTA。

## 项目目标

1. 完成 U-Boot、Kernel、Device Tree、RootFS 定制。
2. 适配 LCD、触摸、PWM 背光、USB Host、EMAC、UART/RS485、SPI、TWI/I2C、GPIO、Watchdog。
3. 基于 V4L2 实现 USB 摄像头采集、本地预览、JPEG 抓拍、MJPEG 推流。
4. 预留 JPEG/MJPEG 硬件编码接口，H.264/H.265 仅按硬解码能力规划远程流显示扩展。
5. 使用 Modbus-RTU 采集工业传感器，MQTT 上云，SQLite 断网缓存。
6. 使用 HiFi4 DSP 做音频/振动 RMS、Peak、主频、频段能量和异常事件预处理。
7. 实现 A/B OTA、OverlayFS 只读根文件系统、硬件 Watchdog 和用户态健康检查。

## 硬件资源

- Cortex-A7 双核运行 Linux 和应用。
- HiFi4 DSP 运行信号处理任务，不作为通用实时控制 MCU 使用。
- USB 摄像头链路基于 UVC/V4L2，当前项目 `package/camera` 已有跑通经验。
- 原理图显示 DVP CAMERA 与 RMII 网络存在二选一复用，若使用 USB 摄像头，可保留 EMAC 网络。
- CTP、CAN、KEY 相关引脚存在复用，启用前需结合原理图、跳线和 DTS 确认。

## 软件架构

```text
Application: LVGL HMI / status dashboard / preview / alarm
Middleware : V4L2 / MJPEG / Modbus / MQTT / SQLite / DSP IPC / Watchdog
RootFS     : Tina Linux / init / OverlayFS / data / log / OTA service
Kernel BSP : DTS / pinctrl / LCD / USB / EMAC / UART / I2C / GPIO / ALSA / V4L2
Bootloader : boot0/SPL / U-Boot / bootargs / A/B slot / rollback
Hardware   : T113Pro Cortex-A7 + HiFi4 DSP + peripherals
```

## 数据流

摄像头数据：`UVC Camera -> USB Host -> V4L2 mmap -> MJPEG Queue -> HTTP MJPEG / HMI Preview`

传感器数据：`RS485 Sensor -> UART -> Modbus RTU -> JSON -> MQTT -> Cloud`

断网缓存：`MQTT publish failed -> SQLite cache -> network recover -> retry upload`

DSP 特征：`Mic/振动传感器 -> HiFi4 DSP -> feature IPC -> Linux HMI/MQTT/Alarm`

## 控制流

1. init 启动 `edge-gateway`。
2. 应用初始化 camera、modbus、mqtt、sqlite、dsp ipc、watchdog。
3. 主循环刷新 HMI、读取摄像头帧、采集传感器、读取 DSP 特征。
4. 业务正常时向 watchdog daemon 上报心跳。
5. OTA 升级完成后由用户态标记启动成功。

## 启动流程

`boot0/SPL -> U-Boot -> 选择 active_slot -> 加载 kernel/dtb/rootfs -> init -> overlay mount -> network -> edge-gateway`

## 视频链路

- 当前优先 USB UVC 摄像头。
- 支持 YUYV 和 MJPEG 格式。
- MJPEG 摄像头可直接推流，YUYV 需要软件或硬件 JPEG 编码。
- JPEG/MJPEG 硬件编码接口需结合 Tina SDK 多媒体库确认。
- H.264/H.265 规划为硬解码显示扩展，不默认作为硬编码能力。

## 工业通信链路

`UART + RS485 transceiver -> ModbusRtu -> CRC16 -> timeout retry -> sensor json -> MQTT/SQLite`

RS485 DE/RE GPIO 需结合原理图确认。若内核启用 serial rs485 ioctl，可优先使用内核方向控制。

## DSP 音频/振动处理链路

`Audio/Vibration input -> HiFi4 window buffer -> filter -> RMS/Peak/FFT -> feature packet -> Linux IPC`

Linux 侧只消费特征值，不把 HiFi4 当通用 MCU 调度 GPIO 或总线。

## OTA 升级链路

`download package -> signature/hash verify -> write inactive slot -> set upgrade_available -> reboot -> U-Boot boot new slot -> user-space mark boot_success`

失败后由 U-Boot 根据 boot_count 回滚旧 slot。

## Watchdog 健康检查链路

业务进程周期性 heartbeat；watchdog daemon 只有在业务心跳正常时喂 `/dev/watchdog`。业务卡死、主循环阻塞、严重异常时停止喂狗，由硬件复位恢复。

## 可靠性设计

- SquashFS/只读 rootfs 降低异常断电损坏概率。
- `/data` 存放 SQLite、配置、OTA 包，使用 fsck 和写入策略保护。
- `/log` 限额轮转，避免写满存储。
- OTA A/B 回滚避免升级失败变砖。
- Watchdog 覆盖应用卡死、死循环和关键线程阻塞。
