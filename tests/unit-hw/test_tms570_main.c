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
 * @file    test_tms570_main.c
 * @author  foxBMS Team
 * @date    2019-08-27 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup GENERAL
 * @prefix  MAIN
 *
 * @brief   Main function to test the hardware dependent functions in different
 *          modules.
 * @details This main function calls a series of test functions to test
 *          different modules.
 */

/*========== Includes =======================================================*/
#include "HL_gio.h"
#include "HL_pinmux.h"
#include "HL_rti.h"
#include "HL_sys_common.h"
#include "HL_system.h"

#include "can.h"
#include "fstd_types.h"
#include "fsystem.h"
#include "main.h"
#include "test_tms570_boot.h"
#include "test_tms570_crc.h"
#include "test_tms570_flash.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/* initialize all tests with a failing value */
uint8_t testResultTEST_FLASH_GetFlashSector          = 100u;
uint8_t testResultTEST_FLASH_PrepareFlashBank        = 100u;
uint8_t testResultTEST_FLASH_FlashProgramCheck       = 100u;
uint8_t testResultTEST_FLASH_EraseFlashForApp        = 100u;
uint8_t testResultTEST_FLASH_WriteFlashSector        = 100u;
uint8_t testResultTEST_CRC_SemiAutoCrcCalculation    = 100u;
uint8_t testResultTEST_CRC_CalculateCrc64            = 100u;
uint8_t testResultTEST_BOOT_UpdateAndLoadProgramInfo = 100u;
uint8_t testResultTEST_BOOT_CleanUpRam               = 100u;

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
int main(void) {
    _disable_IRQ_interrupt_();
    muxInit();
    gioInit();
    CAN_Initialize();
    rtiInit();
    FSYS_SwitchToUserMode();
    FSYS_RaisePrivilegeToSystemMode();

    /* Copy the flash relevant .text and .const to RAM */
    memcpy(&main_textRunStartFlashC, &main_textLoadStartFlashC, (uint32_t)&main_textSizeFlashC);
    memcpy(&main_constRunStartFlashCfgC, &main_constLoadStartFlashCfgC, (uint32_t)&main_constSizeFlashCfgC);

    if (FSYS_RaisePrivilegeToSystemMode() == STD_OK) {
        /* Test flash module */
        testResultTEST_FLASH_GetFlashSector    = TEST_FLASH_GetFlashSector();
        testResultTEST_FLASH_PrepareFlashBank  = TEST_FLASH_PrepareFlashBank();
        testResultTEST_FLASH_FlashProgramCheck = TEST_FLASH_FlashProgramCheck();
        testResultTEST_FLASH_EraseFlashForApp  = TEST_FLASH_EraseFlashForApp();
        testResultTEST_FLASH_WriteFlashSector  = TEST_FLASH_WriteFlashSector();

        /* Test CRC module */
        testResultTEST_CRC_SemiAutoCrcCalculation = TEST_CRC_SemiAutoCrcCalculation();
        testResultTEST_CRC_CalculateCrc64         = TEST_CRC_CalculateCrc64();

        /* Test BOOT module */
        testResultTEST_BOOT_UpdateAndLoadProgramInfo = TEST_BOOT_UpdateAndLoadProgramInfo();
        FSYS_RaisePrivilegeToSystemModeSWI();
        testResultTEST_BOOT_CleanUpRam = TEST_BOOT_CleanUpRam();
    }

    (void)testResultTEST_FLASH_GetFlashSector;
    (void)testResultTEST_FLASH_PrepareFlashBank;
    (void)testResultTEST_FLASH_FlashProgramCheck;
    (void)testResultTEST_FLASH_EraseFlashForApp;
    (void)testResultTEST_FLASH_WriteFlashSector;
    (void)testResultTEST_CRC_SemiAutoCrcCalculation;
    (void)testResultTEST_CRC_CalculateCrc64;
    (void)testResultTEST_BOOT_UpdateAndLoadProgramInfo;
    (void)testResultTEST_BOOT_CleanUpRam;

    while (true) {
        /* stay here forever */
        /* on the HIL, we read out the test result variables to check, whether
         * the tests were successful. */
    }

#pragma diag_push
#pragma diag_suppress 112
    return 0;
#pragma diag_pop
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
