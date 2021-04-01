# Lauterbach command line commands

A list of some useful lauterbach command line commands:


## Flash Manipulation

1. Unlock flash: ``FLASH.ReProgram ALL``
1. Manipulate flash as needed
1. Lock flash: ``FLASH.ReProgram OFF``


## Deleting Flash

1. Unlock flash: ``FLASH.ReProgram ALL``
1. Erase
   -  Complete flash: ``FLASH.Erase ALL``
   -  Only certain memory range: ``FLASH.Erase (xxxxxx)--(yyyyyy)``
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
