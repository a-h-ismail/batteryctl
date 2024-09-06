#!/bin/bash
if [[ $EUID -ne 0 ]]; then
  echo "This script must be run as root."
  exit 1
else
  echo 'Installing battery threshold management service.'
fi
groupadd --system batteryd
set -e
gcc batteryd.c -O2 -o batteryd
gcc batteryctl.c -O2 -o batteryctl
mv batteryd batteryctl /usr/local/bin
cp batteryd.service /usr/lib/systemd/system
systemctl daemon-reload
systemctl enable --now batteryd.service
echo 'Install done!'
