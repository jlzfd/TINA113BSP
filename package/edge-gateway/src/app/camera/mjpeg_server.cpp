#include "camera/mjpeg_server.h"

#include "common/log.h"

#include <arpa/inet.h>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <utility>
#include <unistd.h>

namespace edge {

void FrameQueue::push(std::vector<uint8_t> jpeg)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.size() >= max_depth_) queue_.pop_front();
    queue_.push_back(std::move(jpeg));
    cv_.notify_all();
}

bool FrameQueue::popLatest(std::vector<uint8_t> &jpeg, int timeout_ms)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (!cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms), [&] { return stopped_ || !queue_.empty(); })) {
        return false;
    }
    if (stopped_) return false;
    jpeg = std::move(queue_.back());
    queue_.clear();
    return true;
}

void FrameQueue::stop()
{
    std::lock_guard<std::mutex> lock(mutex_);
    stopped_ = true;
    cv_.notify_all();
}

MjpegServer::MjpegServer(FrameQueue &queue) : queue_(queue) {}

MjpegServer::~MjpegServer()
{
    stop();
}

bool MjpegServer::start(uint16_t port)
{
    listen_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0) {
        EG_ERROR("socket failed: %s", std::strerror(errno));
        return false;
    }

    int yes = 1;
    ::setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (::bind(listen_fd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0 ||
        ::listen(listen_fd_, 8) < 0) {
        EG_ERROR("bind/listen port %u failed: %s", port, std::strerror(errno));
        ::close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }

    running_ = true;
    accept_thread_ = std::thread(&MjpegServer::acceptLoop, this);
    EG_INFO("MJPEG server started at port %u", port);
    return true;
}

void MjpegServer::stop()
{
    running_ = false;
    queue_.stop();
    if (listen_fd_ >= 0) {
        ::shutdown(listen_fd_, SHUT_RDWR);
        ::close(listen_fd_);
        listen_fd_ = -1;
    }
    if (accept_thread_.joinable()) accept_thread_.join();
}

void MjpegServer::acceptLoop()
{
    while (running_) {
        int client = ::accept(listen_fd_, nullptr, nullptr);
        if (client < 0) {
            if (running_) EG_WARN("accept failed: %s", std::strerror(errno));
            continue;
        }
        std::thread(&MjpegServer::clientLoop, this, client).detach();
    }
}

void MjpegServer::clientLoop(int client_fd)
{
    const char *header =
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\n"
        "Cache-Control: no-cache\r\n"
        "Pragma: no-cache\r\n"
        "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
    ::send(client_fd, header, std::strlen(header), 0);

    while (running_) {
        std::vector<uint8_t> jpeg;
        if (!queue_.popLatest(jpeg, 1000)) continue;

        char part[160];
        int n = std::snprintf(part, sizeof(part),
                              "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %zu\r\n\r\n",
                              jpeg.size());
        if (::send(client_fd, part, n, 0) <= 0) break;
        if (::send(client_fd, jpeg.data(), jpeg.size(), 0) <= 0) break;
        if (::send(client_fd, "\r\n", 2, 0) <= 0) break;
    }
    ::close(client_fd);
}

} // namespace edge
