#!/bin/sh
host_os=`uname -s`
if [ "$host_os" = "Linux" ]; then
l=`ipcs -m | grep "$USER" | awk -F" " '{print $1}'`
elif [ "$host_os" = "Darwin" -o  "$host_os" = "FreeBSD" ]; then
l=`ipcs -m | grep "$USER" | awk -F" " '{print $3}'`
fi
for s in $l; do echo $s; ipcrm -M $s; done
if [ ! -z "$l" ]; then echo $l shared memory\(s\) for $USER removed; fi
