# T113Pro 工业多协议边缘网关 BSP 项目入口

本文件是新增 BSP 项目的入口说明，完整文档位于 `docs/edge_gateway/`，应用包位于 `package/edge-gateway/`。

## 快速入口

- 项目 README：`docs/edge_gateway/README.md`
- 总体设计：`docs/edge_gateway/01_system_design.md`
- BSP Bring-up：`docs/edge_gateway/02_bsp_bringup.md`
- Device Tree 示例：`docs/edge_gateway/03_device_tree_examples.md`
- OverlayFS 与 A/B OTA：`docs/edge_gateway/04_rootfs_overlayfs_ota.md`
- V4L2 与 MJPEG：`docs/edge_gateway/05_v4l2_camera_mjpeg.md`
- Modbus/MQTT/SQLite：`docs/edge_gateway/06_modbus_mqtt_sqlite.md`
- DSP 音频监测：`docs/edge_gateway/07_dsp_audio_monitor.md`
- 面试材料：`docs/edge_gateway/08_interview_notes.md`
- 测试方案：`docs/edge_gateway/09_test_plan.md`
- 分阶段路线：`docs/edge_gateway/10_development_plan.md`
- 编译验证：`docs/edge_gateway/11_code_verify.md`

## 新增应用包

```text
package/edge-gateway/
├── Makefile
├── files/
│   ├── edge-gateway.conf
│   └── edge-gateway.init
└── src/
    ├── Makefile
    └── app/
        ├── audio_dsp/
        ├── camera/
        ├── common/
        ├── hmi/
        ├── modbus/
        ├── network/
        ├── storage/
        └── watchdog/
```

## 编译方式

```sh
source build/envsetup.sh
lunch t113_100ask-tina
make menuconfig
make package/edge-gateway/compile V=s
```

在 `menuconfig` 中选择 `Utilities -> edge-gateway` 后，可进入整包编译和打包流程。

## 设计边界

- 当前不修改已有 `package/camera`，它作为摄像头已跑通参考。
- 当前不直接修改 DTS、U-Boot 环境和分区表，只提供文档和示例片段。
- HMI 选型按 LVGL。
- H.264/H.265 按硬解码能力描述；JPEG/MJPEG 按硬件编码能力预留接口，具体 SDK API 需后续确认。
- HiFi4 DSP 只作为音频/振动信号处理单元，不包装成 RISC-V 或通用 MCU。
