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
 * @file    test_tms570_flash.c
 * @author  foxBMS Team
 * @date    2023-08-01 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  FLASH
 *
 * @brief   Test the hardware dependent behavior of function in flash module
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "test_tms570_flash.h"

#include "flash.h"

#include <string.h>

/*========== Macros and Definitions =========================================*/
/* Macros for some general error code */
#define FLASH_RETURN_NO_ERROR (0u)
#define FLASH_RETURN_ERROR_1  (1u)
#define FLASH_RETURN_ERROR_2  (2u)
#define FLASH_RETURN_ERROR_3  (3u)

/* Test sector start address */
#define FLASH_TEST_INVALID_FLASH_SECTOR_START_ADDRESS ((uint32_t *)0x00006000u)

/** Address of the sector buffer that will be used to store the value to write to the flash sector  */
#define FLASH_SECTOR_BUFFER_START_ADDRESS (0x08030000u)

/** Size of the sector buffer that will be used to store the value to write to the flash sector  */
#define FLASH_SECTOR_BUFFER_SIZE (0x40000u)

/** The index of the first flash sector where the application binary will be
written to */
#define FLASH_FIRST_SECTOR_FOR_APP_INDEX (7u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
uint8_t TEST_FLASH_GetFlashSector(void) {
    FLASH_FLASH_SECTOR_s sector = flash_kFlashSectorsInvalid;
    bool gotoNextTest           = true;
    uint8_t retVal              = FLASH_RETURN_NO_ERROR;

    /* Test getting valid sectors */
    for (uint8_t iFlashSector = 0u; iFlashSector < FLASH_NUM_OF_FLASH_SECTORS; iFlashSector++) {
        sector = FLASH_GetFlashSector(flash_kFlashSectors[iFlashSector].pU32SectorAddressStart);
        if (sector.isThisSectorValid != true) {
            retVal       = FLASH_RETURN_ERROR_1;
            gotoNextTest = false;
            break;
        }
        if (sector.idxOfThisSector != iFlashSector) {
            retVal       = FLASH_RETURN_ERROR_2;
            gotoNextTest = false;
            break;
        }
    }

    /* Test getting invalid sectors */
    if (gotoNextTest) {
        /* Test if the entered address is not a sector flash start address */
        sector = FLASH_GetFlashSector(FLASH_TEST_INVALID_FLASH_SECTOR_START_ADDRESS);
        if (sector.isThisSectorValid == true) {
            retVal = FLASH_RETURN_ERROR_3;
        }
    }

    return retVal;
}

uint8_t TEST_FLASH_PrepareFlashBank(void) {
    uint8_t retVal = FLASH_RETURN_NO_ERROR;
    if (FLASH_PrepareFlashBank(Fapi_FlashBank0) != true) {
        retVal = FLASH_RETURN_ERROR_1;
    }
    if (FLASH_PrepareFlashBank(Fapi_FlashBank1) != true) {
        retVal = FLASH_RETURN_ERROR_2;
    }
    if (FLASH_PrepareFlashBank(Fapi_FlashBank7) != true) {
        retVal = FLASH_RETURN_ERROR_3;
    }
    return retVal;
}

uint8_t TEST_FLASH_FlashProgramCheck(void) {
    uint8_t retVal = FLASH_RETURN_NO_ERROR;
    /* Prepare two RAM area with all 0x0A */
    uint8_t *pBufferStartAddress1 = (uint8_t *)FLASH_SECTOR_BUFFER_START_ADDRESS;
    uint8_t *pBufferStartAddress2 = (uint8_t *)(FLASH_SECTOR_BUFFER_START_ADDRESS + 0x1000u);
    uint32_t bufferSize           = 0x100u;
    uint8_t repeatedPattern       = 0xAu;
    memset(pBufferStartAddress1, repeatedPattern, bufferSize);
    memset(pBufferStartAddress2, repeatedPattern, bufferSize);
    if (FLASH_FlashProgramCheck((uint32_t *)pBufferStartAddress1, pBufferStartAddress2, bufferSize) != true) {
        retVal = FLASH_RETURN_ERROR_1;
    }
    return retVal;
}

uint8_t TEST_FLASH_EraseFlashForApp(void) {
    uint8_t retVal = FLASH_RETURN_NO_ERROR;
    memset((uint8_t *)FLASH_SECTOR_BUFFER_START_ADDRESS, 1u, FLASH_SECTOR_BUFFER_SIZE);
    if (FLASH_EraseFlashForApp() != true) {
        retVal = FLASH_RETURN_ERROR_1;
    }
    return retVal;
}

uint8_t TEST_FLASH_WriteFlashSector(void) {
    /* Prepare the buffer to write to flash */
    memset((uint8_t *)FLASH_SECTOR_BUFFER_START_ADDRESS, 0xABu, FLASH_SECTOR_BUFFER_SIZE);

    /* Write the values in the prepared sector buffer into flash sector 1 */
    return FLASH_WriteFlashSector(
        flash_kFlashSectors[FLASH_FIRST_SECTOR_FOR_APP_INDEX].pU32SectorAddressStart,
        (uint8_t *)FLASH_SECTOR_BUFFER_START_ADDRESS,
        (uint32_t)(flash_kFlashSectors[FLASH_FIRST_SECTOR_FOR_APP_INDEX].pU8SectorAddressEnd -
                   (uint8_t *)flash_kFlashSectors[FLASH_FIRST_SECTOR_FOR_APP_INDEX].pU32SectorAddressStart));
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
