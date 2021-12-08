/* fdf552cfcdae551513b4abe184fe9e05 */
ROMS
{
    VECTORS    : origin=0x00000000 length=0x00000020
    KERNEL     : origin=0x00000020 length=0x00008000
    FLASH0     : origin=0x00008020 length=0x001F7FE0
    FLASH1     : origin=0x00200000 length=0x001FFF40
    VERSIONINFO: origin=0x003FFF40 length=0x000000C0
    STACKS     : origin=0x08000000 length=0x00000800
    KRAM       : origin=0x08000800 length=0x00000800
    RAM        : origin=0x08001000 length=0x0007E000
    SHAREDRAM  : origin=0x0807F000 length=0x00001000

    ECC_VEC    : origin=0xf0400000 length=0x4
    ECC_KERN   : origin=0xf0400004 length=0x1000
    ECC_FLA0   : origin=0xf0401004 length=0x3EFFC
    ECC_FLA1   : origin=0xf0440000 length=0x40000
}
