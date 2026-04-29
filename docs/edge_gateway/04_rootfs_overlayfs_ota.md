# 04 OverlayFS 与 A/B OTA 设计

## 为什么工业设备需要只读 RootFS

工业现场常见异常断电、电源抖动、强干扰和长时间无人值守。只读 rootfs 可以降低系统文件损坏概率，避免日志、数据库或临时文件写入根分区导致启动失败。

推荐：

- `rootfs_a` / `rootfs_b`：SquashFS，只读。
- `overlay_a` / `overlay_b`：ext4/ubifs，保存少量配置覆盖。
- `data`：ext4/ubifs，保存 SQLite、日志、OTA 包、业务数据。
- `misc/env`：保存 slot、boot_count、boot_success 等状态。

## 分区设计

```text
boot0/spl
uboot
env
boot_a
rootfs_a
overlay_a
boot_b
rootfs_b
overlay_b
data
log
recovery
```

实际名称和大小需结合 `sys_partition.fex`、存储介质容量和 Tina pack 工具确认。

## U-Boot 环境变量

```text
active_slot=A
boot_count=0
boot_limit=3
boot_success=1
upgrade_available=0
rootfs_a=/dev/by-name/rootfs_a
rootfs_b=/dev/by-name/rootfs_b
```

## U-Boot 回滚伪代码

```c
slot = env_get("active_slot");
upgrade = env_get("upgrade_available");
success = env_get("boot_success");
count = env_get("boot_count");

if (upgrade == 1 && success == 0) {
    count++;
    env_set("boot_count", count);
    if (count > boot_limit) {
        slot = slot == "A" ? "B" : "A";
        env_set("active_slot", slot);
        env_set("upgrade_available", 0);
        env_set("boot_count", 0);
    }
}

boot_kernel(slot);
```

## Linux 用户态升级流程

1. 下载升级包到 `/data/ota/update.swu`。
2. 校验包头、版本、硬件型号、hash、签名。
3. 判断当前 active slot，选择 inactive slot。
4. 写入 inactive boot/rootfs。
5. 设置 `active_slot=inactive`、`upgrade_available=1`、`boot_success=0`、`boot_count=0`。
6. 重启。
7. 新系统启动后，业务健康检查通过，再标记 `boot_success=1`。

## 升级包校验流程

```text
package -> sha256 -> signature verify -> hwrev/product verify -> version policy -> write inactive slot
```

禁止未签名包、硬件型号不匹配包、低版本回滚包直接升级，除非进入工程维护模式。

## 启动成功标记

启动成功不应由 init 一启动就标记，而应满足：

- rootfs 挂载正常。
- 网络服务正常。
- 主业务进程启动成功。
- Watchdog daemon 正常。
- 关键外设至少完成基础探测。

满足后执行：

```sh
fw_setenv boot_success 1
fw_setenv upgrade_available 0
fw_setenv boot_count 0
```

## OverlayFS init 示例

```sh
mount -o ro /dev/by-name/rootfs_${active_slot} /rom
mount /dev/by-name/overlay_${active_slot} /overlay
mkdir -p /overlay/upper /overlay/work /newroot
mount -t overlay overlay -o lowerdir=/rom,upperdir=/overlay/upper,workdir=/overlay/work /newroot
mkdir -p /data /log
mount /dev/by-name/data /data
mount /dev/by-name/log /log
```

## `/etc`、`/var`、`/log`、`/data`

- `/etc`：默认来自只读 rootfs，少量设备配置通过 overlay 覆盖。
- `/var`：tmpfs，避免频繁写 flash。
- `/log`：独立分区，限额轮转。
- `/data`：SQLite、采集缓存、OTA 包、设备证书。

## 异常断电保护

- 根文件系统只读。
- SQLite 使用 WAL 或事务。
- 日志异步写、限额轮转。
- OTA 只写 inactive slot。
- 升级状态变量写入顺序必须可恢复。
