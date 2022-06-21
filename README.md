# ICE-V_WiFiMgr
TCP Socket Firmware for ICE-V with Soft AP Provisioning

## Abstract
This project is a friendlier version of the original ICE-V Wireless firmware
that includes a SoftAP captive port to do the initial SSID/Password setup.
It supports power-on initialization of the ICE40UP5k FPGA as well as wireless
programming of the FPGA, SPI access after configuration and battery
voltage monitoring.

## Prerequisites
Prior to building this firmware you'll need to have a working installation of the
Espressif ESP32 IDF V5.0 toolchain. You can find it here:

https://github.com/espressif/esp-idf

Make sure you get V5.0 and that you've done all the proper installation and
setup steps.

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

## Provisioning
Use a smartphone to attach to the HTTP server. Search for a network
named `ice-v`. Connect using the password `ice-vpwd`. A captive portal will
appear. Choose a network from scanned list, or enter your own. Fill in the
password. Hit the "JOIN" button. Wait for "SUCCESS". Enjoy.

## Restarting
You may want to erase the stored credentials. Do this:

```
idf.py -p /dev/ttyACM0 erase-flash
```

Then re-install as above.
