# FreeRTOS-Plus-TCP Library for TI TMS570LC43x in foxBMS

foxBMS 2 uses FreeRTOS as operating system and the FreeRTOS-Plus-TCP Library
for TCP/IP support.
This document gives a short overview of the changes that were applied to have
the FreeRTOS-Plus-TCP Library working on the ``TI TMS570LC43x``.

## Basis

The basis for FreeRTOS TCP-IP support within the foxBMS 2 project is
``FreeRTOS-Plus-TCP Library V4.3.1``.

The release is obtained from <https://github.com/FreeRTOS/FreeRTOS-Plus-TCP>.

## Changes

Only the following files of the release are distributed within the foxBMS
source tree:

- `freertos-plus-tcp/source/include/*`
- `freertos-plus-tcp/source/*.c`
- `freertos-plus-tcp/source/portable/BufferManagement/*`
- `freertos-plus-tcp/source/portable/Compiler/CCS/*`
- `freertos-plus-tcp/source/portable/NetworkInterface/Common/*`
- `freertos-plus-tcp/source/portable/NetworkInterface/include/*`
- `freertos-plus-tcp/source/portable/NetworkInterface/loopback/*`
- `freertos-plus-tcp/source/portable/NetworkInterface/README_DRIVER_DISCLAIMER.txt`
- `freertos-plus-tcp/GettingStarted.md`
- `freertos-plus-tcp/HISTORY.txt`
- `freertos-plus-tcp/LICENSE.md`
- `freertos-plus-tcp/README.md`
- `freertos-plus-tcp/readme.txt`
- `freertos-plus-tcp/ReadMe.url`

## Further Reading

TODO
