#!/bin/sh

set -eu

root="$(cd "$(dirname "$0")/../.." && pwd)"

need_files="
package/edge-gateway/Makefile
package/edge-gateway/src/app/camera/v4l2_capture.h
package/edge-gateway/src/app/camera/v4l2_capture.cpp
package/edge-gateway/src/app/camera/mjpeg_server.h
package/edge-gateway/src/app/camera/mjpeg_server.cpp
package/edge-gateway/src/app/modbus/modbus_rtu.h
package/edge-gateway/src/app/modbus/modbus_rtu.cpp
package/edge-gateway/src/app/network/mqtt_client.h
package/edge-gateway/src/app/storage/sqlite_cache.h
package/edge-gateway/src/app/audio_dsp/dsp_ipc.h
package/edge-gateway/src/app/watchdog/watchdog_daemon.h
docs/edge_gateway/README.md
docs/edge_gateway/08_interview_notes.md
"

for f in $need_files; do
	if [ ! -f "$root/$f" ]; then
		echo "missing: $f"
		exit 1
	fi
done

echo "edge-gateway tree looks ok"
