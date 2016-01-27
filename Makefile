# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 4841 2006-09-23 19:28:18Z nico $

-include $(TOPDIR)/.config
include $(TOPDIR)/rules.mk

PKG_NAME:=dniutil
PKG_RELEASE:=1
PKG_VERSION:=0.01
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/nvram

include $(INCLUDE_DIR)/package.mk
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg

define Package/dniutil
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=DNI config utility
  DEPENDS:=
  DESCRIPTION:=\
	This package contains an utility to control DNI 'nvram' config \\\
	area.
endef

define Build/Prepare
	rmdir $(PKG_BUILD_DIR)
	ln -s ${PWD}/$(PKG_NAME)/src $(PKG_BUILD_DIR)
endef

define Build/Compile
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -I. -Iinclude -fPIC"
# Marked by Wayne on 2009/09/11
# There is no install-dniutil target.		
#	$(MAKE) -C $(PKG_BUILD_DIR) \
		PREFIX="$(PKG_INSTALL_DIR)" \
		INSTALL_DIR="$(PKG_INSTALL_DIR)/usr/sbin" \
		INSTALL_LIB="$(PKG_INSTALL_DIR)/usr/lib" \
		install-dniutil
endef

define Build/Clean
	rm -rf $(PKG_INSTALL_DIR)
	rm -rf $(PKG_BUILD_DIR)
endef



define Build/InstallDev
	mkdir -p $(STAGING_DIR)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libnvram.so $(STAGING_DIR)/usr/lib/
endef

define Build/UninstallDev
	rm -f $(STAGING_DIR)/usr/lib/libnvram.so
endef


define Package/dniutil/install
	install -d -m0755 $(1)/etc/nvram
	#$(CP) -rf $(PKG_BUILD_DIR)/default/$(BOARD).default $(1)/etc/nvram/nvram.config
ifeq ($(CONFIG_FIRMWARE_REGION_EA),y)
	$(CP) -rf $(PKG_BUILD_DIR)/default/$(BOARD)-WSR-300HP-EA.default $(1)/etc/nvram/nvram.config
else
	$(CP) -rf $(PKG_BUILD_DIR)/default/$(BOARD)-WSR-300HP.default $(1)/etc/nvram/nvram.config
endif
	#install -d -m0755 $(1)/etc/nvram_eu
	#$(CP) -rf $(PKG_BUILD_DIR)/default/$(BOARD)_eu.default $(1)/etc/nvram_eu/nvram.config
	install -d -m0755 $(1)/usr/lib
	install -d -m0755 $(1)/lib
#	$(CP) $(PKG_BUILD_DIR)/libnvram.so $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/libnvram.so $(1)/lib/
	install -d -m0755 $(1)/usr/sbin
	install -m0755 $(PKG_BUILD_DIR)/nvram $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,dniutil))

