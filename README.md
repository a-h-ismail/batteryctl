# batteryctl

## Purpose

This project provides easy control and persistence of battery charge limiters in Linux.

## Usage

To set battery charge limit to 80% (if your device supports it):
```
batteryctl -s 80
```

To check the current charge limit:
```
batteryctl -g
```

The service saves and restores the current battery charge threshold, providing persistence across reboots.

## Installation

Install `gcc` using your favorite distribution package manager, example:
```
sudo apt install gcc
```

Clone the repository and run the install script:
```
git clone https://github.com/a-h-ismail/batteryctl.git
cd batteryctl
chmod +x ./install.sh
sudo ./install.sh
```
Add your user to the `batteryd` group, otherwise `batteryctl` will require root to work:
```
sudo usermod -aG batteryd "$(whoami)"
```
Logout then login for group changes to take effect.
