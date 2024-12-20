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
 * @file    test_utils.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2024-12-20 (date of last update)
 * @version v1.8.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the utils module
 * @details Tests ExtractCharactersFromString
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"

#include "fstd_types.h"
#include "test_assert_helper.h"
#include "utils.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/**
 * @brief   Testing UTIL_ExtractCharactersFromString
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/4: pExtractedCharacters is NULL_PTR
 *            - AT2/4: pString is NULL_PTR
 *            - AT3/4: startChar >= stringLength
 *            - AT4/4: numberOfChars > stringLength
 *          - Routine validation:
 *            - RT1/3: Extract 8 characters
 *            - RT2/3: Extract 2 characters
 *            - RT3/3: Try to extract 8 when only 5 are left
 */
void testUTIL_ExtractCharactersFromString(void) {
    uint64_t testExtractedCharacters = 0u;
    /* "Testing!" == 0x54657374696E6721313233*/
    char testString[11u] = {'T', 'e', 's', 't', 'i', 'n', 'g', '!', '1', '2', '3'};
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/4 ======= */
    TEST_ASSERT_FAIL_ASSERT(UTIL_ExtractCharactersFromString(NULL_PTR, testString, 11u, 1u, 1u));
    /* ======= AT2/4 ======= */
    TEST_ASSERT_FAIL_ASSERT(UTIL_ExtractCharactersFromString(&testExtractedCharacters, NULL_PTR, 11u, 1u, 1u));
    /* ======= AT3/4 ======= */
    TEST_ASSERT_FAIL_ASSERT(UTIL_ExtractCharactersFromString(&testExtractedCharacters, testString, 11u, 11u, 1u));
    /* ======= AT4/4 ======= */
    TEST_ASSERT_FAIL_ASSERT(UTIL_ExtractCharactersFromString(&testExtractedCharacters, testString, 11u, 1u, 12u));
    /* ======= Routine tests =============================================== */
    /* ======= RT1/3: Test implementation */
    /* ======= RT1/3: Call function under test */
    UTIL_ExtractCharactersFromString(&testExtractedCharacters, testString, 11u, 0u, 8u);
    /* ======= RT1/3: Test output verification */
    TEST_ASSERT_EQUAL(0x54657374696E6721u, testExtractedCharacters);

    /* ======= RT2/3: Test implementation */
    testExtractedCharacters = 0u;
    /* ======= RT2/3: Call function under test */
    UTIL_ExtractCharactersFromString(&testExtractedCharacters, testString, 11u, 8u, 2u);
    /* ======= RT2/3: Test output verification */
    TEST_ASSERT_EQUAL(0x3132u, testExtractedCharacters);

    /* ======= RT3/3: Test implementation */
    testExtractedCharacters = 0u;
    /* ======= RT3/3: Call function under test */
    UTIL_ExtractCharactersFromString(&testExtractedCharacters, testString, 11u, 6u, 8u);
    /* ======= RT3/3: Test output verification */
    TEST_ASSERT_EQUAL(0x6721313233u, testExtractedCharacters);
}
