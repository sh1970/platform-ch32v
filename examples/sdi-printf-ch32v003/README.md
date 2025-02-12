Info
====

This example contains code from https://github.com/openwch/ch32v003/tree/main/EVT/EXAM/SDI_Printf/SDI_Printf repository.

In order to get the `printf` on the terminal, first SDI Printf needs to be enabled on the "Project Tasks":

![alt text](image.png)

How to build PlatformIO based project
=====================================

1. [Install PlatformIO Core](https://docs.platformio.org/page/core.html)
2. Download [development platform with examples](https://github.com/Community-PIO-CH32V/platform-ch32v/archive/develop.zip)
3. Extract ZIP archive
4. Run these commands:

```shell
# Change directory to example
$ cd platform-ch32v/examples/sdi-printf-ch32v003

# Build project
$ pio run

# Upload firmware
$ pio run --target upload

# Upload firmware for the specific environment
$ pio run -e ch32v003f4p6_evt_r0 --target upload

# Clean build files
$ pio run --target clean
```
