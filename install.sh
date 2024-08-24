#!/bin/bash
if [[ $EUID -ne 0 ]]; then
  echo "This script must be run as root."
  exit 1
else
  echo 'Installing battery threshold management service.'
fi
set -e
gcc batteryd.c -o batteryd
gcc batteryctl.c -o batteryctl
mv batteryd batteryctl /usr/local/bin
cp batteryd.service /usr/lib/systemd/system
systemctl daemon-reload
systemctl enable --now batteryd.service
echo 'Install done!'
