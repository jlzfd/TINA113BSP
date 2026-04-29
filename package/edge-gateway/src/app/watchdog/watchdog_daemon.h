#pragma once

#include <atomic>
#include <chrono>
#include <string>
#include <thread>

namespace edge {

class WatchdogDaemon {
public:
    ~WatchdogDaemon();

    bool start(const std::string &dev, int timeout_sec);
    void heartbeat();
    void stop();

private:
    void loop();
    bool kick();

    int fd_ = -1;
    int timeout_sec_ = 30;
    std::atomic<bool> running_ {false};
    std::thread thread_;
    std::chrono::steady_clock::time_point last_heartbeat_;
};

} // namespace edge
