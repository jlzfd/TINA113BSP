#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

namespace edge {

class FrameQueue {
public:
    void push(std::vector<uint8_t> jpeg);
    bool popLatest(std::vector<uint8_t> &jpeg, int timeout_ms);
    void stop();

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::deque<std::vector<uint8_t>> queue_;
    bool stopped_ = false;
    const size_t max_depth_ = 4;
};

class MjpegServer {
public:
    explicit MjpegServer(FrameQueue &queue);
    ~MjpegServer();

    bool start(uint16_t port);
    void stop();

private:
    void acceptLoop();
    void clientLoop(int client_fd);

    FrameQueue &queue_;
    int listen_fd_ = -1;
    std::atomic<bool> running_ {false};
    std::thread accept_thread_;
};

} // namespace edge
