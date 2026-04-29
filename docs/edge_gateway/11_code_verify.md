# 11 代码编译与运行验证

## 当前本机验证结论

当前 Windows/PowerShell 环境没有 `make`、`gcc`、`g++`，因此不能在本机直接编译 `edge-gateway`。已经完成的本机检查包括：

- 文件结构完整性检查。
- Tina package Makefile 基础检查。
- 修复 `package.mk` 引入路径为 `$(INCLUDE_DIR)/package.mk`。
- 增加 `--self-test` 运行模式，便于上板后先验证基础模块。

## 在 Ubuntu/Tina SDK 中编译

```sh
cd ~/tina-d1-h
source build/envsetup.sh
lunch t113_100ask-tina
make package/edge-gateway/clean V=s
make package/edge-gateway/compile V=s
```

如果提示找不到 `libsqlite3`，进入 `make menuconfig`，确认 SQLite 库已选中，或者检查 Tina 包名是否为当前 SDK 使用的名称。

## 集成到 RootFS

```sh
make menuconfig
```

选择：

```text
Utilities -> edge-gateway
Libraries -> database -> sqlite
```

然后：

```sh
make -j$(nproc)
pack
```

## 上板 smoke test

烧录或拷贝二进制到板子后，先不要直接启动完整业务，先跑：

```sh
/usr/bin/edge_gateway --self-test
```

预期日志：

```text
edge_gateway self-test starting
HiFi4 DSP IPC placeholder opened
MQTT connected placeholder: 127.0.0.1:1883 client_id=self-test
MQTT publish topic=edge/sensor payload={"self_test":true}
edge_gateway self-test passed
```

该模式会验证：

- Modbus CRC16。
- SQLite 建库、插入、查询、标记上传。
- DSP IPC 模拟特征。
- MQTT 抽象接口。

## 完整业务运行

```sh
/etc/init.d/edge-gateway enable
/etc/init.d/edge-gateway start
logread -f
```

手动运行：

```sh
/usr/bin/edge_gateway /etc/edge-gateway/edge-gateway.conf
```

访问 MJPEG：

```text
http://<board-ip>:8080/
```

## 预期硬件依赖

完整业务会尝试访问：

- `/dev/video0`
- `/dev/watchdog`
- `/data/edge_gateway/cache.db`
- `192.168.1.100:1883` MQTT broker 占位配置

其中摄像头和 watchdog 打不开时，当前程序会记录错误并尽量继续运行；实际量产时应按产品策略决定是否退出、降级或重启。

## 常见编译问题

1. `sqlite3.h: No such file or directory`

   说明 SQLite 开发头未进入交叉编译 sysroot。检查 Tina 的 sqlite 包是否选中。

2. `undefined reference to sqlite3_*`

   检查 `LIBS="-lpthread -lsqlite3"` 是否传入，或 SDK 中库名是否不同。

3. `linux/videodev2.h` 找不到

   检查 kernel headers 或 libc headers 是否完整。

4. `cfmakeraw` 未声明

   可能是 libc 特性宏问题，可在 `CXXFLAGS` 增加 `-D_DEFAULT_SOURCE`。
