#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace edge {

enum class PixelFormat {
    YUYV,
    MJPEG,
};

struct VideoFrame {
    std::vector<uint8_t> data;
    uint32_t width = 0;
    uint32_t height = 0;
    PixelFormat format = PixelFormat::MJPEG;
    uint64_t timestamp_us = 0;
};

class V4L2Capture {
public:
    V4L2Capture();
    ~V4L2Capture();

    bool openDevice(const std::string &device);
    bool queryCapability();
    bool setFormat(uint32_t width, uint32_t height, PixelFormat format);
    bool requestMmapBuffers(uint32_t count);
    bool streamOn();
    bool dequeueFrame(VideoFrame &frame, int timeout_ms);
    bool enqueueBuffer(uint32_t index);
    bool streamOff();
    void closeDevice();

private:
    struct Buffer {
        void *start = nullptr;
        size_t length = 0;
    };

    uint32_t toV4L2Format(PixelFormat format) const;
    bool xioctl(unsigned long request, void *arg);

    int fd_ = -1;
    std::string device_;
    uint32_t width_ = 0;
    uint32_t height_ = 0;
    PixelFormat format_ = PixelFormat::MJPEG;
    std::vector<Buffer> buffers_;
    bool streaming_ = false;
};

} // namespace edge
