/* 4b7cc0f4978f9f1747cf2a726c35d4e8 */

/* Aligned with names in "MEMORY" in the elf-linker script */
ROMS
{
    /* FLASH */
    VECTORS_TABLE           : origin = 0x00000000
                              length = 0x20
                              fill   = 0xFFFFFFFF
    KERNEL_FUNCTIONS        : origin = 0x00000020
                              length = 0x8000
                              fill   = 0xFFFFFFFF
    FLASH                   : origin = 0x00008020 /*(0x00000000 + 0x20 + 0x8000)*/
                              length = 0x3F7F20
                              fill   = 0xFFFFFFFF
    VER_VERSION_INFORMATION : origin = 0x003FFF40 /*(0x00000000 + 0x20 + 0x8000 + 0x3F7F20) */
                              length = 0xC0
                              fill   = 0xFFFFFFFF
    /* RAM */
    STACKS                  : origin = 0x08000000
                              length = 0x800
    KERNEL_DATA             : origin = 0x08000800
                              length = 0x800
    RAM                     : origin = 0x08001000
                              length = 0x7D000
    SHARED_RAM              : origin = 0x0807E000
                              length = 0x2000

    ECC_VECTORS_TABLE       : origin = 0xF0400000
                              length = 0x4
    ECC_KERNEL_FUNCTIONS    : origin = 0xF0400004
                              length = 0x1000
    ECC_FLASH               : origin = 0xF0401004
                              length = 0x7EFE4
    ECC_VERSION_INFORMATION : origin = 0xF047FFE8
                              length = 0x18

}
