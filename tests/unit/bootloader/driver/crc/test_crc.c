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
 * @file    test_crc.c
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
#include "MockHL_crc.h"
#include "MockHL_rti.h"
#include "MockHL_sys_core.h"
#include "MockHL_sys_dma.h"
#include "Mockcrc64_calc.h"
#include "Mockrti.h"

#include "crc.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("crc.c")

TEST_INCLUDE_PATH("../../src/bootloader/driver/crc")
TEST_INCLUDE_PATH("../../src/bootloader/driver/crc/vendor")
TEST_INCLUDE_PATH("../../src/bootloader/driver/rti")

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/**
 * @brief   Testing CRC_SwapCrc64Bits
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/1: Function swaps bits as expected
 */
void testCRC_SwapCrc64Bits(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    uint64_t testCrcValue64Bits = 0x0123456789ABCDEFu;
    /* ======= RT1/1: Call function under test */
    uint64_t testResult = TEST_CRC_SwapCrc64Bits(testCrcValue64Bits);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(0x89ABCDEF01234567u, testResult);
}

/**
 * @brief   Testing CRC_CalculateCrc64
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: dataSizeIn64Bits = 0
 *            - AT2/2: pkData = NULL_PTR
 *          - Routine validation:
 *            - RT1/2: Calculate crc of one uint64_t
 *            - RT2/2: Calculate crc of three uint64_t
 */
void testCRC_CalculateCrc64(void) {
    /* ======= Assertion tests ============================================= */
    uint32_t dataSizeIn64Bits   = 3u;
    const uint64_t testData[3u] = {0x01u, 0x02u, 0x03u};
    uint64_t testCrcRoot        = 0x0123456789ABCDEFu;
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(CRC_CalculateCrc64(testData, 0u, testCrcRoot));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(CRC_CalculateCrc64(NULL_PTR, dataSizeIn64Bits, testCrcRoot));

    /* ======= Routine tests =============================================== */
    uint64_t testDataSwapped[3u]  = {0x0100000000u, 0x0200000000u, 0x0300000000};
    uint64_t testCrc64[4u]        = {0xFEu, 0xDCu, 0xBAu, 0x98u};
    testCrcRoot                   = testCrc64[0u];
    uint64_t testCrc64Swapped[4u] = {0xFE00000000u, 0xDC00000000u, 0xBA00000000u, 0x9800000000u};
    /* ======= RT1/2: Test implementation */
    calc_crc64_user_ExpectAndReturn(testCrc64Swapped[0u], testDataSwapped[0u], testCrc64Swapped[1u]);
    /* ======= RT1/2: Call function under test */
    uint64_t testResult = CRC_CalculateCrc64(testData, 1u, testCrcRoot);
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(testCrc64[1u], testResult);

    /* ======= RT2/2: Test implementation */
    calc_crc64_user_ExpectAndReturn(testCrc64Swapped[0u], testDataSwapped[0u], testCrc64Swapped[1u]);
    calc_crc64_user_ExpectAndReturn(testCrc64Swapped[1u], testDataSwapped[1u], testCrc64Swapped[2u]);
    calc_crc64_user_ExpectAndReturn(testCrc64Swapped[2u], testDataSwapped[2u], testCrc64Swapped[3u]);
    /* ======= RT2/2: Call function under test */
    testResult = CRC_CalculateCrc64(testData, dataSizeIn64Bits, testCrcRoot);
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(testCrc64[3u], testResult);
}
