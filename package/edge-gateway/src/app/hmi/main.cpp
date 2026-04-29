#include "audio_dsp/dsp_ipc.h"
#include "camera/mjpeg_server.h"
#include "camera/v4l2_capture.h"
#include "common/log.h"
#include "modbus/modbus_rtu.h"
#include "network/mqtt_client.h"
#include "storage/sqlite_cache.h"
#include "watchdog/watchdog_daemon.h"

#include <chrono>
#include <cstdio>
#include <csignal>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace edge;

static bool g_running = true;

static void onSignal(int)
{
    g_running = false;
}

static int runSelfTest()
{
    EG_INFO("edge_gateway self-test starting");

    const uint8_t req[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x0A};
    uint16_t crc = ModbusRtu::crc16(req, sizeof(req));
    if (crc != 0xCDC5) {
        EG_ERROR("modbus crc self-test failed: got=0x%04x expected=0xcdc5", crc);
        return 1;
    }

    ::unlink("/tmp/edge_gateway_selftest.db");
    SQLiteCache cache;
    if (!cache.open("/tmp/edge_gateway_selftest.db") ||
        !cache.insertSensorData("{\"temperature\":25.0}")) {
        EG_ERROR("sqlite self-test failed");
        return 1;
    }
    std::vector<CacheRecord> records;
    if (!cache.queryPending(records, 8) || records.empty()) {
        EG_ERROR("sqlite query self-test failed");
        return 1;
    }
    cache.markUploaded(records.front().id);
    cache.close();

    DspIpc dsp;
    dsp_audio_feature_t feature {};
    if (!dsp.open() || !dsp.readFeature(feature) || feature.seq == 0) {
        EG_ERROR("dsp ipc placeholder self-test failed");
        return 1;
    }

    MqttClient mqtt;
    mqtt.connect("127.0.0.1", 1883, "self-test");
    mqtt.publishSensorData("{\"self_test\":true}");
    mqtt.disconnect();

    EG_INFO("edge_gateway self-test passed");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc > 1 && std::string(argv[1]) == "--self-test") {
        return runSelfTest();
    }

    std::signal(SIGINT, onSignal);
    std::signal(SIGTERM, onSignal);

    EG_INFO("T113Pro edge gateway starting");

    V4L2Capture camera;
    FrameQueue frame_queue;
    MjpegServer mjpeg(frame_queue);
    ModbusRtu modbus;
    MqttClient mqtt;
    SQLiteCache cache;
    DspIpc dsp;
    WatchdogDaemon watchdog;

    ::mkdir("/data", 0755);
    ::mkdir("/data/edge_gateway", 0755);
    cache.open("/data/edge_gateway/cache.db");
    mqtt.connect("192.168.1.100", 1883, "t113pro-edge-gateway");
    dsp.open();
    watchdog.start("/dev/watchdog", 30);
    mjpeg.start(8080);

    if (camera.openDevice("/dev/video0") &&
        camera.queryCapability() &&
        camera.setFormat(800, 600, PixelFormat::MJPEG) &&
        camera.requestMmapBuffers(4)) {
        camera.streamOn();
    } else {
        EG_WARN("camera init failed, HMI will continue without preview");
    }

    // LVGL 集成点：这里初始化 fbdev/evdev、创建主界面、刷新状态标签和图像控件。
    while (g_running) {
        VideoFrame frame;
        if (camera.dequeueFrame(frame, 30) && frame.format == PixelFormat::MJPEG) {
            frame_queue.push(frame.data);
        }

        dsp_audio_feature_t feature {};
        if (dsp.readFeature(feature)) {
            char json[256];
            std::snprintf(json, sizeof(json),
                          "{\"seq\":%u,\"rms\":%.3f,\"peak\":%.3f,\"main_freq\":%.1f,\"alarm\":%u}",
                          feature.seq, feature.rms, feature.peak, feature.main_freq, feature.alarm_flags);
            mqtt.publishDspFeature(json);
            if (feature.alarm_flags) cache.insertAlarmEvent(json);
        }

        mqtt.loopOnce();
        watchdog.heartbeat();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    camera.closeDevice();
    mjpeg.stop();
    watchdog.stop();
    dsp.close();
    cache.close();
    EG_INFO("T113Pro edge gateway stopped");
    return 0;
}
