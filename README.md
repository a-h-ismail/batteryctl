# batteryd

## Purpose

This project provides easy control and persistence of battery charge limiters in Linux.

## Usage

To set battery charge limit to 80% (if your device supports it):
```
batteryc -s 80
```

To check the current charge limit:
```
batteryc -g
```

The service saves and restores the current battery charge threshold, providing persistence across reboots.

## Installation

Install `gcc` using your favorite distribution package manager, example:
```
sudo apt install gcc
```

Clone the repository and run the install script:
```
git clone https://github.com/a-h-ismail/batteryc.git
cd batteryc
chmod +x ./install.sh
sudo ./install.sh
```
Add your user to the `batteryd` group, otherwise `batteryc` will require root to work:
```
sudo usermod -aG batteryd "$(whoami)"
```
Logout then login for group changes to take effect.

## Uninstallation

Run the `remove.sh` script as root.

## Connecting to the batteryd without batteryc

- `batteryd` is launched by a systemd service and listens on the unix socket `/run/batteryd`.
- Any client (with `batteryd` group membership) connects to the socket and writes the threshold (1 byte).
- The server validates the value and attempts to set the threshold.
- The server writes its response (success or failure with reasons) in one byte and closes the connection.
