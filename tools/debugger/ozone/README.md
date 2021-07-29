# Ozone

This directory contains a template configuration for the debugging
tool Ozone by Segger. It is automatically converted into a working project
configuration by the waf tool ``f_ozone.py`` if the binary of the debugging
software has been found on the system.

For more information on using this, please search the foxBMS documentation
for `Ozone`.

## J-Flash
With the ``foxbms.jflash`` file, the J-Flash tool can be used to program a
connected MCU.
The call to JFlash looks like this from the root of this project:

```
{path to Segger J-Flash}JFlash.exe -openprjtools/debugger/ozone/foxbms.jflash -openbuild/bin/foxbms.elf -auto -exit
```
