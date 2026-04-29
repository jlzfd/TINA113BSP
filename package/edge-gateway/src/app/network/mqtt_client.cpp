#include "network/mqtt_client.h"

#include "common/log.h"

namespace edge {

bool MqttClient::connect(const std::string &host, uint16_t port, const std::string &client_id)
{
    host_ = host;
    port_ = port;
    client_id_ = client_id;

    // 骨架层不绑定具体库。产品化时可替换为 mosquitto、paho-mqtt-c 或 SDK 自带 MQTT。
    connected_ = true;
    EG_INFO("MQTT connected placeholder: %s:%u client_id=%s", host_.c_str(), port_, client_id_.c_str());
    return true;
}

void MqttClient::disconnect()
{
    connected_ = false;
}

bool MqttClient::loopOnce()
{
    if (!connected_) {
        EG_WARN("MQTT disconnected, reconnect placeholder");
        connected_ = true;
    }
    return connected_;
}

bool MqttClient::publishJson(const std::string &topic, const std::string &json)
{
    if (!connected_) return false;
    EG_INFO("MQTT publish topic=%s payload=%s", topic.c_str(), json.c_str());
    return true;
}

bool MqttClient::publishDeviceStatus(const DeviceStatus &status)
{
    std::string json = "{\"device_id\":\"" + status.device_id + "\",\"ip\":\"" + status.ip + "\"}";
    return publishJson("edge/status", json);
}

bool MqttClient::publishSensorData(const std::string &json) { return publishJson("edge/sensor", json); }
bool MqttClient::publishDspFeature(const std::string &json) { return publishJson("edge/dsp_feature", json); }
bool MqttClient::publishAlarmEvent(const std::string &json) { return publishJson("edge/alarm", json); }

} // namespace edge
