#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace edge {

class ModbusRtu {
public:
    bool openPort(const std::string &dev, int baudrate, int rs485_de_gpio);
    void closePort();

    bool readHoldingRegisters(uint8_t slave, uint16_t addr, uint16_t count,
                              std::vector<uint16_t> &out_regs);

    static uint16_t crc16(const uint8_t *data, size_t len);

private:
    bool setBaudrate(int baudrate);
    bool setDirection(bool tx);
    bool writeAll(const uint8_t *data, size_t len);
    bool readResponse(std::vector<uint8_t> &resp, int timeout_ms);

    int fd_ = -1;
    int de_gpio_ = -1;
    int retry_ = 3;
};

} // namespace edge
