/**
 *
 * @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

--retain="*(.intvecs)"

/* length of VERSIONINFO (has to be large enough to accomodate the version struct) */
#define LENGTH_VERSIONINFO 0xC0
/* length of the VECTORS table */
#define LENGTH_VECTORS 0x20
/* length of KERNEL (accomodates the OS) */
#define LENGTH_KERNEL 0x8000
/* length of FLASH0 (flash section 1 of 2) */
#define LENGHT_FLASH0 0x1F7FE0

/* global size of the FLASH memory in the MCU */
#define GLOBAL_FLASH_SIZE 0x400000

/* The length of the stacks has to be exactly the same as DRIVER.SYSTEM.VAR.RAM_STACK_LENGTH.VALUE from HalCoGen! */
#define LENGTH_STACKS 0x800
/* length of the FreeRTOS RAM; TODO does this have to fit to another value? */
#define LENGTH_KRAM 0x800
/* length of sharedram section for dma */
#define LENGTH_SHAREDRAM 0x1000

/* global size of the RAM hardware on this MCU */
#define GLOBAL_RAM_SIZE 0x80000
/* global start address of RAM hardware on this MCU */
#define GLOBAL_RAM_START 0x8000000

MEMORY
{
    VECTORS     (X)  :  origin=0x00000000
                        length=LENGTH_VECTORS
                        vfill = 0xffffffff
    KERNEL      (RX) :  origin=end(VECTORS)
                        length=LENGTH_KERNEL
                        vfill = 0xffffffff
    FLASH0      (RX) :  origin=end(KERNEL)
                        length=LENGHT_FLASH0
                        vfill = 0xffffffff
    FLASH1      (RX) :  origin=end(FLASH0)
                        length=(GLOBAL_FLASH_SIZE - size(VECTORS) - size(KERNEL) - size(FLASH0) - LENGTH_VERSIONINFO)
                        vfill = 0xffffffff
    VERSIONINFO (RX) :  origin=end(FLASH1)
                        length=LENGTH_VERSIONINFO
                        vfill = 0xffffffff
    STACKS      (RW) :  origin=GLOBAL_RAM_START
                        length=LENGTH_STACKS
    KRAM        (RW) :  origin=end(STACKS)
                        length=LENGTH_KRAM
    RAM         (RW) :  origin=end(KRAM)
                        length=(GLOBAL_RAM_SIZE - size(STACKS) - size(KRAM) - LENGTH_SHAREDRAM)
    SHAREDRAM   (RW) :  origin=end(RAM)
                        length=LENGTH_SHAREDRAM

    ECC_VEC     (R)  :  origin=0xf0400000
                        length=0x4 ECC={ input_range=VECTORS }
    ECC_KERN    (R)  :  origin=0xf0400000 + 0x4
                        length=0x1000 ECC={ input_range=KERNEL }
    ECC_FLA0    (R)  :  origin=0xf0400000 + 0x4 + 0x1000
                        length=0x3EFFC ECC={ input_range=FLASH0 }
    ECC_FLA1    (R)  :  origin=0xf0400000 + 0x4 + 0x1000 + 0x3EFFC
                        length=0x40000 ECC={ input_range=FLASH1 }
}

ECC
{
   algo_name : address_mask = 0xfffffff8
   hamming_mask = R4
   parity_mask = 0x0c
   mirroring = F021
}

SECTIONS
{
    .intvecs : {} > VECTORS
    /* FreeRTOS Kernel in protected region of Flash */
    .kernelTEXT  align(32) : {} > KERNEL
    .cinit       align(32) : {} > KERNEL
    /* Rest of code to user mode flash region */
    .versionInformation align(32) : {} > VERSIONINFO
    .syscallTEXT        align(32) : {} > FLASH0 | FLASH1
    .text               align(32) : {} > FLASH0 | FLASH1
    .const              align(32) : {} > FLASH0 | FLASH1
    /* FreeRTOS Kernel data in protected region of RAM */
    .kernelBSS    : {} > KRAM
    .bss          : {} > RAM
    .data         : {} > RAM
    .sysmem       : {} > RAM

    .sharedRAM : {} > SHAREDRAM
}
