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
 * @file    flash_cfg.h
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

#ifndef FOXBMS__FLASH_CFG_H_
#define FOXBMS__FLASH_CFG_H_

/*========== Includes =======================================================*/
#include "HL_sys_common.h"
#include "HL_system.h"

#include "F021.h"

/*========== Macros and Definitions =========================================*/
/** Define the number of sectors */
#define FLASH_NUM_OF_FLASH_SECTORS_BANK0 (16u)
#define FLASH_NUM_OF_FLASH_SECTORS_BANK1 (16u)
#define FLASH_NUM_OF_FLASH_SECTORS_BANK7 (32u)
#define FLASH_NUM_OF_FLASH_SECTORS \
    ((FLASH_NUM_OF_FLASH_SECTORS_BANK0) + (FLASH_NUM_OF_FLASH_SECTORS_BANK1) + (FLASH_NUM_OF_FLASH_SECTORS_BANK7))

/** Frequency to init the flash controller */
#define FLASH_SYS_CLK_FREQ ((uint32_t)HCLK_FREQ)

/** Define macro for full bits */
/*@{*/
#define FLASH_FULL_32_BITS (0xFFFFFFFFu)
#define FLASH_FULL_16_BITS (0xFFFFu)
/*@}*/

/** Enum to describe the type of memory */
typedef enum {
    FLASH_FLASH,  /*!< flash type of memory */
    FLASH_EEPROM, /*!< EEPROM type of memory */
} FLASH_MEMORY_TYPE_e;

/** Data struct to represent a flash bank */
typedef struct {
    FLASH_MEMORY_TYPE_e typeOfFlashBank; /*!< memory type of the flash bank */
    Fapi_FlashBankType flashBankFapi;    /*!< Fapi_FlashBankType of this flash bank */
    uint32_t *pU32BankAddressStart;      /*!< the (uint32_t *) address of the start of the flash bank */
    uint8_t *pU8BankAddressEnd;          /*!< the (uint8_t *) address of the end of the flash bank */
} FLASH_FLASH_BANK_s;

/** Data struct to represent a flash sector */
typedef struct {
    bool isThisSectorValid;  /*!< the validity of this sector: true - valid; false - invalid */
    uint8_t idxOfThisSector; /*!< index of this sector (0-63: 0-15 flash bank 0; 16-31 flash bank 1; 32-63 EEPROM) */
    FLASH_FLASH_BANK_s flashBank;     /*!< FLASH_FLASH_BANK_s of flash bank */
    uint32_t *pU32SectorAddressStart; /*!< (uint32_t *) start address of one sector */
    uint8_t *pU8SectorAddressEnd;     /*!< (uint8_t *) end address of one sector  */
} FLASH_FLASH_SECTOR_s;

/*========== Extern Constant and Variable Declarations ======================*/
/** Constant struct that matches the flash bank 0 */
extern const FLASH_FLASH_BANK_s flash_kFlashBank0;

/** Constant struct that matches the flash bank 1 */
extern const FLASH_FLASH_BANK_s flash_kFlashBank1;

/** Constant struct that matches the flash bank 7 */
extern const FLASH_FLASH_BANK_s flash_kFlashBank7;

/** Constant struct indicates an invalid flash sector */
extern const FLASH_FLASH_SECTOR_s flash_kFlashSectorsInvalid;

/** Constant struct that matches the flash sectors */
extern const FLASH_FLASH_SECTOR_s flash_kFlashSectors[FLASH_NUM_OF_FLASH_SECTORS];

/*========== Extern Function Prototypes =====================================*/

/*========== Unit Tests ===========*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern void BOOT_NothingToTest(void);
#endif

#endif /* FOXBMS__FLASH_CFG_H_ */
