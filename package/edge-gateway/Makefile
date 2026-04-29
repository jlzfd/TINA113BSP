include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/package.mk

PKG_NAME:=edge-gateway
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=T113Pro industrial edge gateway demo
  DEPENDS:=+libpthread +librt +libsqlite3
endef

define Package/$(PKG_NAME)/description
  Industrial multi-protocol edge gateway skeleton for Allwinner T113Pro.
  It contains V4L2 camera, MJPEG server, Modbus-RTU, MQTT abstraction,
  SQLite cache, HiFi4 DSP feature IPC abstraction, watchdog daemon and
  LVGL-style HMI entry points.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CXX="$(TARGET_CXX)" \
		CC="$(TARGET_CC)" \
		CXXFLAGS="$(TARGET_CXXFLAGS) -D_DEFAULT_SOURCE -std=c++11 -Wall -Wextra -I./app" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		LIBS="-lpthread -lsqlite3"
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/edge_gateway $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/edge-gateway.init $(1)/etc/init.d/edge-gateway
	$(INSTALL_DIR) $(1)/etc/edge-gateway
	$(INSTALL_CONF) ./files/edge-gateway.conf $(1)/etc/edge-gateway/edge-gateway.conf
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
