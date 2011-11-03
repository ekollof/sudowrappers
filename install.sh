#!/bin/sh

if [ "$(id -u)" != "0" ]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi

mkdir -p /opt/sudowrappers/
cp -rp * /opt/sudowrappers/
cd /opt/sudowrappers/libwrapper
ln -fs /opt/sudowrappers/scripts/suedit /usr/bin

