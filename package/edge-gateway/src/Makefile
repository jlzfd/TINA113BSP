CXX ?= g++
CXXFLAGS ?= -D_DEFAULT_SOURCE -std=c++11 -Wall -Wextra -I./app
LDFLAGS ?=
LIBS ?= -lpthread -lsqlite3

SRCS := \
	app/camera/v4l2_capture.cpp \
	app/camera/mjpeg_server.cpp \
	app/modbus/modbus_rtu.cpp \
	app/network/mqtt_client.cpp \
	app/storage/sqlite_cache.cpp \
	app/audio_dsp/dsp_ipc.cpp \
	app/watchdog/watchdog_daemon.cpp \
	app/hmi/main.cpp

OBJS := $(SRCS:.cpp=.o)

all: edge_gateway

edge_gateway: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS) edge_gateway
