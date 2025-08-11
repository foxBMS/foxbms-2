/* c0c40b9d037bb0eb743c359247de721e */

/* Aligned with names in "MEMORY" in the elf-linker script */
ROMS
{
    /* FLASH */
    VECTORS_TABLE           : origin = 0x0001FFE0
                              length = 0x20
                              fill   = 0xFFFFFFFF
    VER_VERSION_INFORMATION : origin = 0x00020000
                              length = 0xC0
                              fill   = 0xFFFFFFFF
    KERNEL_FUNCTIONS        : origin = 0x000200C0
                              length = 0x8000
                              fill   = 0xFFFFFFFF
    APP_FLASH               : origin = 0x000280C0
                              length = 0x3D7F40
                              fill   = 0xFFFFFFFF

    /* RAM */
    STACKS                  : origin = 0x08000000
                              length = 0x800
    KERNEL_DATA             : origin = 0x08000800
                              length = 0x1000
    RAM                     : origin = 0x08001800
                              length = 0x7C800
    SHARED_RAM              : origin = 0x0807E000
                              length = 0x2000

    ECC_VECTORS_TABLE       : origin = 0xF0403FFC
                              length = 0x4
    ECC_VERSION_INFORMATION : origin = 0xF0404000
                              length = 0x18
    ECC_KERNEL_FUNCTIONS    : origin = 0xF0404018
                              length = 0x1000
    ECC_APP_FLASH           : origin = 0xF0405018
                              length = 0x7AFE8


}
