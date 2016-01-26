#!/bin/sh
#cd /usr/lib/lua/luci/sys/zoneinfo
for f in tzdata.lua tzoffset.lua
do
    mv $f $f.bak
    wget -nv --no-check-certificate https://raw.githubusercontent.com/openwrt/luci/master/modules/luci-base/luasrc/sys/zoneinfo/$f
done
