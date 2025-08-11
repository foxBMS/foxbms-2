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
 * @file    flash_cfg.c
 * @author  foxBMS Team
 * @date    2023-08-01 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  FLASH
 *
 * @brief   Implementation of Flash configuration
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "flash_cfg.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/* All variables defined in this file need to be constants as only constants in
 * are loaded in the RAM. See src/bootloader/main/bootloader.cmd for details
 */

/* AXIVION Disable Style MisraC2012-11.4: it is intended to convert uint to pointer */
const FLASH_FLASH_BANK_s flash_kFlashBank0 = {FLASH_FLASH, Fapi_FlashBank0, (uint32_t *)NULL, (uint8_t *)0x001FFFFFu};
const FLASH_FLASH_BANK_s flash_kFlashBank1 =
    {FLASH_FLASH, Fapi_FlashBank1, (uint32_t *)0x00200000u, (uint8_t *)0x003FFFFFu};
const FLASH_FLASH_BANK_s flash_kFlashBank7 =
    {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu};
const FLASH_FLASH_SECTOR_s flash_kFlashSectorsInvalid =
    {false, 0u, {FLASH_FLASH, Fapi_FlashBank0, NULL, NULL}, NULL, NULL};
#ifndef UNITY_UNIT_TEST
const FLASH_FLASH_SECTOR_s flash_kFlashSectors[FLASH_NUM_OF_FLASH_SECTORS] = {
    {true,
     0u,
     {FLASH_FLASH, Fapi_FlashBank0, (uint32_t *)NULL, (uint8_t *)0x001FFFFFu},
     (uint32_t *)NULL,
     (uint8_t *)0x00003FFFu}, /* sector 0 on flash bank 0 */
    {true,
     1u,
     {FLASH_FLASH, Fapi_FlashBank0, (uint32_t *)NULL, (uint8_t *)0x001FFFFFu},
     (uint32_t *)0x00004000u,
     (uint8_t *)0x00007FFFu}, /* sector 1 on flash bank 0 */
    {true,
     2u,
     {FLASH_FLASH, Fapi_FlashBank0, (uint32_t *)NULL, (uint8_t *)0x001FFFFFu},
     (uint32_t *)0x00008000u,
     (uint8_t *)0x0000BFFFu}, /* sector 2 on flash bank 0 */
    {true,
     3u,
     {FLASH_FLASH, Fapi_FlashBank0, (uint32_t *)NULL, (uint8_t *)0x001FFFFFu},
     (uint32_t *)0x0000C000u,
     (uint8_t *)0x0000FFFFu}, /* sector 3 on flash bank 0 */
    {true,
     4u,
     {FLASH_FLASH, Fapi_FlashBank0, (uint32_t *)NULL, (uint8_t *)0x001FFFFFu},
     (uint32_t *)0x00010000u,
     (uint8_t *)0x00013FFFu}, /* sector 4 on flash bank 0 */
    {true,
     5u,
     {FLASH_FLASH, Fapi_FlashBank0, (uint32_t *)NULL, (uint8_t *)0x001FFFFFu},
     (uint32_t *)0x00014000u,
     (uint8_t *)0x00017FFFu}, /* sector 5 on flash bank 0 */
    {true,
     6u,
     {FLASH_FLASH, Fapi_FlashBank0, (uint32_t *)NULL, (uint8_t *)0x001FFFFFu},
     (uint32_t *)0x00018000u,
     (uint8_t *)0x0001FFFFu}, /* sector 6 on flash bank 0 */
    {true,
     7u,
     {FLASH_FLASH, Fapi_FlashBank0, (uint32_t *)NULL, (uint8_t *)0x001FFFFFu},
     (uint32_t *)0x00020000u,
     (uint8_t *)0x0003FFFFu}, /* sector 7 on flash bank 0 */
    {true,
     8u,
     {FLASH_FLASH, Fapi_FlashBank0, (uint32_t *)NULL, (uint8_t *)0x001FFFFFu},
     (uint32_t *)0x00040000u,
     (uint8_t *)0x0005FFFFu}, /* sector 8 on flash bank 0 */
    {true,
     9u,
     {FLASH_FLASH, Fapi_FlashBank0, (uint32_t *)NULL, (uint8_t *)0x001FFFFFu},
     (uint32_t *)0x00060000u,
     (uint8_t *)0x0007FFFFu}, /* sector 9 on flash bank 0 */
    {true,
     10u,
     {FLASH_FLASH, Fapi_FlashBank0, (uint32_t *)NULL, (uint8_t *)0x001FFFFFu},
     (uint32_t *)0x00080000u,
     (uint8_t *)0x000BFFFFu}, /* sector 10 on flash bank 0 */
    {true,
     11u,
     {FLASH_FLASH, Fapi_FlashBank0, (uint32_t *)NULL, (uint8_t *)0x001FFFFFu},
     (uint32_t *)0x000C0000u,
     (uint8_t *)0x000FFFFFu}, /* sector 11 on flash bank 0 */
    {true,
     12u,
     {FLASH_FLASH, Fapi_FlashBank0, (uint32_t *)NULL, (uint8_t *)0x001FFFFFu},
     (uint32_t *)0x00100000u,
     (uint8_t *)0x0013FFFFu}, /* sector 12 on flash bank 0 */
    {true,
     13u,
     {FLASH_FLASH, Fapi_FlashBank0, (uint32_t *)NULL, (uint8_t *)0x001FFFFFu},
     (uint32_t *)0x00140000u,
     (uint8_t *)0x0017FFFFu}, /* sector 13 on flash bank 0 */
    {true,
     14u,
     {FLASH_FLASH, Fapi_FlashBank0, (uint32_t *)NULL, (uint8_t *)0x001FFFFFu},
     (uint32_t *)0x00180000u,
     (uint8_t *)0x001BFFFFu}, /* sector 14 on flash bank 0 */
    {true,
     15u,
     {FLASH_FLASH, Fapi_FlashBank0, (uint32_t *)NULL, (uint8_t *)0x001FFFFFu},
     (uint32_t *)0x001C0000u,
     (uint8_t *)0x001FFFFFu}, /* sector 15 on flash bank 0 */
    {true,
     16u,
     {FLASH_FLASH, Fapi_FlashBank1, (uint32_t *)0x00200000u, (uint8_t *)0x003FFFFFu},
     (uint32_t *)0x00200000u,
     (uint8_t *)0x0021FFFFu}, /* sector 0 on flash bank 1 */
    {true,
     17u,
     {FLASH_FLASH, Fapi_FlashBank1, (uint32_t *)0x00200000u, (uint8_t *)0x003FFFFFu},
     (uint32_t *)0x00220000u,
     (uint8_t *)0x0023FFFFu}, /* sector 1 on flash bank 1 */
    {true,
     18u,
     {FLASH_FLASH, Fapi_FlashBank1, (uint32_t *)0x00200000u, (uint8_t *)0x003FFFFFu},
     (uint32_t *)0x00240000u,
     (uint8_t *)0x0025FFFFu}, /* sector 2 on flash bank 1 */
    {true,
     19u,
     {FLASH_FLASH, Fapi_FlashBank1, (uint32_t *)0x00200000u, (uint8_t *)0x003FFFFFu},
     (uint32_t *)0x00260000u,
     (uint8_t *)0x0027FFFFu}, /* sector 3 on flash bank 1 */
    {true,
     20u,
     {FLASH_FLASH, Fapi_FlashBank1, (uint32_t *)0x00200000u, (uint8_t *)0x003FFFFFu},
     (uint32_t *)0x00280000u,
     (uint8_t *)0x0029FFFFu}, /* sector 4 on flash bank 1 */
    {true,
     21u,
     {FLASH_FLASH, Fapi_FlashBank1, (uint32_t *)0x00200000u, (uint8_t *)0x003FFFFFu},
     (uint32_t *)0x002A0000u,
     (uint8_t *)0x002BFFFFu}, /* sector 5 on flash bank 1 */
    {true,
     22u,
     {FLASH_FLASH, Fapi_FlashBank1, (uint32_t *)0x00200000u, (uint8_t *)0x003FFFFFu},
     (uint32_t *)0x002C0000u,
     (uint8_t *)0x002DFFFFu}, /* sector 6 on flash bank 1 */
    {true,
     23u,
     {FLASH_FLASH, Fapi_FlashBank1, (uint32_t *)0x00200000u, (uint8_t *)0x003FFFFFu},
     (uint32_t *)0x002E0000u,
     (uint8_t *)0x002FFFFFu}, /* sector 7 on flash bank 1 */
    {true,
     24u,
     {FLASH_FLASH, Fapi_FlashBank1, (uint32_t *)0x00200000u, (uint8_t *)0x003FFFFFu},
     (uint32_t *)0x00300000u,
     (uint8_t *)0x0031FFFFu}, /* sector 8 on flash bank 1 */
    {true,
     25u,
     {FLASH_FLASH, Fapi_FlashBank1, (uint32_t *)0x00200000u, (uint8_t *)0x003FFFFFu},
     (uint32_t *)0x00320000u,
     (uint8_t *)0x0033FFFFu}, /* sector 9 on flash bank 1 */
    {true,
     26u,
     {FLASH_FLASH, Fapi_FlashBank1, (uint32_t *)0x00200000u, (uint8_t *)0x003FFFFFu},
     (uint32_t *)0x00340000u,
     (uint8_t *)0x0035FFFFu}, /* sector 10 on flash bank 1 */
    {true,
     27u,
     {FLASH_FLASH, Fapi_FlashBank1, (uint32_t *)0x00200000u, (uint8_t *)0x003FFFFFu},
     (uint32_t *)0x00360000u,
     (uint8_t *)0x0037FFFFu}, /* sector 11 on flash bank 1 */
    {true,
     28u,
     {FLASH_FLASH, Fapi_FlashBank1, (uint32_t *)0x00200000u, (uint8_t *)0x003FFFFFu},
     (uint32_t *)0x00380000u,
     (uint8_t *)0x0039FFFFu}, /* sector 12 on flash bank 1 */
    {true,
     29u,
     {FLASH_FLASH, Fapi_FlashBank1, (uint32_t *)0x00200000u, (uint8_t *)0x003FFFFFu},
     (uint32_t *)0x003A0000u,
     (uint8_t *)0x003BFFFFu}, /* sector 13 on flash bank 1 */
    {true,
     30u,
     {FLASH_FLASH, Fapi_FlashBank1, (uint32_t *)0x00200000u, (uint8_t *)0x003FFFFFu},
     (uint32_t *)0x003C0000u,
     (uint8_t *)0x003DFFFFu}, /* sector 14 on flash bank 1 */
    {true,
     31u,
     {FLASH_FLASH, Fapi_FlashBank1, (uint32_t *)0x00200000u, (uint8_t *)0x003FFFFFu},
     (uint32_t *)0x003E0000u,
     (uint8_t *)0x003FFFFFu}, /* sector 15 on flash bank 1 */
    {true,
     32u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF0200000u,
     (uint8_t *)0xF0200FFFu}, /* sector 0 on flash bank 7 */
    {true,
     33u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF0201000u,
     (uint8_t *)0xF0201FFFu}, /* sector 1 on flash bank 7 */
    {true,
     34u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF0202000u,
     (uint8_t *)0xF0202FFFu}, /* sector 2 on flash bank 7 */
    {true,
     35u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF0203000u,
     (uint8_t *)0xF0203FFFu}, /* sector 3 on flash bank 7 */
    {true,
     36u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF0204000u,
     (uint8_t *)0xF0204FFFu}, /* sector 4 on flash bank 7 */
    {true,
     37u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF0205000u,
     (uint8_t *)0xF0205FFFu}, /* sector 5 on flash bank 7 */
    {true,
     38u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF0206000u,
     (uint8_t *)0xF0206FFFu}, /* sector 6 on flash bank 7 */
    {true,
     39u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF0207000u,
     (uint8_t *)0xF0207FFFu}, /* sector 7 on flash bank 7 */
    {true,
     40u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF0208000u,
     (uint8_t *)0xF0208FFFu}, /* sector 8 on flash bank 7 */
    {true,
     41u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF0209000u,
     (uint8_t *)0xF0209FFFu}, /* sector 9 on flash bank 7 */
    {true,
     42u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF020A000u,
     (uint8_t *)0xF020AFFFu}, /* sector 10 on flash bank 7 */
    {true,
     43u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF020B000u,
     (uint8_t *)0xF020BFFFu}, /* sector 11 on flash bank 7 */
    {true,
     44u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF020C000u,
     (uint8_t *)0xF020CFFFu}, /* sector 12 on flash bank 7 */
    {true,
     45u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF020D000u,
     (uint8_t *)0xF020DFFFu}, /* sector 13 on flash bank 7 */
    {true,
     46u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF020E000u,
     (uint8_t *)0xF020EFFFu}, /* sector 14 on flash bank 7 */
    {true,
     47u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF020F000u,
     (uint8_t *)0xF020FFFFu}, /* sector 15 on flash bank 7 */
    {true,
     48u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF0210000u,
     (uint8_t *)0xF0210FFFu}, /* sector 16 on flash bank 7 */
    {true,
     49u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF0211000u,
     (uint8_t *)0xF0211FFFu}, /* sector 17 on flash bank 7 */
    {true,
     50u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF0212000u,
     (uint8_t *)0xF0212FFFu}, /* sector 18 on flash bank 7 */
    {true,
     51u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF0213000u,
     (uint8_t *)0xF0213FFFu}, /* sector 19 on flash bank 7 */
    {true,
     52u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF0214000u,
     (uint8_t *)0xF0214FFFu}, /* sector 20 on flash bank 7 */
    {true,
     53u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF0215000u,
     (uint8_t *)0xF0215FFFu}, /* sector 21 on flash bank 7 */
    {true,
     54u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF0216000u,
     (uint8_t *)0xF0216FFFu}, /* sector 22 on flash bank 7 */
    {true,
     55u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF0217000u,
     (uint8_t *)0xF0217FFFu}, /* sector 23 on flash bank 7 */
    {true,
     56u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF0218000u,
     (uint8_t *)0xF0218FFFu}, /* sector 24 on flash bank 7 */
    {true,
     57u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF0219000u,
     (uint8_t *)0xF0219FFFu}, /* sector 25 on flash bank 7 */
    {true,
     58u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF021A000u,
     (uint8_t *)0xF021AFFFu}, /* sector 26 on flash bank 7 */
    {true,
     59u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF021B000u,
     (uint8_t *)0xF021BFFFu}, /* sector 27 on flash bank 7 */
    {true,
     60u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF021C000u,
     (uint8_t *)0xF021CFFFu}, /* sector 28 on flash bank 7 */
    {true,
     61u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF021D000u,
     (uint8_t *)0xF021DFFFu}, /* sector 29 on flash bank 7 */
    {true,
     62u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF021E000u,
     (uint8_t *)0xF021EFFFu}, /* sector 30 on flash bank 7 */
    {true,
     63u,
     {FLASH_EEPROM, Fapi_FlashBank7, (uint32_t *)0xF0200000u, (uint8_t *)0xF021FFFFu},
     (uint32_t *)0xF021F000u,
     (uint8_t *)0xF021FFFFu}, /* sector 31 on flash bank 7 */
};

#endif

/* AXIVION Enable Style MisraC2012-11.4 */

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void BOOT_NothingToTest(void) {
}
#endif
