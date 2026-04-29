#include "watchdog/watchdog_daemon.h"

#include "common/log.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <linux/watchdog.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace edge {

WatchdogDaemon::~WatchdogDaemon()
{
    stop();
}

bool WatchdogDaemon::start(const std::string &dev, int timeout_sec)
{
    timeout_sec_ = timeout_sec;
    fd_ = ::open(dev.c_str(), O_WRONLY);
    if (fd_ < 0) {
        EG_ERROR("open watchdog %s failed: %s", dev.c_str(), std::strerror(errno));
        return false;
    }
    ::ioctl(fd_, WDIOC_SETTIMEOUT, &timeout_sec_);
    last_heartbeat_ = std::chrono::steady_clock::now();
    running_ = true;
    thread_ = std::thread(&WatchdogDaemon::loop, this);
    EG_INFO("watchdog daemon started timeout=%d", timeout_sec_);
    return true;
}

void WatchdogDaemon::heartbeat()
{
    last_heartbeat_ = std::chrono::steady_clock::now();
}

void WatchdogDaemon::stop()
{
    running_ = false;
    if (thread_.joinable()) thread_.join();
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

void WatchdogDaemon::loop()
{
    while (running_) {
        auto age = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - last_heartbeat_).count();
        if (age < timeout_sec_ / 2) {
            kick();
        } else {
            EG_ERROR("main service heartbeat timeout, stop feeding watchdog");
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

bool WatchdogDaemon::kick()
{
    int dummy = 0;
    return ::ioctl(fd_, WDIOC_KEEPALIVE, &dummy) == 0;
}

} // namespace edge
