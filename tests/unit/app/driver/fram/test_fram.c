/**
 *
 * @copyright &copy; 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    test_fram.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2024-08-08 (date of last update)
 * @version v1.7.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the fram module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockHL_spi.h"
#include "Mockcrc.h"
#include "Mockdiag.h"
#include "Mockio.h"
#include "Mockmcu.h"
#include "Mockspi.h"

#include "fram_cfg.h"
#include "spi_cfg.h"
#include "version_cfg.h"

#include "fassert.h"
#include "fram.h"
#include "test_assert_helper.h"

#include <stdbool.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/crc")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/fram")
TEST_INCLUDE_PATH("../../src/app/driver/io")
TEST_INCLUDE_PATH("../../src/app/driver/spi")
TEST_INCLUDE_PATH("../../src/app/engine/diag")

/*========== Definitions and Implementations for Unit Test ==================*/
const VER_VERSION_s ver_foxbmsVersionInformation VER_VERSION_INFORMATION = {
    .underVersionControl     = true,
    .isDirty                 = true,
    .major                   = 120,
    .minor                   = 121,
    .patch                   = 122,
    .distanceFromLastRelease = 22,
    .commitHash              = "abcdefgh",
    .gitRemote               = "git@remote-repo.de:foxbms-2.git",
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/**
 * @brief   Testing extern function #FRAM_WriteData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: invalid blockId &rarr; assert
 *          - Routine validation:
 *            - RT1/x: TODO
 */
void testFRAM_WriteData(void) {
    /* ======= Assertion tests ============================================= */
    const FRAM_BLOCK_ID_e invalidFramBlockId = (FRAM_BLOCK_ID_e)(FRAM_BLOCK_MAX + 1u);
    /* ======= AT1/1: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(FRAM_WriteData(invalidFramBlockId));
    /* ======= AT2/2: Assertion test */
    spi_framInterface.csType = SPI_CHIP_SELECT_HARDWARE;
    TEST_ASSERT_FAIL_ASSERT(FRAM_ReadData(invalidFramBlockId));
/* reset for tests */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"
    spi_framInterface.csType == SPI_CHIP_SELECT_SOFTWARE;
#pragma GCC diagnostic pop

    /* ======= Routine tests =============================================== */
    /* ======= RT1/x: Test implementation */
    uint64_t crc           = 0u;
    uint8_t pWrite         = 0u;
    uint32_t lengthInBytes = 0u;
    CRC_CalculateCrc_ExpectAndReturn(&crc, &pWrite, lengthInBytes, STD_NOT_OK);
    CRC_CalculateCrc_IgnoreArg_pCrc();
    CRC_CalculateCrc_IgnoreArg_pData();
    CRC_CalculateCrc_IgnoreArg_lengthInBytes();
    FRAM_RETURN_TYPE_e ret = FRAM_WriteData(FRAM_BLOCK_ID_VERSION);
    TEST_ASSERT_EQUAL(FRAM_ACCESS_CRC_BUSY, ret);
}

/**
 * @brief   Testing extern function #FRAM_WriteData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: invalid blockId &rarr; assert
 *            - AT2/2: invalid chip select type
 *          - Routine validation:
 *            - RT1/x: TODO
 */
void testFRAM_ReadData(void) {
    /* ======= Assertion tests ============================================= */
    const FRAM_BLOCK_ID_e invalidFramBlockId = (FRAM_BLOCK_ID_e)(FRAM_BLOCK_MAX + 1u);
    /* ======= AT1/2: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(FRAM_ReadData(invalidFramBlockId));
    /* ======= AT2/2: Assertion test */
    spi_framInterface.csType = SPI_CHIP_SELECT_HARDWARE;
    TEST_ASSERT_FAIL_ASSERT(FRAM_ReadData(invalidFramBlockId));
/* reset for tests */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"
    spi_framInterface.csType == SPI_CHIP_SELECT_SOFTWARE;
#pragma GCC diagnostic pop

    /* ======= Routine tests =============================================== */
    /* ======= RT1/x: Test implementation */
}
