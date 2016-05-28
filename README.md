# CMSIS-DAP and CDC firmware for LPC11U35

This firmware implements a composite USB device with two classes:
- CMSIS-DAP HID interface, based on [Armstart's IBDAP-CMSIS-DAP Project](https://github.com/Armstart-com/IBDAP-CMSIS-DAP)
- USB CDC virtual communication serial port, connected to target's physical UART, based on NXP LPCOpen CDC-UART example.

## Supported and tested target:
- LPC11U35FBD48/401

## Available libraries:
- [CMSIS](http://www.arm.com/products/processors/cortex-m/cortex-microcontroller-software-interface-standard.php) and [LPCOpen](https://www.lpcware.com/lpcopen)

## Supported toolchains:
- gcc-arm-none-eabi

