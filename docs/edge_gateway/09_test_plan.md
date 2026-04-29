# 09 测试方案

## BSP 测试

- 启动日志：U-Boot、Kernel、init 无严重 error。
- 串口：console 可登录，波特率稳定。
- 网络：EMAC up/down、DHCP/static IP、ping、iperf。
- LCD：纯色、色条、图片显示，无花屏闪屏。
- 触摸：I2C 探测、中断、坐标校准。
- USB：U 盘、USB 摄像头枚举。
- GPIO：LED、KEY、输入输出电平。
- PWM：背光亮度 0-100%。
- Watchdog：停止喂狗后自动复位。

## 视频测试

```sh
v4l2-ctl -d /dev/video0 --all
v4l2-ctl -d /dev/video0 --list-formats-ext
v4l2-ctl -d /dev/video0 --stream-mmap --stream-count=300
top
```

测试项：

- 格式枚举：YUYV/MJPEG。
- 帧率：15/30 fps。
- CPU 占用。
- MJPEG 多客户端推流。
- 长时间推流 24h。

## 工业通信测试

- RS485 A/B 极性。
- 终端电阻。
- Modbus CRC 正确性。
- 超时重试。
- 多 slave 轮询。
- MQTT 断线重连。
- SQLite 断网缓存和补传。

## DSP 测试

- 模拟特征数据序号递增。
- RMS/Peak/主频显示。
- 阈值报警。
- MQTT 上报。
- Linux 侧 HMI 状态刷新。

## 可靠性测试

- 断电重启 100 次。
- OTA 成功升级。
- OTA 写入中断电。
- OTA 新系统启动失败回滚。
- 应用卡死 Watchdog 复位。
- rootfs 只读保护。
- `/data` 写满保护。
- 24h/72h 长稳运行。
