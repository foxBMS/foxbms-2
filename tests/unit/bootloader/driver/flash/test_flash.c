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
 * @file    test_flash.c
 * @author  foxBMS Team
 * @date    2024-09-17 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of some module
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockFapiFunctions.h"
#include "Mockinfinite-loop-helper.h"

#include "flash_cfg.h"

#include "F021.h"
#include "Types.h"
#include "flash.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("flash.c")

TEST_INCLUDE_PATH("../../src/bootloader/driver/config")
TEST_INCLUDE_PATH("../../src/bootloader/driver/flash")
TEST_INCLUDE_PATH("../../src/bootloader/main/include")
TEST_INCLUDE_PATH("C:/ti/Hercules/F021 Flash API/02.01.01/include")

/*========== Definitions and Implementations for Unit Test ==================*/
#define TEST_MOCK_FLASH_SECTOR_SIZE  (128u)
#define TEST_MOCK_FLASH_SECTOR_INDEX (FLASH_FIRST_SECTOR_FOR_APP_INDEX)

bool FLASH_WriteFlashSector_Return[FLASH_NUM_OF_FLASH_SECTORS] = {0u};

uint32_t FLASH_FAPI_CHECK_FSM_READY_BUSY = 0u;
uint32_t FLASH_FAPI_GET_FSM_STATUS       = 0u;
const uint8_t mockFlashSector7Size       = TEST_MOCK_FLASH_SECTOR_SIZE;
uint8_t mockFlashSector7[TEST_MOCK_FLASH_SECTOR_SIZE];
const uint8_t mockFlashSector8Size = TEST_MOCK_FLASH_SECTOR_SIZE;
uint8_t mockFlashSector8[TEST_MOCK_FLASH_SECTOR_SIZE];
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
    /* mock the 7th sector using address of an uint8_t array */
    {true,
     7u,
     {FLASH_FLASH, Fapi_FlashBank0, (uint32_t *)NULL, (uint8_t *)0x001FFFFFu},
     (uint32_t *)mockFlashSector7,
     (uint8_t *)(mockFlashSector7 + mockFlashSector7Size - 1u)}, /* sector 7 on flash bank 0 */
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

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/**
 * @brief   Testing function FLASH_PrepareFlashBank
 * @details The following cases will be tested:
 *          - Argument validation:
 *            - AT1/1: Invalid flash bank &rarr; assert
 *          - Routine validation:
 *            - RT1/x: TODO
 */
void testFLASH_PrepareFlashBank(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1: Fapi_FlashBank2 cannot be used */
    TEST_ASSERT_FAIL_ASSERT(FLASH_PrepareFlashBank(Fapi_FlashBank2));

    /* ======= Routine tests =============================================== */
    bool retval = false;

    /* ======= RT1/4: Set active flash bank fails */
    Fapi_setActiveFlashBank_ExpectAndReturn(Fapi_FlashBank1, Fapi_Error_Fail);
    retval = FLASH_PrepareFlashBank(Fapi_FlashBank1);
    TEST_ASSERT_EQUAL(false, retval);

    /* ======= RT1/4: Prepare EEPROM, successful */
    Fapi_setActiveFlashBank_ExpectAndReturn(Fapi_FlashBank7, Fapi_Status_Success);
    Fapi_enableEepromBankSectors_ExpectAndReturn(FLASH_FULL_32_BITS, 0u, Fapi_Status_Success);
    FOREVER_ExpectAndReturn(1);
    FLASH_FAPI_CHECK_FSM_READY_BUSY = Fapi_Status_FsmReady;
    retval                          = FLASH_PrepareFlashBank(Fapi_FlashBank7);
    TEST_ASSERT_EQUAL(true, retval);

    /* ======= RT2/4: Prepare EEPROM, not successful */
    Fapi_setActiveFlashBank_ExpectAndReturn(Fapi_FlashBank7, Fapi_Status_Success);
    Fapi_enableEepromBankSectors_ExpectAndReturn(FLASH_FULL_32_BITS, 0u, Fapi_Error_Fail);
    retval = FLASH_PrepareFlashBank(Fapi_FlashBank7);
    TEST_ASSERT_EQUAL(false, retval);

    /* ======= RT3/4: Prepare normal flash bank Fapi_FlashBank0, successful */
    Fapi_setActiveFlashBank_ExpectAndReturn(Fapi_FlashBank0, Fapi_Status_Success);
    Fapi_enableMainBankSectors_ExpectAndReturn(FLASH_FULL_16_BITS, Fapi_Status_Success);
    FOREVER_ExpectAndReturn(1);
    FLASH_FAPI_CHECK_FSM_READY_BUSY = Fapi_Status_FsmReady;
    retval                          = FLASH_PrepareFlashBank(Fapi_FlashBank0);
    TEST_ASSERT_EQUAL(true, retval);

    /* ======= RT4/4: Prepare normal flash bank Fapi_FlashBank0, the status of FSM is always busy */
    Fapi_setActiveFlashBank_ExpectAndReturn(Fapi_FlashBank0, Fapi_Status_Success);
    Fapi_enableMainBankSectors_ExpectAndReturn(FLASH_FULL_16_BITS, Fapi_Status_Success);
    FOREVER_ExpectAndReturn(1);
    FLASH_FAPI_CHECK_FSM_READY_BUSY = Fapi_Status_FsmBusy;
    FOREVER_ExpectAndReturn(0);
    retval = FLASH_PrepareFlashBank(Fapi_FlashBank0);
    TEST_ASSERT_EQUAL(false, retval);

    /* ======= RT4/4: Prepare normal flash bank Fapi_FlashBank0, not successful */
    Fapi_setActiveFlashBank_ExpectAndReturn(Fapi_FlashBank0, Fapi_Status_Success);
    Fapi_enableMainBankSectors_ExpectAndReturn(FLASH_FULL_16_BITS, Fapi_Error_Fail);
    retval = FLASH_PrepareFlashBank(Fapi_FlashBank0);
    TEST_ASSERT_EQUAL(false, retval);
}

void testFLASH_WriteFlashWithEccAutoGeneration(void) {
    bool retval = false;

    /* Prepare a data buffer and a flash memory */
    const uint32_t dataSize = 128u;
    uint8_t dataBuffer[dataSize];
    uint8_t flashMemoryCorrect[dataSize];
    for (uint32_t iByte = 0u; iByte < dataSize; iByte++) {
        dataBuffer[iByte]         = 0xAA;
        flashMemoryCorrect[iByte] = 0xAA;
    }

    /* ======= AT1/3: Assertion tests: Data buffer address is null */
    TEST_ASSERT_FAIL_ASSERT(FLASH_WriteFlashWithEccAutoGeneration((uint32_t *)flashMemoryCorrect, NULL, dataSize));

    /* ======= AT2/3: Flash memory address is null */
    TEST_ASSERT_FAIL_ASSERT(FLASH_WriteFlashWithEccAutoGeneration(NULL, (uint8_t *)dataBuffer, dataSize));

    /* ======= AT3/3: Data buffer size in bytes is 0u */
    TEST_ASSERT_FAIL_ASSERT(
        FLASH_WriteFlashWithEccAutoGeneration((uint32_t *)flashMemoryCorrect, (uint8_t *)dataBuffer, 0u));

    /* ======= RT1/3: Write flash successfully */
    FOREVER_ExpectAndReturn(1);
    FLASH_FAPI_CHECK_FSM_READY_BUSY = Fapi_Status_FsmReady;
    Fapi_issueProgrammingCommand_ExpectAndReturn(
        (uint32_t *)flashMemoryCorrect,
        (uint8_t *)dataBuffer,
        dataSize,
        NULL,
        0u,
        Fapi_AutoEccGeneration,
        Fapi_Status_Success);
    FOREVER_ExpectAndReturn(1);
    FLASH_FAPI_CHECK_FSM_READY_BUSY = Fapi_Status_FsmReady;
    FOREVER_ExpectAndReturn(1);
    FLASH_FAPI_GET_FSM_STATUS = Fapi_Status_Success;
    retval = FLASH_WriteFlashWithEccAutoGeneration((uint32_t *)flashMemoryCorrect, (uint8_t *)dataBuffer, dataSize);
    TEST_ASSERT_EQUAL(true, retval);

    /* ======= RT2/3: Write flash successfully, but the status is not ready */
    FLASH_FAPI_CHECK_FSM_READY_BUSY = Fapi_Status_FsmBusy;
    FLASH_FAPI_GET_FSM_STATUS       = Fapi_Error_Fail;
    FOREVER_ExpectAndReturn(1);
    FOREVER_ExpectAndReturn(0);
    Fapi_issueProgrammingCommand_ExpectAndReturn(
        (uint32_t *)flashMemoryCorrect,
        (uint8_t *)dataBuffer,
        dataSize,
        NULL,
        0u,
        Fapi_AutoEccGeneration,
        Fapi_Status_Success);
    FOREVER_ExpectAndReturn(1);
    FOREVER_ExpectAndReturn(0);
    FOREVER_ExpectAndReturn(1);
    FOREVER_ExpectAndReturn(0);

    retval = FLASH_WriteFlashWithEccAutoGeneration((uint32_t *)flashMemoryCorrect, (uint8_t *)dataBuffer, dataSize);
    TEST_ASSERT_EQUAL(true, retval);

    /* ======= RT3/3: Write flash not successfully */
    FOREVER_ExpectAndReturn(1);
    FLASH_FAPI_CHECK_FSM_READY_BUSY = Fapi_Status_FsmReady;
    Fapi_issueProgrammingCommand_ExpectAndReturn(
        (uint32_t *)flashMemoryCorrect,
        (uint8_t *)dataBuffer,
        dataSize,
        NULL,
        0u,
        Fapi_AutoEccGeneration,
        Fapi_Error_Fail);
    retval = FLASH_WriteFlashWithEccAutoGeneration((uint32_t *)flashMemoryCorrect, (uint8_t *)dataBuffer, dataSize);
    TEST_ASSERT_EQUAL(false, retval);
}

void testFLASH_EraseFlashSector(void) {
    bool retval                      = false;
    uint32_t *pU32StartAddressSector = (uint32_t *)0x00020000u;

    /* ======= AT1/1: Invalid sector address range */
    TEST_ASSERT_FAIL_ASSERT(FLASH_EraseFlashSector(NULL_PTR));

    /* ======= RT1/3: Erase flash sector successfully */
    Fapi_issueAsyncCommandWithAddress_ExpectAndReturn(Fapi_EraseSector, pU32StartAddressSector, Fapi_Status_Success);
    FOREVER_ExpectAndReturn(1);
    FLASH_FAPI_CHECK_FSM_READY_BUSY = Fapi_Status_FsmReady;
    FLASH_FAPI_GET_FSM_STATUS       = Fapi_Status_Success;
    retval                          = FLASH_EraseFlashSector(pU32StartAddressSector);
    TEST_ASSERT_EQUAL(true, retval);

    /* ======= RT2/3: Erase flash sector command is successful, but the status is not ready */
    Fapi_issueAsyncCommandWithAddress_ExpectAndReturn(Fapi_EraseSector, pU32StartAddressSector, Fapi_Status_Success);
    FOREVER_ExpectAndReturn(1);
    FLASH_FAPI_CHECK_FSM_READY_BUSY = Fapi_Status_FsmBusy;
    FOREVER_ExpectAndReturn(0);
    FLASH_FAPI_GET_FSM_STATUS = Fapi_Error_Fail;
    retval                    = FLASH_EraseFlashSector(pU32StartAddressSector);
    TEST_ASSERT_EQUAL(false, retval);

    /* ======= RT2/3: Erase flash sector not successfully */
    Fapi_issueAsyncCommandWithAddress_ExpectAndReturn(Fapi_EraseSector, pU32StartAddressSector, Fapi_Error_Fail);
    retval = FLASH_EraseFlashSector(pU32StartAddressSector);
    TEST_ASSERT_EQUAL(false, retval);
}

void testFLASH_FlashProgramCheck(void) {
    bool retval = false;

    /* Prepare a data buffer and a flash memory */
    const uint32_t dataSize = 128u;
    uint8_t dataBuffer[dataSize];
    uint8_t flashMemoryCorrect[dataSize];
    uint8_t flashMemoryIncorrect[dataSize];
    for (uint32_t iByte = 0u; iByte < dataSize; iByte++) {
        dataBuffer[iByte]           = 0xAA;
        flashMemoryCorrect[iByte]   = 0xAA;
        flashMemoryIncorrect[iByte] = 0xBA;
    }

    /* ======= AT1/3: Assertion Tests: Data buffer address is null */
    TEST_ASSERT_FAIL_ASSERT(FLASH_FlashProgramCheck((uint32_t *)flashMemoryCorrect, NULL, dataSize));

    /* ======= AT2/3: Assertion Tests: Flash memory address is null */
    TEST_ASSERT_FAIL_ASSERT(FLASH_FlashProgramCheck(NULL, (uint8_t *)dataBuffer, dataSize));

    /* ======= AT3/3: Assertion Tests: Data size is 0u */
    TEST_ASSERT_FAIL_ASSERT(FLASH_FlashProgramCheck((uint32_t *)flashMemoryCorrect, (uint8_t *)dataBuffer, 0u));

    /* ======= RT1/2: The data that has been written into the flash is identical
    to the data in the data buffer */
    retval = FLASH_FlashProgramCheck((uint32_t *)flashMemoryCorrect, (uint8_t *)dataBuffer, dataSize);
    TEST_ASSERT_EQUAL(true, retval);

    /* ======= RT2/2: The data that has been written into the flash is not identical
    to the data in the data buffer */
    retval = FLASH_FlashProgramCheck((uint32_t *)flashMemoryIncorrect, (uint8_t *)dataBuffer, dataSize);
    TEST_ASSERT_EQUAL(false, retval);
}

void testFLASH_GetFlashSector(void) {
    FLASH_FLASH_SECTOR_s sector;
    /* ======= AT1/1: Invalid sector address range */
    TEST_ASSERT_FAIL_ASSERT(FLASH_GetFlashSector((const uint32_t *)0x004FFFFFu));
    TEST_ASSERT_FAIL_ASSERT(FLASH_GetFlashSector((const uint32_t *)0xF022FFFFu));

    /* ======= RT1/2: Invalid sector address */
    sector = FLASH_GetFlashSector((const uint32_t *)0x200);
    TEST_ASSERT_EQUAL(0u, sector.idxOfThisSector);

    /* ======= RT2/2: Valid sector address */
    for (uint8_t iFlashSector = 0u; iFlashSector < FLASH_NUM_OF_FLASH_SECTORS; iFlashSector++) {
        sector = FLASH_GetFlashSector((const uint32_t *)flash_kFlashSectors[iFlashSector].pU32SectorAddressStart);
        TEST_ASSERT_EQUAL(iFlashSector, sector.idxOfThisSector);
        TEST_ASSERT_EQUAL(true, sector.isThisSectorValid);
    }
}

void testFLASH_WriteFlashSector(void) {
    uint8_t retVal;

    /* Prepare the test variables */
    uint32_t sectorSize = (uint32_t)(flash_kFlashSectors[FLASH_FIRST_SECTOR_FOR_APP_INDEX].pU8SectorAddressEnd + 1u) -
                          (uint32_t)flash_kFlashSectors[FLASH_FIRST_SECTOR_FOR_APP_INDEX].pU32SectorAddressStart;
    uint8_t dataBuffer[sectorSize];
    for (uint32_t iByte = 0u; iByte < sectorSize; iByte++) {
        dataBuffer[iByte] = 0xAA;
    }
    uint32_t *pU32SectorStartAddress = flash_kFlashSectors[FLASH_FIRST_SECTOR_FOR_APP_INDEX].pU32SectorAddressStart;
    uint8_t *pU8DataBuffer           = (uint8_t *)dataBuffer;

    /* ======= AT1/3: Assertion tests: Flash address null Pointer */
    TEST_ASSERT_FAIL_ASSERT(FLASH_WriteFlashSector(NULL, pU8DataBuffer, sectorSize));

    /* ======= AT2/3: Assertion tests: Flash address null Pointer */
    TEST_ASSERT_FAIL_ASSERT(FLASH_WriteFlashSector(pU32SectorStartAddress, NULL, sectorSize));

    /* ======= AT3/3: Assertion tests: sectorSize is 0u */
    TEST_ASSERT_FAIL_ASSERT(FLASH_WriteFlashSector(pU32SectorStartAddress, pU8DataBuffer, 0u));

    /* ======= RT1/8: FLASH_EXCEPTION_INITIALIZATION_FAILED */
    Fapi_initializeFlashBanks_ExpectAndReturn((uint32_t)FLASH_SYS_CLK_FREQ, Fapi_Error_Fail);
    retVal = FLASH_WriteFlashSector(pU32SectorStartAddress, pU8DataBuffer, sectorSize);
    TEST_ASSERT_EQUAL(FLASH_EXCEPTION_INITIALIZATION_FAILED, retVal);

    /* ======= RT2/8: FLASH_EXCEPTION_FLASH_SECTOR_INVALID */
    Fapi_initializeFlashBanks_ExpectAndReturn((uint32_t)FLASH_SYS_CLK_FREQ, Fapi_Status_Success);
    retVal = FLASH_WriteFlashSector((uint32_t *)0x22, pU8DataBuffer, sectorSize);
    TEST_ASSERT_EQUAL(FLASH_EXCEPTION_FLASH_SECTOR_INVALID, retVal);

    /* ======= RT3/8: FLASH_EXCEPTION_WRONG_SECTOR_SIZE */
    Fapi_initializeFlashBanks_ExpectAndReturn((uint32_t)FLASH_SYS_CLK_FREQ, Fapi_Status_Success);
    retVal = FLASH_WriteFlashSector(pU32SectorStartAddress, pU8DataBuffer, 200u);
    TEST_ASSERT_EQUAL(FLASH_EXCEPTION_WRONG_SECTOR_SIZE, retVal);

    /* ======= RT4/8: FLASH_EXCEPTION_PREPARE_FLASH_BANK_FAILED */
    Fapi_initializeFlashBanks_ExpectAndReturn((uint32_t)FLASH_SYS_CLK_FREQ, Fapi_Status_Success);

    /* Prepare flash not successfully */
    Fapi_setActiveFlashBank_ExpectAndReturn(Fapi_FlashBank0, Fapi_Status_Success);
    Fapi_enableMainBankSectors_ExpectAndReturn(FLASH_FULL_16_BITS, Fapi_Error_Fail);

    retVal = FLASH_WriteFlashSector(pU32SectorStartAddress, pU8DataBuffer, sectorSize);
    TEST_ASSERT_EQUAL(FLASH_EXCEPTION_PREPARE_FLASH_BANK_FAILED, retVal);

    /* ======= RT5/8: FLASH_EXCEPTION_ERASING_FLASH_SECTOR_FAILED */
    Fapi_initializeFlashBanks_ExpectAndReturn((uint32_t)FLASH_SYS_CLK_FREQ, Fapi_Status_Success);

    /* Prepare flash successfully */
    Fapi_setActiveFlashBank_ExpectAndReturn(Fapi_FlashBank0, Fapi_Status_Success);
    Fapi_enableMainBankSectors_ExpectAndReturn(FLASH_FULL_16_BITS, Fapi_Status_Success);
    FOREVER_ExpectAndReturn(1);
    FLASH_FAPI_CHECK_FSM_READY_BUSY = Fapi_Status_FsmReady;

    /* Erase flash sector not successfully */
    Fapi_issueAsyncCommandWithAddress_ExpectAndReturn(Fapi_EraseSector, pU32SectorStartAddress, Fapi_Error_Fail);

    retVal = FLASH_WriteFlashSector(pU32SectorStartAddress, pU8DataBuffer, sectorSize);
    TEST_ASSERT_EQUAL(FLASH_EXCEPTION_ERASING_FLASH_SECTOR_FAILED, retVal);

    /* ======= RT6/8: FLASH_EXCEPTION_WRITING_FLASH_SECTOR_FAILED */
    Fapi_initializeFlashBanks_ExpectAndReturn((uint32_t)FLASH_SYS_CLK_FREQ, Fapi_Status_Success);

    /* Prepare flash successfully */
    Fapi_setActiveFlashBank_ExpectAndReturn(Fapi_FlashBank0, Fapi_Status_Success);
    Fapi_enableMainBankSectors_ExpectAndReturn(FLASH_FULL_16_BITS, Fapi_Status_Success);
    FOREVER_ExpectAndReturn(1);
    FLASH_FAPI_CHECK_FSM_READY_BUSY = Fapi_Status_FsmReady;

    /* Erase flash sector successfully */
    Fapi_issueAsyncCommandWithAddress_ExpectAndReturn(Fapi_EraseSector, pU32SectorStartAddress, Fapi_Status_Success);
    FOREVER_ExpectAndReturn(1);
    FLASH_FAPI_CHECK_FSM_READY_BUSY = Fapi_Status_FsmReady;
    FLASH_FAPI_GET_FSM_STATUS       = Fapi_Status_Success;

    /* Write flash not successfully */
    FOREVER_ExpectAndReturn(1);
    FLASH_FAPI_CHECK_FSM_READY_BUSY = Fapi_Status_FsmReady;
    Fapi_issueProgrammingCommand_ExpectAndReturn(
        (uint32_t *)pU32SectorStartAddress,
        (uint8_t *)pU8DataBuffer,
        FLASH_DATA_BUFFER_ARRAY_TO_FLASH_SIZE,
        NULL,
        0u,
        Fapi_AutoEccGeneration,
        Fapi_Error_Fail);

    retVal = FLASH_WriteFlashSector(pU32SectorStartAddress, pU8DataBuffer, sectorSize);
    TEST_ASSERT_EQUAL(FLASH_EXCEPTION_WRITING_FLASH_SECTOR_FAILED, retVal);

    /* ======= RT7/8: FLASH_EXCEPTION_FLASH_CHECK_FAILED */
    Fapi_initializeFlashBanks_ExpectAndReturn((uint32_t)FLASH_SYS_CLK_FREQ, Fapi_Status_Success);

    /* Prepare flash successfully */
    Fapi_setActiveFlashBank_ExpectAndReturn(Fapi_FlashBank0, Fapi_Status_Success);
    Fapi_enableMainBankSectors_ExpectAndReturn(FLASH_FULL_16_BITS, Fapi_Status_Success);
    FOREVER_ExpectAndReturn(1);
    FLASH_FAPI_CHECK_FSM_READY_BUSY = Fapi_Status_FsmReady;

    /* Erase flash sector successfully */
    Fapi_issueAsyncCommandWithAddress_ExpectAndReturn(Fapi_EraseSector, pU32SectorStartAddress, Fapi_Status_Success);
    FOREVER_ExpectAndReturn(1);
    FLASH_FAPI_CHECK_FSM_READY_BUSY = Fapi_Status_FsmReady;
    FLASH_FAPI_GET_FSM_STATUS       = Fapi_Status_Success;

    /* Write flash successfully */
    uint32_t *pU32InLoops         = pU32SectorStartAddress;
    uint8_t *pU8DataBufferInLoops = pU8DataBuffer;
    for (uint32_t iWrite = 0u; iWrite < sectorSize; iWrite += FLASH_DATA_BUFFER_ARRAY_TO_FLASH_SIZE) {
        FOREVER_ExpectAndReturn(1);
        FLASH_FAPI_CHECK_FSM_READY_BUSY = Fapi_Status_FsmReady;
        Fapi_issueProgrammingCommand_ExpectAndReturn(
            (uint32_t *)pU32InLoops,
            (uint8_t *)pU8DataBufferInLoops,
            FLASH_DATA_BUFFER_ARRAY_TO_FLASH_SIZE,
            NULL,
            0u,
            Fapi_AutoEccGeneration,
            Fapi_Status_Success);
        FOREVER_ExpectAndReturn(1);
        FLASH_FAPI_CHECK_FSM_READY_BUSY = Fapi_Status_FsmReady;
        FOREVER_ExpectAndReturn(1);
        FLASH_FAPI_GET_FSM_STATUS = Fapi_Status_Success;
        pU32InLoops += FLASH_PU32_ADDRESS_INCREASE_EACH_WRITE_LOOP;
        pU8DataBufferInLoops += FLASH_DATA_BUFFER_ARRAY_TO_FLASH_SIZE;
    }

    /* Check the flash sector fails */
    retVal = FLASH_WriteFlashSector(pU32SectorStartAddress, pU8DataBuffer, sectorSize);
    TEST_ASSERT_EQUAL(FLASH_EXCEPTION_FLASH_CHECK_FAILED, retVal);

    /* ======= RT8/8: FLASH_EXCEPTION_CODE_NO_EXCEPTION */
    /* Prepare the test variables */
    sectorSize = (uint32_t)(flash_kFlashSectors[7u].pU8SectorAddressEnd + 1u) -
                 (uint32_t)flash_kFlashSectors[7u].pU32SectorAddressStart;
    TEST_ASSERT_EQUAL(128u, sectorSize);

    /* Prepare a data buffer and a mocked flash sector which have same data */
    for (uint32_t iByte = 0u; iByte < mockFlashSector7Size; iByte++) {
        dataBuffer[iByte]       = 0xAA;
        mockFlashSector7[iByte] = 0xAA;
    }

    /* Initialize flash bank successfully */
    Fapi_initializeFlashBanks_ExpectAndReturn((uint32_t)FLASH_SYS_CLK_FREQ, Fapi_Status_Success);

    /* Prepare flash successfully */
    Fapi_setActiveFlashBank_ExpectAndReturn(Fapi_FlashBank0, Fapi_Status_Success);
    Fapi_enableMainBankSectors_ExpectAndReturn(FLASH_FULL_16_BITS, Fapi_Status_Success);
    FOREVER_ExpectAndReturn(1);
    FLASH_FAPI_CHECK_FSM_READY_BUSY = Fapi_Status_FsmReady;

    /* Erase flash sector successfully */
    Fapi_issueAsyncCommandWithAddress_ExpectAndReturn(Fapi_EraseSector, pU32SectorStartAddress, Fapi_Status_Success);
    FOREVER_ExpectAndReturn(1);
    FLASH_FAPI_CHECK_FSM_READY_BUSY = Fapi_Status_FsmReady;
    FLASH_FAPI_GET_FSM_STATUS       = Fapi_Status_Success;

    /* Write flash successfully */
    pU32InLoops          = flash_kFlashSectors[7u].pU32SectorAddressStart;
    pU8DataBufferInLoops = (uint8_t *)dataBuffer;
    for (uint32_t iWrite = 0u; iWrite < sectorSize; iWrite += FLASH_DATA_BUFFER_ARRAY_TO_FLASH_SIZE) {
        FOREVER_ExpectAndReturn(1);
        FLASH_FAPI_CHECK_FSM_READY_BUSY = Fapi_Status_FsmReady;
        Fapi_issueProgrammingCommand_ExpectAndReturn(
            (uint32_t *)pU32InLoops,
            (uint8_t *)pU8DataBufferInLoops,
            FLASH_DATA_BUFFER_ARRAY_TO_FLASH_SIZE,
            NULL,
            0u,
            Fapi_AutoEccGeneration,
            Fapi_Status_Success);
        FOREVER_ExpectAndReturn(1);
        FLASH_FAPI_CHECK_FSM_READY_BUSY = Fapi_Status_FsmReady;
        FOREVER_ExpectAndReturn(1);
        FLASH_FAPI_GET_FSM_STATUS = Fapi_Status_Success;
        pU32InLoops += FLASH_PU32_ADDRESS_INCREASE_EACH_WRITE_LOOP;
        pU8DataBufferInLoops += FLASH_DATA_BUFFER_ARRAY_TO_FLASH_SIZE;
    }

    /* Check the flash sector successfully */
    retVal = FLASH_WriteFlashSector(pU32SectorStartAddress, pU8DataBuffer, sectorSize);
    TEST_ASSERT_EQUAL(FLASH_EXCEPTION_CODE_NO_EXCEPTION, retVal);
}

void testFLASH_EraseFlashForApp(void) {
    bool retval = false;
    /* ======= RT1/2: Erase Flash for App successfully */
    for (uint8_t i = FLASH_FIRST_SECTOR_FOR_APP_INDEX; i <= FLASH_LAST_SECTOR_FOR_APP_INDEX; i++) {
        FLASH_WriteFlashSector_Return[i] = true;
    }
    retval = FLASH_EraseFlashForApp();
    TEST_ASSERT_EQUAL(true, retval);

    /* ======= RT2/2: Erase Flash for App not successfully */
    FLASH_WriteFlashSector_Return[FLASH_FIRST_SECTOR_FOR_APP_INDEX] = false;
    retval                                                          = FLASH_EraseFlashForApp();
    TEST_ASSERT_EQUAL(false, retval);
}
