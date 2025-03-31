/**
 *
 * @copyright &copy; 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * We kindly request you to use one or more of the following phrases to refer to
 * foxBMS in your hardware, software, documentation or advertising materials:
 *
 * - "This product uses parts of foxBMS&reg;"
 * - "This product includes parts of foxBMS&reg;"
 * - "This product is derived from foxBMS&reg;"
 *
 */

/* docref: SPNU151W JANUARY 1998 - REVISED MARCH 2023, p. 26 */
/* Retain a list of sections that otherwise would be discarded */
--retain="*(.intvecs_init)"
--retain="*(.intvecs)"

 /* Tells the linker to set the C stack and heap size */
--stack_size=0x8000 /* SOFTWARE STACK SIZE */
--heap_size=0x2000 /* HEAP AREA SIZE */

/*========== TMS570LC43xx Memory Map ========================================*/

/* docref: SPNS195C - FEBRUARY 2014 - REVISED JUNE 2016
 *  TMS570LC4357 Hercules&trade; Microcontroller Based on the ARM&reg; Cortex&reg;-R Core SPNS195C
 *  6.9.1 Memory Map Diagram (Page 90)
 *   - Figure 6-9. Memory Map: Flash ECC, OTP and EEPROM accesses) from 0xF000 0000 to 0xF047 FFFF
 *  6.9.2 Memory Map Table (Page 91)
 *   - Table 6-25. Module Registers / Memories Memory Map:
 *     Flash Data Space ECC start:   0xF040 0000
 *     Flash Data Space ECC end:     0xF05F FFFF (but only usable till 0xF047 FFFF, see also memory map)
 */

/* Size of flash banks in TMS570LC43xx */
#define FLASH_BANK_SIZE (0x200000)

/* Global size of the FLASH memory in the MCU (TMS570LC43xx has 2 flash banks) */
#define GLOBAL_FLASH_SIZE (2 * FLASH_BANK_SIZE)

/* Addresses of flash banks */
#define FLASH_BANK0_START_ADDRESS (0x00000000)
#define FLASH_BANK0_END_ADDRESS   (0x001FFFFF)
#define FLASH_BANK1_START_ADDRESS (0x00200000)
#define FLASH_BANK1_END_ADDRESS   (0x003FFFFF)

/* Data ECC area: 0xF0400000 to 0xF0480000 */
#define ECC_DATA_START_ADDRESS (0xF0400000)
/* Size of RAM in TMS570LC43xx */
#define GLOBAL_RAM_SIZE (0x80000)
/* Global start address of RAM in TMS570LC43xx */
#define GLOBAL_RAM_START_ADDRESS (0x08000000)

/*========== Flash section configuration ====================================*/
/* Start address of vector table */
#define VECTORS_TABLE_INIT_START_ADDRESS (0x00000000)
/* Size of vector table */
#define VECTORS_SIZE (0x00000020)
/* Size of flash for bootloader */
#define BOOTLOADER_SIZE (0x00017FE0)
/* Size of information block:
    +----------------+-----------------------------------------------------+
    | Address        | What is stored under this address?                  |
    +----------------+-----------------------------------------------------+
    | 0x00018000     | magic number (0xAAAAAAAA) to show that the program  |
    |                | info sector has been written deliberately           |
    +----------------+-----------------------------------------------------+
    | 0x00018004     | the length of the program in bytes                  |
    +----------------+-----------------------------------------------------+
    | 0x0001800C     | the 8-byte CRC signature of the program             |
    +----------------+-----------------------------------------------------+
    | 0x00018014     | the 8-byte CRC signature of the vector table        |
    +----------------+-----------------------------------------------------+
    | 0x0001801C     | a 4 byte bit pattern to show if there is a program  |
    |                | available on board                                  |
    |                | (xCCCCCCCC means there is a program available)      |
    +----------------+-----------------------------------------------------+
 */
#define PROGRAM_INFO_AREA_SIZE (0x00007FE0)
/* Size of flash for application */
#define PROGRAM_PLACE_HOLDER_SIZE (0x003E0000)
/* Fill pattern for flash */
#define VFILL_FLASH_PATTERN (0xffffffff)

/*========== RAM section configuration ======================================*/
/* Size of stack */
#define STACK_SIZE (0x00010000)
/* Size of RAM */
#define RAM_SIZE (0x00020000)
/* Size of the RAM that will only be used as sector buffer to flash the sectors */
#define RAM_FLASH_SIZE (0x00040000)
/* Size of the RAM where the flash relevant functions will be loaded */
#define RAM_FLASH_API_SIZE (0x00010000)

/*========== ECC Flash section configuration ================================*/
/* Sizes for ECC */
#define ECC_VECTORS_TABLE_INIT_SIZE            (VECTORS_SIZE/8)
#define ECC_BOOTLOADER_SIZE                    (BOOTLOADER_SIZE/8)
#define ECC_PROGRAM_INFO_AREA_SIZE             (PROGRAM_INFO_AREA_SIZE/8)
#define ECC_VECTORS_TABLE_SIZE                 (VECTORS_SIZE/8)
#define ECC_PROGRAM_PLACE_HOLDER_SIZE          (PROGRAM_PLACE_HOLDER_SIZE/8)

/*========== Memory Layout ==================================================*/
MEMORY
{
    /* FLASH */
    VECTORS_TABLE_INIT       (X)   :     origin=VECTORS_TABLE_INIT_START_ADDRESS
                                         length=VECTORS_SIZE
                                         vfill = VFILL_FLASH_PATTERN
    BOOTLOADER               (RX)  :     origin=end(VECTORS_TABLE_INIT)
                                         length=BOOTLOADER_SIZE
                                         vfill = VFILL_FLASH_PATTERN
    PROGRAM_INFO_AREA        (RX)  :     origin=end(BOOTLOADER)
                                         length=PROGRAM_INFO_AREA_SIZE
                                         vfill = VFILL_FLASH_PATTERN
    VECTORS_TABLE            (X)   :     origin=end(PROGRAM_INFO_AREA)
                                         length=VECTORS_SIZE
                                         vfill = VFILL_FLASH_PATTERN
    PROGRAM_PLACE_HOLDER     (RX)  :     origin=end(VECTORS_TABLE)
                                         length=PROGRAM_PLACE_HOLDER_SIZE
                                         vfill = VFILL_FLASH_PATTERN

    /* RAM */
    STACK                    (RW)  :     origin=GLOBAL_RAM_START_ADDRESS
                                         length=STACK_SIZE
    RAM                      (RWX) :     origin=end(STACK)
                                         length=RAM_SIZE
    RAM_FLASH                (RWX) :     origin=end(RAM)
                                         length=RAM_FLASH_SIZE
    RAM_FLASH_API            (RWX) :     origin=end(RAM_FLASH)
                                         length=RAM_FLASH_API_SIZE

    /* ECC */
    ECC_VECTORS_TABLE_INIT   (R)   :     origin = ECC_DATA_START_ADDRESS
                                         length = ECC_VECTORS_TABLE_INIT_SIZE
                                         ECC    = { input_range = VECTORS_TABLE_INIT }

    ECC_BOOTLOADER           (R)   :     origin = end(ECC_VECTORS_TABLE_INIT)
                                         length = ECC_BOOTLOADER_SIZE
                                         ECC    = { input_range = BOOTLOADER }

    ECC_PROGRAM_INFO_AREA    (R)   :     origin = end(ECC_BOOTLOADER)
                                         length = ECC_PROGRAM_INFO_AREA_SIZE
                                         ECC    = { input_range = PROGRAM_INFO_AREA }

    ECC_VECTORS_TABLE        (R)   :     origin = end(ECC_PROGRAM_INFO_AREA)
                                         length = ECC_VECTORS_TABLE_SIZE
                                         ECC    = {algorithm=algoL2R5F021, input_range=VECTORS_TABLE}

    ECC_PROGRAM_PLACE_HOLDER (R)   :     origin = end(ECC_VECTORS_TABLE)
                                         length=ECC_PROGRAM_PLACE_HOLDER_SIZE
                                         ECC={algorithm=algoL2R5F021, input_range=PROGRAM_PLACE_HOLDER}
}

/*========== ECC Algorithm Configuration ====================================*/
ECC
{
    algoL2R5F021 : address_mask = 0xfffffff8 /* Address Bits 31:3 */
                   hamming_mask = R4         /* Use R4/R5 build in Mask */
                   parity_mask  = 0x0c       /* Set which ECC bits are Even and Odd parity */
                   mirroring    = F021       /* RM57Lx and TMS570LCx are build in F021 */
}

/*========== Section Layout =================================================*/
SECTIONS
{
    .intvecs_init       : {} > VECTORS_TABLE_INIT
    .intvecs            : {} > VECTORS_TABLE
    .text   align(32)   : {} > BOOTLOADER
    .const  align(32)   : {} > BOOTLOADER
    .cinit  align(32)   : {} > BOOTLOADER
    .pinit  align(32)   : {} > BOOTLOADER
    .bss                : {} > RAM
    .data               : {} > RAM
    .sysmem             : {} > RAM
}
