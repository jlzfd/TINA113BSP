#include "modbus/modbus_rtu.h"

#include "common/log.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <unistd.h>

namespace edge {

bool ModbusRtu::openPort(const std::string &dev, int baudrate, int rs485_de_gpio)
{
    fd_ = ::open(dev.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd_ < 0) {
        EG_ERROR("open modbus port %s failed: %s", dev.c_str(), std::strerror(errno));
        return false;
    }
    de_gpio_ = rs485_de_gpio;
    if (!setBaudrate(baudrate)) return false;
    EG_INFO("modbus rtu opened: %s baud=%d de_gpio=%d", dev.c_str(), baudrate, de_gpio_);
    return true;
}

void ModbusRtu::closePort()
{
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

bool ModbusRtu::readHoldingRegisters(uint8_t slave, uint16_t addr, uint16_t count,
                                     std::vector<uint16_t> &out_regs)
{
    uint8_t req[8] = {
        slave, 0x03,
        static_cast<uint8_t>(addr >> 8), static_cast<uint8_t>(addr),
        static_cast<uint8_t>(count >> 8), static_cast<uint8_t>(count),
        0, 0
    };
    uint16_t crc = crc16(req, 6);
    req[6] = static_cast<uint8_t>(crc);
    req[7] = static_cast<uint8_t>(crc >> 8);

    for (int attempt = 0; attempt < retry_; ++attempt) {
        setDirection(true);
        if (!writeAll(req, sizeof(req))) continue;
        ::tcdrain(fd_);
        setDirection(false);

        std::vector<uint8_t> resp;
        if (!readResponse(resp, 500)) {
            EG_WARN("modbus timeout attempt=%d", attempt + 1);
            continue;
        }
        if (resp.size() < 5 || resp[0] != slave || resp[1] != 0x03) continue;
        uint16_t got_crc = resp[resp.size() - 2] | (resp[resp.size() - 1] << 8);
        if (crc16(resp.data(), resp.size() - 2) != got_crc) {
            EG_WARN("modbus crc error");
            continue;
        }

        out_regs.clear();
        for (size_t i = 3; i + 1 < resp.size() - 2; i += 2) {
            out_regs.push_back((resp[i] << 8) | resp[i + 1]);
        }
        return true;
    }
    return false;
}

uint16_t ModbusRtu::crc16(const uint8_t *data, size_t len)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int b = 0; b < 8; ++b) {
            crc = (crc & 1) ? (crc >> 1) ^ 0xA001 : (crc >> 1);
        }
    }
    return crc;
}

bool ModbusRtu::setBaudrate(int baudrate)
{
    termios tio {};
    if (::tcgetattr(fd_, &tio) < 0) return false;
    ::cfmakeraw(&tio);
    speed_t spd = baudrate == 115200 ? B115200 : baudrate == 19200 ? B19200 : B9600;
    ::cfsetispeed(&tio, spd);
    ::cfsetospeed(&tio, spd);
    tio.c_cflag |= CLOCAL | CREAD;
    tio.c_cflag &= ~CSTOPB;
    tio.c_cflag &= ~PARENB;
    return ::tcsetattr(fd_, TCSANOW, &tio) == 0;
}

bool ModbusRtu::setDirection(bool tx)
{
    // de_gpio_ < 0 表示当前板级配置暂未接入 GPIO 控制，后续可替换为 sysfs GPIO 或 libgpiod。
    (void)tx;
    return true;
}

bool ModbusRtu::writeAll(const uint8_t *data, size_t len)
{
    size_t off = 0;
    while (off < len) {
        ssize_t n = ::write(fd_, data + off, len - off);
        if (n < 0 && errno == EAGAIN) continue;
        if (n <= 0) return false;
        off += static_cast<size_t>(n);
    }
    return true;
}

bool ModbusRtu::readResponse(std::vector<uint8_t> &resp, int timeout_ms)
{
    pollfd pfd {fd_, POLLIN, 0};
    uint8_t buf[256];
    int ret = ::poll(&pfd, 1, timeout_ms);
    if (ret <= 0) return false;
    ssize_t n = ::read(fd_, buf, sizeof(buf));
    if (n <= 0) return false;
    resp.assign(buf, buf + n);
    return true;
}

} // namespace edge
