#pragma once

#include <cstdint>
#include <string>

namespace edge {

struct DeviceStatus {
    std::string device_id;
    std::string ip;
    int uptime_sec = 0;
    int cpu_percent = 0;
};

class MqttClient {
public:
    bool connect(const std::string &host, uint16_t port, const std::string &client_id);
    void disconnect();
    bool loopOnce();

    bool publishJson(const std::string &topic, const std::string &json);
    bool publishDeviceStatus(const DeviceStatus &status);
    bool publishSensorData(const std::string &json);
    bool publishDspFeature(const std::string &json);
    bool publishAlarmEvent(const std::string &json);

private:
    bool connected_ = false;
    std::string host_;
    uint16_t port_ = 1883;
    std::string client_id_;
};

} // namespace edge
