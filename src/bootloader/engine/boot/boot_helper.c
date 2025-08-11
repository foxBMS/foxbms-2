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

/**
 * @file    boot_helper.c
 * @author  foxBMS Team
 * @date    2021-08-02 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  BOOT
 *
 * @brief   File that contains the implementation of
 *          the functions that can assist the functions in boot.c and boot.h
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "boot_helper.h"

#include "boot_cfg.h"

#include "HL_sys_core.h"
#include "HL_system.h"

#include "flash.h"
#include "fstd_types.h"
#include "fstring.h"
#include "fsystem.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/
#ifdef UNITY_UNIT_TEST
volatile uint32_t BOOT_SYSTEM_REG1_SYSECR = 0u;
#endif

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern void BOOT_LoadProgramInfoFromFlash(void) {
    /* Disable IRQ interrupt to prevent any issue caused by interrupt (e.g.,
     * CAN RX) */
    _disable_IRQ_interrupt_();

    /* The program info sector will be intentionally written in two cases:
     *   1: during resetting, the program info sector will be initialized;
     *   2: after validating a received program.
     * In both cases, the magic number will be written into the beginning of
     * the program info sector in the flash memory. This is to prevent reading
     * from flash memory in cases where it has not been intentionally written.
     */
    uint32_t boot_programInfoMagicNumber = 0u;
    boot_programInfoMagicNumber          = *((uint32_t *)BOOT_PROGRAM_INFO_ADDRESS_MAGIC_NUM);

    /* Check if the program info section has ever been written intentionally by
     * checking its magic number */
    if (boot_programInfoMagicNumber == BOOT_PROGRAM_INFO_MAGIC_NUM) {
        /* Load information of last flashed program from flash to
         * boot_infoOfLastFlashedProgram */
        boot_infoOfLastFlashedProgram.programLength       = *((uint32_t *)BOOT_PROGRAM_INFO_ADDRESS_PROGRAM_LEN);
        boot_infoOfLastFlashedProgram.programStartAddress = *((uint32_t *)BOOT_PROGRAM_INFO_ADDRESS_PROGRAM_START);
        boot_infoOfLastFlashedProgram.programCrc8Bytes    = *((uint64_t *)BOOT_PROGRAM_INFO_ADDRESS_CRC_8_BYTES);
        boot_infoOfLastFlashedProgram.vectorTableCrc8Bytes =
            *((uint64_t *)BOOT_PROGRAM_INFO_ADDRESS_VECTOR_CRC_8_BYTES);
        boot_infoOfLastFlashedProgram.isProgramAvailable =
            *((uint32_t *)BOOT_PROGRAM_INFO_ADDRESS_IS_PROGRAM_AVAILABLE);

        /* Load the bootloader vector table from flash to variable
         * boot_backupVectorTable */
        memcpy(
            &boot_backupVectorTable.vectorTable, (uint8_t *)BOOT_VECTOR_TABLE_BACKUP_ADDRESS, BOOT_VECTOR_TABLE_SIZE);
    } else {
        /* Set the boot_infoOfLastFlashedProgram by default values */
        boot_infoOfLastFlashedProgram.programLength        = 0u;
        boot_infoOfLastFlashedProgram.programStartAddress  = 0u;
        boot_infoOfLastFlashedProgram.programCrc8Bytes     = 0u;
        boot_infoOfLastFlashedProgram.vectorTableCrc8Bytes = 0u;
        boot_infoOfLastFlashedProgram.isProgramAvailable   = 0u;
        for (uint8_t i_vector = 0u; i_vector < BOOT_NUM_OF_VECTOR_TABLE_8_BYTES; i_vector++) {
            boot_backupVectorTable.vectorTable[i_vector] = 0u;
        }
    }

    /* Load the current vector table from flash to variable
     * boot_currentVectorTable */
    memcpy(&boot_currentVectorTable.vectorTable, (uint8_t *)BOOT_VECTOR_TABLE_START_ADDRESS, BOOT_VECTOR_TABLE_SIZE);
}

extern void BOOT_CleanUpRam(void) {
    /* Disable IRQ interrupt to prevent any issue caused by interrupt (e.g.,
    CAN RX) */
    _disable_IRQ_interrupt_();
    memset((uint8_t *)BOOT_RAM_START_ADDRESS, 0, BOOT_RAM_SIZE);
}

extern STD_RETURN_TYPE_e BOOT_UpdateProgramInfoIntoFlash(void) {
    STD_RETURN_TYPE_e retVal = STD_OK;
    bool gotoNext            = true;

    /* Disable IRQ interrupt to prevent any issue caused by interrupt (e.g.,
    CAN RX) */
    _disable_IRQ_interrupt_();

    /* Check the variables that carry the vector tables again before writting
    them into the memory buffer, which will be later dumped into a flash sector:
    if any member of the vector tables is zero, the next steps will be not executed. */
    for (uint8_t i_vector = 0u; i_vector < BOOT_NUM_OF_VECTOR_TABLE_8_BYTES; i_vector++) {
        if (boot_backupVectorTable.vectorTable[i_vector] == 0u) {
            gotoNext = false;
            retVal   = STD_NOT_OK;
            break;
        }
        if (boot_currentVectorTable.vectorTable[i_vector] == 0u) {
            gotoNext = false;
            retVal   = STD_NOT_OK;
            break;
        }
    }

    /* Copy the the magic number to RAM */
    if (gotoNext) {
        uint32_t boot_programInfoMagicNumer = BOOT_PROGRAM_INFO_MAGIC_NUM;
        memcpy(
            (uint8_t *)BOOT_SECTOR_BUFFER_MAGIC_NUMBER_START_ADDRESS,
            (uint8_t *)&boot_programInfoMagicNumer,
            BOOT_PROGRAM_INFO_SIZE_MAGIC_NUM);

        /* Copy the the info block to RAM */
        memcpy(
            (uint8_t *)BOOT_SECTOR_BUFFER_PROGRAM_INFO_START_ADDRESS,
            (uint8_t *)&boot_infoOfLastFlashedProgram,
            BOOT_PROGRAM_INFO_SIZE_IN_BYTES);

        /* Copy the backup vector table for bootloader to RAM */
        memcpy(
            (uint8_t *)BOOT_SECTOR_BUFFER_BACKUP_VECTOR_TABLE_START_ADDRESS,
            (uint64_t *)boot_backupVectorTable.vectorTable,
            BOOT_VECTOR_TABLE_SIZE);

        /* Copy the current vector table to RAM */
        memcpy(
            (uint8_t *)BOOT_SECTOR_BUFFER_VECTOR_TABLE_START_ADDRESS,
            (uint64_t *)boot_currentVectorTable.vectorTable,
            BOOT_VECTOR_TABLE_SIZE);
    }

    /* Write the sector buffer that carry the program info data struct,
     + backup vector table, current vector table to the sector that is assigned
     + as program info sector. */
    if (gotoNext) {
        if (FSYS_RaisePrivilegeToSystemMode() == STD_NOT_OK) {
            retVal   = STD_NOT_OK;
            gotoNext = false;
        }
    }

    if (gotoNext) {
        /* Disable IRQ interrupt before every function that will be run from
         * RAM */
        _disable_IRQ_interrupt_();
        uint8_t retValWriteFlash = FLASH_WriteFlashSector(
            (uint32_t *)BOOT_PROGRAM_INFO_SECTOR_ADDRESS,
            (uint8_t *)BOOT_SECTOR_BUFFER_START_ADDRESS,
            BOOT_PROGRAM_INFO_SECTOR_SIZE);
        FSYS_SwitchToUserMode();
        if (retValWriteFlash != 0u) {
            retVal = STD_NOT_OK;
        }
    }

    return retVal;
}

extern STD_RETURN_TYPE_e BOOT_ResetBootInfo(void) {
    STD_RETURN_TYPE_e retVal = STD_OK;

    /* Disable IRQ interrupt to prevent any issue caused by interrupt (e.g.,
     * CAN RX) */
    _disable_IRQ_interrupt_();

    /* Load program relevant information from flash to global variables
     * (boot_backupVectorTable, boot_currentVectorTable and
     * boot_infoOfLastFlashedProgram). */
    BOOT_LoadProgramInfoFromFlash();

    /* Reset the vector table using the backup table only if the program is
     * previously available, except for this situation, the current vector
     * table will be untouched. */
    if (boot_infoOfLastFlashedProgram.isProgramAvailable == BOOT_PROGRAM_IS_AVAILABLE) {
        /* If any part of the obtained backup vector table is 0, then the
         * vector table will not be recovered. */
        bool doesVectorTableContainZero = false;
        for (uint8_t i_vector = 0u; i_vector < BOOT_NUM_OF_VECTOR_TABLE_8_BYTES; i_vector++) {
            if (boot_backupVectorTable.vectorTable[i_vector] == 0u) {
                doesVectorTableContainZero = true;
                break;
            }
        }
        if (doesVectorTableContainZero == false) {
            for (uint8_t i_vector = 0u; i_vector < BOOT_NUM_OF_VECTOR_TABLE_8_BYTES; i_vector++) {
                boot_currentVectorTable.vectorTable[i_vector] = boot_backupVectorTable.vectorTable[i_vector];
            }
        }
    }

    /* Reset the relevant program info */
    boot_infoOfLastFlashedProgram.programLength        = 0u;
    boot_infoOfLastFlashedProgram.programStartAddress  = 0u;
    boot_infoOfLastFlashedProgram.programCrc8Bytes     = 0u;
    boot_infoOfLastFlashedProgram.vectorTableCrc8Bytes = 0u;
    boot_infoOfLastFlashedProgram.isProgramAvailable   = 0u;

    /* Write the modified global variables (boot_backupVectorTable,
     * boot_currentVectorTable and boot_infoOfLastFlashedProgram) to their
     * corresponding addresses in flash. */
    if (BOOT_UpdateProgramInfoIntoFlash() == STD_NOT_OK) {
        retVal = STD_NOT_OK;
    }

    return retVal;
}

extern void BOOT_SoftwareResetMcu(void) {
    bool gotoNext = true;

    /* Disable IRQ interrupt to prevent any issue caused by interrupt (e.g.,
     * CAN RX) */
    _disable_IRQ_interrupt_();

    if (FSYS_RaisePrivilegeToSystemMode() == STD_NOT_OK) {
        gotoNext = false;
    }

    if (gotoNext) {
        /* Software reset according to docref:
         * SPNU563A-March 2018 p. 197 Table 2-64.*/
        BOOT_SYSTEM_REG1_SYSECR = BOOT_RESET_VALUE_TO_WRITE << BOOT_RESET_SHIFT_BITS_NUMBER;
        FSYS_SwitchToUserMode();
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
