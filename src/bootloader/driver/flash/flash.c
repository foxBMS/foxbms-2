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
 * @file    flash.c
 * @author  foxBMS Team
 * @date    2024-08-01 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  FLASH
 *
 * @brief   Implementation of Flash software
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "general.h"

#include "flash.h"

#include "flash_cfg.h"

#include "HL_sys_common.h"
#include "HL_system.h"

#include "Device_TMS570LC43.h"
#include "F021.h"
#include "FapiFunctions.h"
#include "Types.h"
#include "fassert.h"
#include "fstring.h"
#include "infinite-loop-helper.h"

#include <stdint.h>
#include <stdlib.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

#if !defined(UNITY_UNIT_TEST)
Fapi_StatusType Fapi_serviceWatchdogTimer(void) { /* LCOV_EXCL_LINE */
    /* User to add their own watchdog servicing code here */
    return (Fapi_Status_Success); /* LCOV_EXCL_LINE */
}
#endif

FLASH_FLASH_SECTOR_s FLASH_GetFlashSector(const uint32_t *pkU32FlashSectorStartAddress) {
    /* The address range of the flash memory is from 0u to 0x003FFFFFu;
        The address range of the EEPROM is from 0xF0200000u to 0xF021FFFFu */
    FAS_ASSERT(
        (pkU32FlashSectorStartAddress <= (uint32_t *)0x003FFFFFu) ||
        ((pkU32FlashSectorStartAddress >= (uint32_t *)0xF0200000u) &&
         (pkU32FlashSectorStartAddress <= (uint32_t *)0xF021FFFFu)));

    FLASH_FLASH_SECTOR_s currentSector = flash_kFlashSectorsInvalid;
    for (uint8_t iFlashSector = 0u; iFlashSector < FLASH_NUM_OF_FLASH_SECTORS; iFlashSector++) {
        if (pkU32FlashSectorStartAddress == flash_kFlashSectors[iFlashSector].pU32SectorAddressStart) {
            currentSector = flash_kFlashSectors[iFlashSector];
        }
    }
    return currentSector;
}

bool FLASH_PrepareFlashBank(Fapi_FlashBankType flashBankNumber) {
    bool isSuccessful     = false;
    bool isEnabledSectors = false;

    FAS_ASSERT(
        (flashBankNumber == Fapi_FlashBank0) || (flashBankNumber == Fapi_FlashBank1) ||
        (flashBankNumber == Fapi_FlashBank7));

    /* Sets the active bank for a erase or program command */
    if (Fapi_setActiveFlashBank(flashBankNumber) == Fapi_Status_Success) {
        /* Enable the bank sectors based on the chosen bank */
        if (flashBankNumber == Fapi_FlashBank7) {
            /* Enabling All 32 sectors of EEPROM */
            if (Fapi_enableEepromBankSectors(FLASH_FULL_32_BITS, 0u) == Fapi_Status_Success) {
                isEnabledSectors = true;
            }
        } else {
            /* Enabling 16 sectors of FLASH */
            if (Fapi_enableMainBankSectors(FLASH_FULL_16_BITS) == Fapi_Status_Success) {
                isEnabledSectors = true;
            }
        }
    }
    if (isEnabledSectors == true) {
        while (FOREVER()) {
            if (FLASH_FAPI_CHECK_FSM_READY_BUSY == Fapi_Status_FsmReady) {
                isSuccessful = true;
                break;
            }
        }
    }

    return isSuccessful;
}

bool FLASH_EraseFlashSector(uint32_t *pU32StartAddressSector) {
    FAS_ASSERT(pU32StartAddressSector != NULL_PTR);

    bool isSuccessfulErase = false;
    if (Fapi_issueAsyncCommandWithAddress(Fapi_EraseSector, pU32StartAddressSector) == Fapi_Status_Success) {
        while (FOREVER()) {
            if (FLASH_FAPI_CHECK_FSM_READY_BUSY == Fapi_Status_FsmReady) {
                break;
            }
        }
        if (FLASH_FAPI_GET_FSM_STATUS == Fapi_Status_Success) {
            isSuccessfulErase = true;
        }
    }

    return isSuccessfulErase;
}

bool FLASH_FlashProgramCheck(uint32_t *pU32StartAddress, uint8_t *pU8DataBuffer, uint32_t u8DataBufferSizeInBytes) {
    FAS_ASSERT(pU32StartAddress != NULL_PTR);
    FAS_ASSERT(pU8DataBuffer != NULL_PTR);
    FAS_ASSERT(u8DataBufferSizeInBytes != 0u);

    bool isSuccessfullyProgrammed               = true;
    register const uint8_t *pkU8StartAddressRr  = (uint8_t *)pU32StartAddress;
    register const uint8_t *pkU8DataBufferRr    = pU8DataBuffer;
    register uint32_t u8DataBufferSizeInBytesRr = u8DataBufferSizeInBytes;
    uint8_t destData                            = *pkU8StartAddressRr;
    uint8_t bufferData                          = *pkU8DataBufferRr;
    while (u8DataBufferSizeInBytesRr > 0u) {
        destData   = *pkU8StartAddressRr;
        bufferData = *pkU8DataBufferRr;
        if (destData != bufferData) {
            isSuccessfullyProgrammed = false;
            break;
        }
        pkU8StartAddressRr++;
        pkU8DataBufferRr++;
        u8DataBufferSizeInBytesRr--;
    }

    return isSuccessfullyProgrammed;
}

bool FLASH_WriteFlashWithEccAutoGeneration(
    uint32_t *pU32StartAddress,
    uint8_t *pU8DataBuffer,
    uint8_t u8DataBufferSizeInBytes) {
    FAS_ASSERT(pU32StartAddress != NULL_PTR);
    FAS_ASSERT(pU8DataBuffer != NULL_PTR);
    FAS_ASSERT(u8DataBufferSizeInBytes != 0u);
    bool isSuccessfulWrite = true;
    bool gotoNext          = true;

    while (FOREVER()) {
        if (FLASH_FAPI_CHECK_FSM_READY_BUSY == Fapi_Status_FsmReady) {
            break;
        }
    }
    Fapi_StatusType retVal = Fapi_issueProgrammingCommand(
        pU32StartAddress, pU8DataBuffer, u8DataBufferSizeInBytes, NULL_PTR, 0u, Fapi_AutoEccGeneration);
    if (retVal != Fapi_Status_Success) {
        isSuccessfulWrite = false;
        gotoNext          = false;
    }

    if (gotoNext) {
        while (FOREVER()) {
            if (FLASH_FAPI_CHECK_FSM_READY_BUSY == Fapi_Status_FsmReady) {
                break;
            }
        }
        while (FOREVER()) {
            if (FLASH_FAPI_GET_FSM_STATUS == Fapi_Status_Success) {
                break;
            }
        }
    }

    return isSuccessfulWrite;
}

bool FLASH_EraseFlashForApp(void) {
    bool retVal = true;
    memset((uint8_t *)FLASH_SECTOR_BUFFER_START_ADDRESS, 0, FLASH_SECTOR_BUFFER_SIZE);
    for (uint8_t iSector = FLASH_FIRST_SECTOR_FOR_APP_INDEX; iSector <= FLASH_LAST_SECTOR_FOR_APP_INDEX; iSector++) {
#ifndef UNITY_UNIT_TEST
        uint32_t sectorSize = (uint32_t)(flash_kFlashSectors[iSector].pU8SectorAddressEnd + 1u) -
                              (uint32_t)flash_kFlashSectors[iSector].pU32SectorAddressStart;
        if (FLASH_WriteFlashSector(
                flash_kFlashSectors[iSector].pU32SectorAddressStart,
                (uint8_t *)FLASH_SECTOR_BUFFER_START_ADDRESS,
                sectorSize) != FLASH_EXCEPTION_CODE_NO_EXCEPTION)
#else
        /* Mock the return of the FLASH_WriteFlashSector */
        if (FLASH_WriteFlashSector_Return[iSector] == false)
#endif
        {
            retVal = false;
            break;
        }
    }
    return retVal;
}

uint8_t FLASH_WriteFlashSector(uint32_t *pU32SectorStartAddress, uint8_t *pU8DataBuffer, uint32_t sectorSize) {
    FAS_ASSERT(pU32SectorStartAddress != NULL_PTR);
    FAS_ASSERT(pU8DataBuffer != NULL_PTR);
    FAS_ASSERT(sectorSize != 0u);

    uint8_t retVal                          = FLASH_EXCEPTION_CODE_NO_EXCEPTION;
    bool gotoNext                           = true;
    FLASH_FLASH_SECTOR_s currentFlashSector = flash_kFlashSectorsInvalid;
    FLASH_FLASH_BANK_s currentFlashBank     = {FLASH_FLASH, Fapi_FlashBank0, NULL, NULL};
    uint32_t *pU32FlashAddress              = NULL;
    uint8_t *pU8DataBufferToWrite           = NULL;

    /* Initialize the flash memory controller before using any asynchronous
     * command, program or read functions. */
    Fapi_StatusType isFlashBanksInitialized = Fapi_initializeFlashBanks((uint32_t)FLASH_SYS_CLK_FREQ);
    if (isFlashBanksInitialized != Fapi_Status_Success) {
        retVal   = FLASH_EXCEPTION_INITIALIZATION_FAILED;
        gotoNext = false;
    }

    /* Get the sector based on the input sector start address and check whether
     * it is valid.  */
    if (gotoNext) {
        currentFlashSector = FLASH_GetFlashSector(pU32SectorStartAddress);
        if (currentFlashSector.isThisSectorValid == false) {
            retVal   = FLASH_EXCEPTION_FLASH_SECTOR_INVALID;
            gotoNext = false;
        }
    }

    /* Check the sector size (the reason to leave the sector size as one of the
     * input argument is to provide the functionality to verify the sector
     * size, in case the sector size is also used in other function). */
    if (gotoNext) {
        uint32_t currentSectorSize = (uint32_t)(currentFlashSector.pU8SectorAddressEnd + 1u) -
                                     (uint32_t)(currentFlashSector.pU32SectorAddressStart);
        if (sectorSize != currentSectorSize) {
            retVal   = FLASH_EXCEPTION_WRONG_SECTOR_SIZE;
            gotoNext = false;
        }
    }

    /* Based on the chosen sector, get its flash bank and prepare this flash
     * bank. */
    if (gotoNext) {
        currentFlashBank = currentFlashSector.flashBank;
        /* Prepare the current flash bank (activate, enable all relevant
         * sectors) */
        bool isSuccessfulPrepareFlashBank = FLASH_PrepareFlashBank(currentFlashBank.flashBankFapi);
        if (isSuccessfulPrepareFlashBank == false) {
            retVal   = FLASH_EXCEPTION_PREPARE_FLASH_BANK_FAILED;
            gotoNext = false;
        }
    }

    /* Erase the whole flash sector */
    if (gotoNext) {
        if (FLASH_EraseFlashSector(currentFlashSector.pU32SectorAddressStart) == false) {
            retVal   = FLASH_EXCEPTION_ERASING_FLASH_SECTOR_FAILED;
            gotoNext = false;
        }
    }

    /* Write this flash sector */
    if (gotoNext) {
        pU32FlashAddress     = currentFlashSector.pU32SectorAddressStart;
        pU8DataBufferToWrite = pU8DataBuffer;
        for (uint32_t iWrite = 0u; iWrite < sectorSize; iWrite += FLASH_DATA_BUFFER_ARRAY_TO_FLASH_SIZE) {
            if (FLASH_WriteFlashWithEccAutoGeneration(
                    pU32FlashAddress, pU8DataBufferToWrite, FLASH_DATA_BUFFER_ARRAY_TO_FLASH_SIZE) != true) {
                retVal   = FLASH_EXCEPTION_WRITING_FLASH_SECTOR_FAILED;
                gotoNext = false;
                break;
            }
            pU32FlashAddress += FLASH_PU32_ADDRESS_INCREASE_EACH_WRITE_LOOP;
            pU8DataBufferToWrite += FLASH_DATA_BUFFER_ARRAY_TO_FLASH_SIZE;
        }
    }

    /* Check this flash sector */
    if (gotoNext) {
        if (FLASH_FlashProgramCheck(pU32SectorStartAddress, pU8DataBuffer, sectorSize) != true) {
            retVal = FLASH_EXCEPTION_FLASH_CHECK_FAILED;
        }
    }

    return retVal;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
