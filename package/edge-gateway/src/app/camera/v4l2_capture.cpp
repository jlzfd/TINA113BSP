#include "camera/v4l2_capture.h"

#include "common/log.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace edge {

V4L2Capture::V4L2Capture() = default;

V4L2Capture::~V4L2Capture()
{
    closeDevice();
}

bool V4L2Capture::openDevice(const std::string &device)
{
    device_ = device.empty() ? "/dev/video0" : device;
    fd_ = ::open(device_.c_str(), O_RDWR | O_NONBLOCK);
    if (fd_ < 0) {
        EG_ERROR("open %s failed: %s", device_.c_str(), std::strerror(errno));
        return false;
    }
    EG_INFO("camera device opened: %s", device_.c_str());
    return true;
}

bool V4L2Capture::queryCapability()
{
    v4l2_capability cap {};
    if (!xioctl(VIDIOC_QUERYCAP, &cap)) {
        EG_ERROR("VIDIOC_QUERYCAP failed");
        return false;
    }
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) || !(cap.capabilities & V4L2_CAP_STREAMING)) {
        EG_ERROR("device is not a streaming video capture device");
        return false;
    }
    EG_INFO("camera driver=%s card=%s bus=%s", cap.driver, cap.card, cap.bus_info);
    return true;
}

bool V4L2Capture::setFormat(uint32_t width, uint32_t height, PixelFormat format)
{
    v4l2_format fmt {};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = toV4L2Format(format);
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if (!xioctl(VIDIOC_S_FMT, &fmt)) {
        EG_ERROR("VIDIOC_S_FMT failed");
        return false;
    }

    width_ = fmt.fmt.pix.width;
    height_ = fmt.fmt.pix.height;
    format_ = format;
    EG_INFO("camera format set: %ux%u fourcc=0x%x", width_, height_, fmt.fmt.pix.pixelformat);
    return true;
}

bool V4L2Capture::requestMmapBuffers(uint32_t count)
{
    v4l2_requestbuffers req {};
    req.count = count;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (!xioctl(VIDIOC_REQBUFS, &req) || req.count < 2) {
        EG_ERROR("VIDIOC_REQBUFS failed or returned too few buffers");
        return false;
    }

    buffers_.resize(req.count);
    for (uint32_t i = 0; i < req.count; ++i) {
        v4l2_buffer buf {};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (!xioctl(VIDIOC_QUERYBUF, &buf)) {
            EG_ERROR("VIDIOC_QUERYBUF failed at index %u", i);
            return false;
        }

        buffers_[i].length = buf.length;
        buffers_[i].start = ::mmap(nullptr, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, buf.m.offset);
        if (buffers_[i].start == MAP_FAILED) {
            EG_ERROR("mmap buffer %u failed: %s", i, std::strerror(errno));
            buffers_[i].start = nullptr;
            return false;
        }

        if (!enqueueBuffer(i)) {
            return false;
        }
    }
    EG_INFO("camera mmap buffers ready: %u", req.count);
    return true;
}

bool V4L2Capture::streamOn()
{
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (!xioctl(VIDIOC_STREAMON, &type)) {
        EG_ERROR("VIDIOC_STREAMON failed");
        return false;
    }
    streaming_ = true;
    return true;
}

bool V4L2Capture::dequeueFrame(VideoFrame &frame, int timeout_ms)
{
    pollfd pfd {};
    pfd.fd = fd_;
    pfd.events = POLLIN;
    int ret = ::poll(&pfd, 1, timeout_ms);
    if (ret <= 0) {
        if (ret == 0) EG_WARN("camera dequeue timeout");
        else EG_ERROR("poll camera failed: %s", std::strerror(errno));
        return false;
    }

    v4l2_buffer buf {};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    if (!xioctl(VIDIOC_DQBUF, &buf)) {
        EG_ERROR("VIDIOC_DQBUF failed");
        return false;
    }

    frame.data.assign(static_cast<uint8_t *>(buffers_[buf.index].start),
                      static_cast<uint8_t *>(buffers_[buf.index].start) + buf.bytesused);
    frame.width = width_;
    frame.height = height_;
    frame.format = format_;
    frame.timestamp_us = static_cast<uint64_t>(buf.timestamp.tv_sec) * 1000000ULL + buf.timestamp.tv_usec;

    return enqueueBuffer(buf.index);
}

bool V4L2Capture::enqueueBuffer(uint32_t index)
{
    v4l2_buffer buf {};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = index;
    return xioctl(VIDIOC_QBUF, &buf);
}

bool V4L2Capture::streamOff()
{
    if (!streaming_) return true;
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bool ok = xioctl(VIDIOC_STREAMOFF, &type);
    streaming_ = false;
    return ok;
}

void V4L2Capture::closeDevice()
{
    streamOff();
    for (auto &b : buffers_) {
        if (b.start) {
            ::munmap(b.start, b.length);
        }
    }
    buffers_.clear();
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

uint32_t V4L2Capture::toV4L2Format(PixelFormat format) const
{
    return format == PixelFormat::YUYV ? V4L2_PIX_FMT_YUYV : V4L2_PIX_FMT_MJPEG;
}

bool V4L2Capture::xioctl(unsigned long request, void *arg)
{
    int ret;
    do {
        ret = ::ioctl(fd_, request, arg);
    } while (ret < 0 && errno == EINTR);
    if (ret < 0) {
        EG_ERROR("ioctl 0x%lx failed: %s", request, std::strerror(errno));
        return false;
    }
    return true;
}

} // namespace edge
