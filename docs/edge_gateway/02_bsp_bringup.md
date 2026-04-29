# 02 BSP Bring-up 路线

## 当前 SDK 状态

当前工作区为 Tina SDK，目标配置为 `CONFIG_TARGET_t113_100ask=y`，Kernel 为 Linux 5.4，RootFS 当前启用 SquashFS。

关键路径：

- `device/config/chips/t113/configs/100ask/linux-5.4/board.dts`
- `device/config/chips/t113/configs/100ask/uboot-board.dts`
- `target/allwinner/t113-100ask/BoardConfig.mk`
- `lichee/linux-5.4`
- `lichee/brandy-2.0`

## 阶段 1：基础 BSP Bring-up

目标：系统能启动，串口可交互，网络可用。

任务：

1. `source build/envsetup.sh`
2. `lunch t113_100ask-tina`
3. 编译 SDK：`make -j$(nproc)`
4. 打包：`pack`
5. 串口确认 U-Boot、Kernel、RootFS 日志。
6. 确认 bootargs 中 console 与 DTS UART 一致。
7. 确认 EMAC pinctrl、PHY reset、电源、RMII/RGMII 模式。

验收：

- 串口进入 shell。
- `ifconfig eth0 up` 可用。
- `ping` 网关成功。

## 阶段 2：显示与基础外设

目标：LCD、触摸、背光、GPIO、PWM 可用。

任务：

- 选择 RGB/LVDS/MIPI 中一种真实屏。
- 适配 panel timing、pinctrl、电源、reset、enable。
- 配置 PWM backlight。
- 触摸使用 I2C CTP 时确认 INT/RST GPIO，图纸中 CTP 与 CAN/KEY 有复用线索。
- GPIO LED/KEY 验证。
- LVGL fbdev/evdev 验证。

## 阶段 3：工业通信与边缘数据

目标：采集传感器并上传。

任务：

- 选择 UART 作为 RS485。
- 确认 RS485 DE/RE GPIO 或内核 RS485 自动方向控制。
- 实现 Modbus-RTU CRC、超时、重试。
- MQTT 断线重连。
- SQLite 断网缓存和补传。

## 阶段 4：视觉监控

目标：摄像头采集、本地预览、抓拍、推流。

任务：

- USB Host 和 UVC 驱动验证。
- `v4l2-ctl --list-devices`
- `v4l2-ctl --list-formats-ext -d /dev/video0`
- V4L2 mmap 采集。
- MJPEG HTTP server。
- YUYV 到 JPEG 的软件或硬件编码路径。

## 阶段 5：DSP 与可靠性

目标：企业级亮点。

任务：

- DSP 模拟特征接入 HMI/MQTT。
- 后续替换为 HiFi4 vendor IPC。
- A/B OTA 分区和回滚逻辑。
- OverlayFS 只读 rootfs。
- Watchdog daemon。
- 24h/72h 稳定性测试。
