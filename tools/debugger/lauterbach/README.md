# Lauterbach Debugger Scripts

This directory contains a template configuration for the debugging
tool Trace32 by Lauterbach.
It is automatically converted into a working project configuration by the waf
tool [`tools/waf-tools/f_lauterbach.py`](../../waf-tools/f_lauterbach.py) if
the binary of the debugging software has been found on the system.

For more information on using this, please search the foxBMS documentation
for `Lauterbach`.

## Lauterbach Commands

A list of some useful lauterbach command line commands:

## Flash Manipulation

1. Unlock flash: ``FLASH.ReProgram ALL``
1. Manipulate flash as needed
1. Lock flash: ``FLASH.ReProgram OFF``

## Deleting Flash

1. Unlock flash: ``FLASH.ReProgram ALL``
1. Erase
   - Complete flash: ``FLASH.Erase ALL``
   - Only certain memory range: ``FLASH.Erase (xxxxxx)--(yyyyyy)``
1. Lock flash: ``FLASH.ReProgram OFF``

## Saving Flash to File

1. Save flash to file: ``DATA.SAVE.BINARY filename.bin 0x08000000--0x081FFFFF``

The file is saved in lauterbach installation directory if a relative path is
used.

## Writing to Flash from File

1. Erase flash: ``FLASH.Erase ALL``
2. Unlock flash: ``FLASH.Program ALL``
3. Load and flash file:
   ``DATA.LOAD.BINARY filename.bin 0x08000000--0x081FFFFF``
4. Lock flash: ``FLASH.Program off``

## FAQ

- **Q:** I encounter "*Debug port fail*" when starting Trace32.

  **A:** Decrease the JTAG frequency.
