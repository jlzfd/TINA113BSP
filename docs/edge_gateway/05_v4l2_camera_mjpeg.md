# 05 V4L2 摄像头与 MJPEG 推流

## 当前基础

当前工程中 `package/camera` 已有摄像头跑通代码，包含 V4L2、framebuffer、OpenCV、JPEG 队列等实验内容。本项目新增 `package/edge-gateway`，不破坏旧代码，而是抽象出工程化模块：

- `app/camera/v4l2_capture.*`
- `app/camera/mjpeg_server.*`

## 链路设计

```text
/dev/video0 -> VIDIOC_QUERYCAP -> VIDIOC_S_FMT -> mmap buffers
-> STREAMON -> DQBUF -> FrameQueue -> HTTP multipart MJPEG -> browser/HMI
```

## 格式策略

- MJPEG 摄像头：直接推流，CPU 占用低。
- YUYV 摄像头：需要转换为 RGB 供 HMI，或编码为 JPEG 后推流。
- JPEG/MJPEG 硬件编码：T113Pro 有相关能力，但具体库、节点和 sample 需结合 Tina SDK 确认。
- H.264/H.265：按硬解码能力规划远程流解码显示，不默认写 H.264 硬编码。

## 调试命令

```sh
dmesg | grep -iE "usb|uvc|video"
ls -l /dev/video*
v4l2-ctl -d /dev/video0 --all
v4l2-ctl -d /dev/video0 --list-formats-ext
v4l2-ctl -d /dev/video0 --stream-mmap --stream-count=100
```

## HMI 预览

LVGL 可通过以下方式显示：

- MJPEG 解码后更新 `lv_img`。
- YUYV 转 RGB565 后写入 framebuffer 或 LVGL image buffer。
- 双缓冲避免撕裂。

## 性能关注

- mmap 零拷贝采集。
- MJPEG 直接推流优先。
- 限制队列深度，避免网络慢客户端拖垮采集。
- 多客户端推流要避免每个客户端重复编码。
