/*----------------------------------------------------------------------------*/
/* sys_link_freeRTOS.cmd                                                      */
/*                                                                            */
/*
* Copyright (C) 2009-2018 Texas Instruments Incorporated - www.ti.com
*
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

--retain="*(.intvecs)"

MEMORY
{
    VECTORS       (X)  : origin=0x00000000 length=0x00000020 vfill = 0xffffffff
    KERNEL        (RX) : origin=0x00000020 length=0x00008000 vfill = 0xffffffff
    FLASH0        (RX) : origin=0x00008020 length=0x001F7FE0 vfill = 0xffffffff
    FLASH1        (RX) : origin=0x00200000 length=0x00200000 vfill = 0xffffffff
    STACKS        (RW) : origin=0x08000000 length=0x00000800
    KRAM          (RW) : origin=0x08000800 length=0x00000800
    RAM           (RW) : origin=(0x08000800+0x00000800) length=(0x0007f800 - 0x00000800 - 0x00001000)
    SHAREDRAM     (RW) : origin=((0x08000800+0x00000800)+(0x0007F800-0x00000800-0x00001000)) length=0x00001000

    ECC_VEC (R)  : origin=0xf0400000 length=0x4 ECC={ input_range=VECTORS }
    ECC_KERN (R) : origin=0xf0400000 + 0x4 length=0x1000 ECC={ input_range=KERNEL }
    ECC_FLA0 (R) : origin=0xf0400000 + 0x4 + 0x1000 length=0x3EFFC ECC={ input_range=FLASH0 }
    ECC_FLA1 (R) : origin=0xf0400000 + 0x4 + 0x1000 + 0x3EFFC length=0x40000 ECC={ input_range=FLASH1 }
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
    .pinit       align(32) : {} > KERNEL
    /* Rest of code to user mode flash region */
    .syscallTEXT align(32) : {} > FLASH0 | FLASH1
    .text        align(32) : {} > FLASH0 | FLASH1
    .const       align(32) : {} > FLASH0 | FLASH1
    /* FreeRTOS Kernel data in protected region of RAM */
    .kernelBSS    : {} > KRAM
    .kernelHEAP   : {} > RAM
    .bss          : {} > RAM
    .data         : {} > RAM
    .sysmem       : {} > RAM
    FEE_TEXT_SECTION align(32) : {} > FLASH0 | FLASH1
    FEE_CONST_SECTION align(32): {} > FLASH0 | FLASH1
    FEE_DATA_SECTION : {} > RAM

    .sharedRAM : {} > SHAREDRAM
}
