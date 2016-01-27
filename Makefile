# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 4841 2006-09-23 19:28:18Z nico $

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
		CFLAGS="$(TARGET_CFLAGS) -I. -Iinclude"
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
	$(CP) -rf $(PKG_BUILD_DIR)/default/$(BOARD).default $(1)/etc/nvram/nvram.config
ifeq ("$(MODELNAME)", "2_6_LANTIQ")
	$(CP) -rf $(PKG_BUILD_DIR)/default/$(BOARD).default $(1)/etc/nvram/nvram.config
endif
ifeq ("$(MODELNAME)", "2_6_VEGN2500")
	$(CP) -rf $(PKG_BUILD_DIR)/default/$(BOARD)-VEGN-VRX288.default $(1)/etc/nvram/nvram.config
endif
ifeq ("$(MODELNAME)", "2_6_VEGN2200")
	$(CP) -rf $(PKG_BUILD_DIR)/default/$(BOARD)-VEGN-VRX268.default $(1)/etc/nvram/nvram.config
endif
ifeq ("$(MODELNAME)", "2_6_MVBR1000v4")
	$(CP) -rf $(PKG_BUILD_DIR)/default/$(BOARD)-MVBR1000v4.default $(1)/etc/nvram/nvram.config
endif
ifeq ("$(MODELNAME)", "2_6_VEVG3000")
ifeq ($(CONFIG_SFB),y)
		$(CP) -rf $(PKG_BUILD_DIR)/default/$(BOARD)-EVG3000.default $(1)/etc/nvram/nvram.config
else
		$(CP) -rf $(PKG_BUILD_DIR)/default/$(BOARD)-VEVG3000.default $(1)/etc/nvram/nvram.config
ifeq ($(CONFIG_ANNEXB),y)
			echo "annex=B" >> $(1)/etc/nvram/nvram.config
			echo "max_dsl_line=1" >> $(1)/etc/nvram/nvram.config
else
			echo "annex=A" >> $(1)/etc/nvram/nvram.config
endif
endif
endif
ifeq ("$(MODELNAME)", "2_6_D6300")
	$(CP) -rf $(PKG_BUILD_DIR)/default/$(BOARD)-D6300-GRX388.default $(1)/etc/nvram/nvram.config
endif
ifeq ("$(MODELNAME)", "2_6_D6400")
	$(CP) -rf $(PKG_BUILD_DIR)/default/$(BOARD)-D6400-VRX388.default $(1)/etc/nvram/nvram.config
endif
ifeq ("$(MODELNAME)", "2_6_D6100")
	$(CP) -rf $(PKG_BUILD_DIR)/default/$(BOARD)-D6100-GRX388.default $(1)/etc/nvram/nvram.config
endif
ifeq ("$(MODELNAME)", "2_6_D6200")
	$(CP) -rf $(PKG_BUILD_DIR)/default/$(BOARD)-D6200-GRX388.default $(1)/etc/nvram/nvram.config
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

