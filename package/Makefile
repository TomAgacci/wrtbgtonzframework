include $(TOPDIR)/rules.mk

PKG_NAME:=morsebandd
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/morsebandd
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Multi-band timing-encoded Morse daemon
  DEPENDS:=+kmod-tun
endef

define Package/morsebandd/description
  morsebandd is a multi-band timing-encoded Morse-like transmission daemon
  featuring slope-based timing, zero-circle prepacking, FEC/majority vote,
  slope-delta prediction, and tun/tap virtual interface integration.
endef

define Build/Prepare
    mkdir -p $(PKG_BUILD_DIR)
    $(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
    $(MAKE) -C $(PKG_BUILD_DIR) \
        CC="$(TARGET_CC)" \
        CFLAGS="$(TARGET_CFLAGS)" \
        LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/morsebandd/install
    $(INSTALL_DIR) $(1)/usr/sbin
    $(INSTALL_BIN) $(PKG_BUILD_DIR)/morsebandd $(1)/usr/sbin/

    $(INSTALL_DIR) $(1)/etc
    $(INSTALL_CONF) ./files/morsebandd.conf $(1)/etc/morsebandd.conf

    $(INSTALL_DIR) $(1)/etc/init.d
    $(INSTALL_BIN) ./files/morsebandd.init $(1)/etc/init.d/morsebandd
endef

$(eval $(call BuildPackage,morsebandd))
