/**
 *
 * @copyright &copy; 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * - &Prime;This product uses parts of foxBMS&reg;&Prime;
 * - &Prime;This product includes parts of foxBMS&reg;&Prime;
 * - &Prime;This product is derived from foxBMS&reg;&Prime;
 *
 */

/* docref: SPNU118Y - September 1995 - Revised February 2020, p. 133
 * never remove intvecs during linking */
--retain="*(.intvecs)"

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

/* size of flash banks in TMS570LC43xx */
#define FLASH_BANK_SIZE (0x200000)

/* global size of the FLASH memory in the MCU (TMS570LC43xx has 2 flash banks) */
#define GLOBAL_FLASH_SIZE (2 * FLASH_BANK_SIZE)

#define FLASH_BANK0_START_ADDRESS (0x00000000)
#define FLASH_BANK0_END_ADDRESS   (0x001FFFFF)
#define FLASH_BANK1_START_ADDRESS (0x00200000)
#define FLASH_BANK1_END_ADDRESS   (0x003FFFFF)

/* Data ECC area: 0xF0400000 to 0xF0480000 */
#define ECC_DATA_START_ADDRESS (0xF0400000)

/* start address of the vectors table in ARM (if not otherwise defined, i.e., during flashing) */
#define VECTORS_TABLE_START_ADDRESS (0x00000000)

/* size of RAM in TMS570LC43xx */
#define GLOBAL_RAM_SIZE (0x80000)
/* global start address of RAM in TMS570LC43xx */
#define GLOBAL_RAM_START_ADDRESS (0x08000000)

/*========== Flash section configuration ====================================*/

#define VFILL_FLASH_PATTERN (0xffffffff)

/* size of VER_VERSION_INFORMATION (has to be large enough to accomodate the version struct) */
#define VERSION_INFORMATION_SIZE (0xC0)

/* size of the VECTORS table */
#define VECTORS_TABLE_SIZE (0x20)

/* size of KERNEL (accomodates the OS) */
#define KERNEL_FUNCTIONS_SIZE (0x8000)

/* remaining size of flash bank 0 as at the beginning of flash bank 0 the vector
 * table and version information struct are placed */
#define REMAINING_FLASH_BANK0_SIZE (FLASH_BANK_SIZE - VECTORS_TABLE_SIZE - KERNEL_FUNCTIONS_SIZE - VERSION_INFORMATION_SIZE)

/* program usable flash size is "remaining flash bank0 size + complete flash bank 1 size" */
#define FLASH_SIZE (REMAINING_FLASH_BANK0_SIZE + FLASH_BANK_SIZE)

/*========== ECC Flash section configuration ================================*/
/* sizes for ECC */
#define ECC_VECTORS_TABLE_SIZE          (VECTORS_TABLE_SIZE/8)
#define ECC_KERNEL_FUNCTIONS_SIZE       (KERNEL_FUNCTIONS_SIZE/8)
#define ECC_FLASH_SIZE                  (FLASH_SIZE/8)
#define ECC_VERSION_INFORMATION_SIZE    (VERSION_INFORMATION_SIZE/8)

/*========== RAM section configuration ======================================*/

/* The size of the stacks has to be exactly the same as DRIVER.SYSTEM.VAR.RAM_STACK_LENGTH.VALUE from HalCoGen! */
/* The stack consists of:
 * - user stack
 * - supervisor stack
 * - FIQ stack (FIQ: fast interrupt)
 * - IRQ stack (IRQ: normal interrupt)
 * - Abort stack
 * - Undefined stack
 */
#define STACKS_SIZE (0x800)

/* size of the FreeRTOS RAM; TODO does this have to fit to another value? */
#define KERNEL_DATA_SIZE (0x800)

/* size of shared-RAM section for DMA */
#define SHARED_RAM_SIZE (0x2000)

/*========== Memory Layout ==================================================*/
MEMORY
{
    /* FLASH */
    VECTORS_TABLE         (X) : origin = VECTORS_TABLE_START_ADDRESS
                                length = VECTORS_TABLE_SIZE
                                vfill  = VFILL_FLASH_PATTERN
    KERNEL_FUNCTIONS     (RX) : origin = end(VECTORS_TABLE)
                                length = KERNEL_FUNCTIONS_SIZE
                                vfill  = VFILL_FLASH_PATTERN
    FLASH                (RX) : origin = end(KERNEL_FUNCTIONS)
                                length = FLASH_SIZE
                                vfill  = VFILL_FLASH_PATTERN
    VER_VERSION_INFORMATION  (RX) : origin = end(FLASH)
                                length = VERSION_INFORMATION_SIZE
                                vfill  = VFILL_FLASH_PATTERN
    /* RAM */
    STACKS               (RW) : origin = GLOBAL_RAM_START_ADDRESS
                                length = STACKS_SIZE
    KERNEL_DATA          (RW) : origin = end(STACKS)
                                length = KERNEL_DATA_SIZE
    RAM                  (RW) : origin = end(KERNEL_DATA)
                                length = (GLOBAL_RAM_SIZE - STACKS_SIZE - KERNEL_DATA_SIZE - SHARED_RAM_SIZE)
    SHARED_RAM           (RW) : origin = end(RAM)
                                length = SHARED_RAM_SIZE

    ECC_VECTORS_TABLE       (R) : origin = ECC_DATA_START_ADDRESS
                                  length = ECC_VECTORS_TABLE_SIZE
                                  ECC    = { input_range = VECTORS_TABLE }
    ECC_KERNEL_FUNCTIONS    (R) : origin = end(ECC_VECTORS_TABLE)
                                  length = ECC_KERNEL_FUNCTIONS_SIZE
                                  ECC    = { input_range = KERNEL_FUNCTIONS }
    ECC_FLASH               (R) : origin = end(ECC_KERNEL_FUNCTIONS)
                                  length = ECC_FLASH_SIZE
                                  ECC    = { input_range = FLASH }
    ECC_VERSION_INFORMATION (R) : origin = end(ECC_FLASH)
                                  length = ECC_VERSION_INFORMATION_SIZE
                                  ECC    = { input_range = VER_VERSION_INFORMATION }
}

/*========== ECC Algorithm Configuration ====================================*/
ECC
{
   algo_name : address_mask = 0xfffffff8
   hamming_mask = R4
   parity_mask = 0x0c
   mirroring = F021
}

/*========== Section Layout =================================================*/
SECTIONS
{
    .intvecs                      : {} > VECTORS_TABLE
    /* FreeRTOS Kernel in protected region of Flash */
    .kernelTEXT         align(32) : {} > KERNEL_FUNCTIONS
    .cinit              align(32) : {} > KERNEL_FUNCTIONS
    /* Rest of code to user mode flash region */
    .versionInformation align(32) : {} > VER_VERSION_INFORMATION
    .syscallTEXT        align(32) : {} > FLASH
    .text               align(32) : {} > FLASH
    .const              align(32) : {} > FLASH
    /* FreeRTOS Kernel data in protected region of RAM */
    .kernelBSS                    : {} > KERNEL_DATA
    .bss                          : {} > RAM
    .data                         : {} > RAM
    .sysmem                       : {} > RAM
    .sharedRAM                    : {} > SHARED_RAM
}
