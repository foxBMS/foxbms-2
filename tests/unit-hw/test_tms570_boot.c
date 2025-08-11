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
 * @file    test_tms570_boot.c
 * @author  foxBMS Team
 * @date    2023-08-01 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  BOOT
 *
 * @brief   Test the hardware dependent behavior of function in boot module
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "test_tms570_boot.h"

#include <string.h>

/*========== Macros and Definitions =========================================*/
/* Some macros for test data */
#define BOOT_TEST_INFO_PROGRAM_LENGTH                   (12u)
#define BOOT_TEST_INFO_PROGRAM_START_ADDRESS            (0x00220002u)
#define BOOT_TEST_INFO_PROGRAM_CRC_8_BYTES              (0x822298af55fad25eu)
#define BOOT_TEST_INFO_PROGRAM_VECTOR_TABLE_CRC_8_BYTES (0x322298af55fad25fu)
#define BOOT_TEST_INFO_PROGRAM_IS_PROGRAM_AVAILABLE     (BOOT_PROGRAM_IS_AVAILABLE)
#define BOOT_TEST_INFO_PROGRAM_IS_PROGRAM_NOT_AVAILABLE (0u)
#define BOOT_TEST_VECTOR_TABLE_PATTERN                  (0xAAu)

/* Macros for some general error code */
#define FLASH_RETURN_NO_ERROR (0u)
#define FLASH_RETURN_ERROR_1  (1u)
#define FLASH_RETURN_ERROR_2  (2u)
#define FLASH_RETURN_ERROR_3  (3u)
#define FLASH_RETURN_ERROR_4  (4u)
#define FLASH_RETURN_ERROR_5  (5u)
#define FLASH_RETURN_ERROR_6  (6u)
#define FLASH_RETURN_ERROR_7  (7u)
#define FLASH_RETURN_ERROR_8  (8u)
#define FLASH_RETURN_ERROR_9  (9u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
uint8_t TEST_BOOT_UpdateAndLoadProgramInfo(void) {
    /* Load the program info from flash to variable */
    BOOT_LoadProgramInfoFromFlash();

    /* Write random value into the sector buffer*/
    memset((uint8_t *)BOOT_SECTOR_BUFFER_START_ADDRESS, 10u, BOOT_SECTOR_BUFFER_SIZE);

    /* Prepare the variable that contains the program info */
    boot_infoOfLastFlashedProgram.programLength        = BOOT_TEST_INFO_PROGRAM_LENGTH;
    boot_infoOfLastFlashedProgram.programStartAddress  = BOOT_TEST_INFO_PROGRAM_START_ADDRESS;
    boot_infoOfLastFlashedProgram.programCrc8Bytes     = BOOT_TEST_INFO_PROGRAM_CRC_8_BYTES;
    boot_infoOfLastFlashedProgram.vectorTableCrc8Bytes = BOOT_TEST_INFO_PROGRAM_VECTOR_TABLE_CRC_8_BYTES;
    boot_infoOfLastFlashedProgram.isProgramAvailable   = BOOT_TEST_INFO_PROGRAM_IS_PROGRAM_AVAILABLE;

    /* Update the program info into flash */
    if (BOOT_UpdateProgramInfoIntoFlash() != STD_OK) {
        return FLASH_RETURN_ERROR_1;
    }

    /* Reset the variable that contains the program info */
    boot_infoOfLastFlashedProgram.programLength        = 0u;
    boot_infoOfLastFlashedProgram.programStartAddress  = 0u;
    boot_infoOfLastFlashedProgram.programCrc8Bytes     = 0u;
    boot_infoOfLastFlashedProgram.vectorTableCrc8Bytes = 0u;
    boot_infoOfLastFlashedProgram.isProgramAvailable   = 0u;
    boot_backupVectorTable.vectorTable[0u]             = 0u;
    boot_backupVectorTable.vectorTable[1u]             = 0u;
    boot_backupVectorTable.vectorTable[2u]             = 0u;
    boot_backupVectorTable.vectorTable[3u]             = 0u;
    boot_currentVectorTable.vectorTable[0u]            = 0u;
    boot_currentVectorTable.vectorTable[1u]            = 0u;
    boot_currentVectorTable.vectorTable[2u]            = 0u;
    boot_currentVectorTable.vectorTable[3u]            = 0u;

    /* Load the program info from flash to variable */
    BOOT_LoadProgramInfoFromFlash();

    /* Check the results */
    if (boot_infoOfLastFlashedProgram.programLength != BOOT_TEST_INFO_PROGRAM_LENGTH) {
        return FLASH_RETURN_ERROR_2;
    }
    if (boot_infoOfLastFlashedProgram.programStartAddress != BOOT_TEST_INFO_PROGRAM_START_ADDRESS) {
        return FLASH_RETURN_ERROR_3;
    }
    if (boot_infoOfLastFlashedProgram.programCrc8Bytes != BOOT_TEST_INFO_PROGRAM_CRC_8_BYTES) {
        return FLASH_RETURN_ERROR_4;
    }
    if (boot_infoOfLastFlashedProgram.vectorTableCrc8Bytes != BOOT_TEST_INFO_PROGRAM_VECTOR_TABLE_CRC_8_BYTES) {
        return FLASH_RETURN_ERROR_5;
    }
    if (boot_infoOfLastFlashedProgram.isProgramAvailable != BOOT_TEST_INFO_PROGRAM_IS_PROGRAM_AVAILABLE) {
        return FLASH_RETURN_ERROR_6;
    }
    for (uint8_t iVectorTable = 0u; iVectorTable < BOOT_NUM_OF_VECTOR_TABLE_8_BYTES; iVectorTable++) {
        if (boot_currentVectorTable.vectorTable[iVectorTable] == 0u) {
            return FLASH_RETURN_ERROR_7;
        }
    }

    return FLASH_RETURN_NO_ERROR;
}

uint8_t TEST_BOOT_CleanUpRam(void) {
    *((uint8_t *)BOOT_SECTOR_BUFFER_START_ADDRESS) = 100u;
    BOOT_CleanUpRam();
    if (*((uint8_t *)BOOT_SECTOR_BUFFER_START_ADDRESS) != 0u) {
        return FLASH_RETURN_ERROR_1;
    }
    return FLASH_RETURN_NO_ERROR;
}

uint8_t TEST_BOOT_JumpInToLastFlashedProgram(void) {
    if (BOOT_JumpInToLastFlashedProgram() == STD_NOT_OK) {
        return FLASH_RETURN_ERROR_1;
    }
    return FLASH_RETURN_NO_ERROR;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
