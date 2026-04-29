# 06 Modbus、MQTT 与 SQLite

## Modbus-RTU

链路：

```text
RS485 sensor -> UART -> DE/RE direction -> Modbus request -> CRC16 -> response parse
```

实现点：

- 打开 `/dev/ttySx`。
- 配置 9600/19200/115200，8N1。
- 发送前拉高 DE，发送完成 `tcdrain()` 后切回接收。
- 接收超时后重试。
- CRC16 使用 Modbus 标准多项式 `0xA001`。

RS485 DE/RE GPIO 需结合原理图确认；如果内核串口驱动支持 RS485 自动方向控制，优先使用内核。

## MQTT

主题建议：

```text
edge/{device_id}/status
edge/{device_id}/sensor
edge/{device_id}/dsp_feature
edge/{device_id}/alarm
edge/{device_id}/ota/status
```

策略：

- 网络断开后指数退避重连。
- 发布失败的数据写入 SQLite。
- 状态主题可设置 retained。
- 设备证书、token 放 `/data/edge_gateway/certs`。

## SQLite

表：

```sql
CREATE TABLE cache_records (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    topic TEXT NOT NULL,
    payload TEXT NOT NULL,
    uploaded INTEGER DEFAULT 0,
    created_at INTEGER DEFAULT (strftime('%s','now'))
);
```

补传流程：

1. MQTT 在线。
2. 查询 `uploaded=0`。
3. 按 id 顺序发布。
4. 成功后标记 uploaded。
5. 失败则停止本轮补传，避免乱序。

## 数据 JSON 示例

```json
{
  "device_id": "t113pro-gw-001",
  "ts": 1710000000,
  "slave": 1,
  "temperature": 26.5,
  "humidity": 60.2
}
```
