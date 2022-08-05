# ICE-V_WiFiMgr
TCP Socket Firmware for ICE-V with Soft AP Provisioning

## Abstract
This project is a friendlier version of the original ICE-V Wireless firmware
that includes a SoftAP captive portal to do the initial SSID/Password setup.
It supports power-on initialization of the ICE40UP5k FPGA as well as wireless
programming of the FPGA, SPI access after configuration and battery
voltage monitoring.

## Prerequisites

### ESP32 Tools
Prior to building this firmware you'll need to have a working installation of the
Espressif ESP32 IDF toolchain. Unfortunately there are many different versions
available and not all components and features used here are compatible across
them all. This project was initially built against an intermediate release of
unstable V5.0 and the most recent V5.0 no longer supports the WiFi Manager
component so it's best to fall back to the V4.4.2. You can find it here:

https://github.com/espressif/esp-idf

Make sure you get `stable (V4.4.2)` and that you've done all the proper
installation and setup steps.

### Components
This firmware also relies on the ESP32 WiFi Manager component. To get it do
the following:
```
git submodule update --init
```

## Building
### First build
For the initial build it is necessary to install the SPIFFS filesystem that holds
the default FPGA build. To do this it is necessary to uncomment a line in the
`main/CMakeLists.txt` file which informs the build system to create and flash
the filesystem into the ESP32C3 memory. Find the line

```
#spiffs_create_partition_image(storage ../spiffs FLASH_IN_PROJECT)
```
and remove the leading `#`

### Subsequent builds
After the first build the SPIFFS filesystem will exist and the firmware will know
where to find it so it's no longer necessary to recreate it every time the
firmware is updated. Return to the `main/CMakeLists.txt` and add a `#` to the
beginning of the above text line.

### How to build
Use the normal IDF build command:
```
idf.py build
```

## Installation
Connect the USB-C port of the ESP32C3 FPGA board to the build host machine and
use the command
```
idf.py -p <serial device> flash
````

where `<serial device>` is the USB serial device which is created when the board
enumerates.

## Monitoring
The board generates a fair amount of status information that is useful for
debugging and tracking performance. Use the command
```
idf.py -p <serial device> monitor
```
  
to view this information.

NOTE: For ESP-IDF V4.4.1 on, the boot-up firmware automatically disables the USB
serial port that's used for monitoring, so this feature will only show you the
first few lines of the boot-up process. Starting at some point in ESP-IDF V4.4.1
the ability to prevent disabling USB was added but is not available in earlier
versions so it's important to use the latest stable version of ESP-IDF V4.4.x

## Provisioning
Use a smartphone to attach to the SoftAP. Search for a network
named `ice-v`. Connect using the password `ice-vpwd`. A captive portal will
appear. Choose a network from scanned list, or enter your own. Fill in the
password. Hit the "JOIN" button. Wait for "SUCCESS". If you accidentally chose
the wrong network or mis-typed the password then the system may hang and should
be reset to start over.

### IMPORTANT
After you see the "SUCCESS" indicator, reset the ICE-V Wireless by pressing
the "RST" button. This is needed to restart the networking with the new
credentials and a predictable starting state.

### Resetting Credentials
If you want to clear out the network credentials for any reason you can do so
by pressing the BOOT button on the ICE-V board for approximately 3 seconds. This
will wipe the existing network SSID and password and force a restart into the
SoftAP/Captive Portal to re-enter new credentials.

## IP Addressing
This firmware uses DHCP and mDNS to request an IP address from the router on the
WiFi network and to advertise the address and its specific service/socket. If your
system doesn't support mDNS then you'll need to query your DHCP server to
find the IP address that it assigned to the ICE-V. Normally the device can be
found at the mDNS alias of `ICE-V.local` and the host-side Python interface script
defaults to using that name.
