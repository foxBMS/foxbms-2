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
 * @file    flash.h
 * @author  foxBMS Team
 * @date    2023-08-01 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  FLASH
 *
 * @brief   Header for the driver for the Flash module
 * @details TODO
 */

#ifndef FOXBMS__FLASH_H_
#define FOXBMS__FLASH_H_

/*========== Includes =======================================================*/
#include "flash_cfg.h"

#include "Types.h"

/*========== Macros and Definitions =========================================*/
/** The increment of the pU32 address each loop in FLASH_WriteFlash */
#define FLASH_PU32_ADDRESS_INCREASE_EACH_WRITE_LOOP (4u)

/** The size of the data array that contains uint8_t type of data and works
 * as buffer while writing to flash memory */
#define FLASH_DATA_BUFFER_ARRAY_TO_FLASH_SIZE (16u)

/** The index of the first flash sector where the application binary will be
written to */
#define FLASH_FIRST_SECTOR_FOR_APP_INDEX (7u)

/** The index of the last flash sector where the application binary will be
written to */
#define FLASH_LAST_SECTOR_FOR_APP_INDEX (31u)

/** Exception code */
/*@{*/
typedef enum {
    FLASH_EXCEPTION_CODE_NO_EXCEPTION,
    FLASH_EXCEPTION_INITIALIZATION_FAILED,
    FLASH_EXCEPTION_FLASH_SECTOR_INVALID,
    FLASH_EXCEPTION_WRONG_SECTOR_SIZE,
    FLASH_EXCEPTION_PREPARE_FLASH_BANK_FAILED,
    FLASH_EXCEPTION_ERASING_FLASH_SECTOR_FAILED,
    FLASH_EXCEPTION_WRITING_FLASH_SECTOR_FAILED,
    FLASH_EXCEPTION_FLASH_CHECK_FAILED,
} EXCEPTION_CODES_e;
/*@}*/

/** Address of the sector buffer that will be used to store the value to write to the flash sector  */
#define FLASH_SECTOR_BUFFER_START_ADDRESS (0x08030000u)

/** Size of the sector buffer that will be used to store the value to write to the flash sector  */
#define FLASH_SECTOR_BUFFER_SIZE (0x40000u)

/** Registers to check the state of the FLASH controller */
/*@{*/
#ifndef UNITY_UNIT_TEST
#define FLASH_FAPI_CHECK_FSM_READY_BUSY (FAPI_CHECK_FSM_READY_BUSY)
#define FLASH_FAPI_GET_FSM_STATUS       (FAPI_GET_FSM_STATUS)
#else
extern uint32_t FLASH_FAPI_CHECK_FSM_READY_BUSY;
extern uint32_t FLASH_FAPI_GET_FSM_STATUS;
#endif
/*@}*/

#ifdef UNITY_UNIT_TEST
extern bool FLASH_WriteFlashSector_Return[FLASH_NUM_OF_FLASH_SECTORS];
#endif

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   Activate the flash bank and enable the relevant sectors in that
 *          flash bank
 * @param   flashBankNumber Fapi_FlashBankType
 * @return  true if success
 */
bool FLASH_PrepareFlashBank(Fapi_FlashBankType flashBankNumber);

/**
 * @brief   Erase Sector of a flash Bank
 * @param[in] pU32StartAddressSector Start address of this Flash bank
 * @return  true if success
 */
bool FLASH_EraseFlashSector(uint32_t *pU32StartAddressSector);

/**
 * @brief   Write to flash
 * @param[in] pU32StartAddress        start address in flash for the data and
 *                                    ECC to be programmed
 * @param[in] pU8DataBuffer           pointer to the data buffer address
 * @param[in] u8DataBufferSizeInBytes number of bytes in the Data buffer
 * @return  true if success
 */
bool FLASH_WriteFlashWithEccAutoGeneration(
    uint32_t *pU32StartAddress,
    uint8_t *pU8DataBuffer,
    uint8_t u8DataBufferSizeInBytes);

/**
 * @brief   Check the programmed area
 * @param[in] pU32StartAddress        start address in flash from where the
 *                                    data has been flashed
 * @param[in] pU8DataBuffer           pointer to the data buffer address
 * @param[in] u8DataBufferSizeInBytes number of bytes in the data buffer
 * @return  true if success
 */
bool FLASH_FlashProgramCheck(uint32_t *pU32StartAddress, uint8_t *pU8DataBuffer, uint32_t u8DataBufferSizeInBytes);

/**
 * @brief   Get the sector based on the input flash sector start address
 * @param   pkU32FlashSectorStartAddress Pointer that points to the start
 *                                       address of flash sector
 * @return  FLASH_FLASH_SECTOR_s type of sector
 */
FLASH_FLASH_SECTOR_s FLASH_GetFlashSector(const uint32_t *pkU32FlashSectorStartAddress);

/**
 * @brief   Erase the app relevant flash sectors and calculate the ECC for them
 * @details Erased flash sectors will be filled with 0xFF.
 * @return  true if success
 */
bool FLASH_EraseFlashForApp(void);

/**
 * @brief   Write the data to flash
 * @param[in] pU32SectorStartAddress start address of a flash sector
 * @param[in] pU8DataBuffer          pointer to the data buffer address
 * @param[in] sectorSize             size of this flash sector
 * @return  0 if success
 */
uint8_t FLASH_WriteFlashSector(uint32_t *pU32SectorStartAddress, uint8_t *pU8DataBuffer, uint32_t sectorSize);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__FLASH_H_ */
