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
 * @file    test_crc64_calc.c
 * @author  foxBMS Team
 * @date    2024-09-17 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of some module
 * @details The test cases shall be aligned with the Python implementation of
 *          the test cases in 'tests/cli/cmd_bootloader/test_crc64_ti_impl.py'.
 */

/*========== Includes =======================================================*/
#include "unity.h"

#include "crc64_calc.h"
#include "test_assert_helper.h"

#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("crc64_calc.c")

TEST_INCLUDE_PATH("../../src/bootloader/driver/crc/vendor")

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/**
 * @brief   Testing calc_crc64_user
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/2: data = 0x0123456789ABCDEF, crc = 0x0101010101010101
 *            - RT1/2: data = 0x11223344CCDDEEFF, crc = 0x1232123212321232
 */
void testcalc_crc64_user(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    /* The first and second 32 bits of input data and crc have to be swapped */
    uint64_t testDataSwapped = 0x89ABCDEF01234567u;
    uint64_t testCrcSwapped  = 0x0101010101010101u;
    /* ======= RT1/2: Call function under test */
    uint64_t testResult = calc_crc64_user(testDataSwapped, testCrcSwapped);
    /* ======= RT1/2: Test output verification */
    /* The first and second 32 bits of the output are also swapped*/
    uint64_t expectedCrcSwapped = 0x5605FCA20350A955u;
    TEST_ASSERT_EQUAL(expectedCrcSwapped, testResult);

    /* ======= RT2/2: Test implementation */
    testDataSwapped = 0xCCDDEEFF11223344U;
    testCrcSwapped  = 0x1232123212321232U;
    /* ======= RT2/2: Call function under test */
    testResult = calc_crc64_user(testDataSwapped, testCrcSwapped);
    /* ======= RT2/2: Test output verification */
    expectedCrcSwapped = 0x7AB02FEF2CB37FA4u;
    TEST_ASSERT_EQUAL(expectedCrcSwapped, testResult);
}
