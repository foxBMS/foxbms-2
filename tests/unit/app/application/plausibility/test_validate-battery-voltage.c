/**
 *
 * @copyright &copy; 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    test_validate-battery-voltage.c
 * @author  foxBMS Team
 * @date    2026-07-23 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Validate battery-voltage behavior
 * @details Verify #PL_ValidateBatteryVoltageMeasurement aggregation rules for
 *          connected and disconnected strings.
 *          Covered behaviors include:
 *          - averaging over valid strings when no string is connected,
 *          - averaging over connected and valid strings when strings are
 *            connected, and
 *          - invalid fallback behavior when no usable string voltage exists.
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockbms.h"

#include "fstd_types.h"
#include "plausibility.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/**
 * @brief   Test #PL_ValidateBatteryVoltageMeasurement
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for pTablePackValues -> assert
 *          - Routine validation:
 *            - RT1/4: No strings connected and no valid string voltages
 *                     -> battery voltage invalid.
 *            - RT2/4: No strings connected and at least one valid string
 *                     voltage
 *                     -> battery voltage is average of all valid strings.
 *            - RT3/4: Strings connected and only closed+valid strings are used
 *                     -> battery voltage is average of closed valid strings.
 *            - RT4/4: Strings connected but none closed+valid
 *                     -> battery voltage invalid.
 *            - RT5/5: String is connected but string voltage invalid
 *                     -> connected invalid strings are ignored, battery
 *                        voltage invalid if no other valid connected string.
 */
void testPL_ValidateBatteryVoltageMeasurement(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 */
    TEST_ASSERT_FAIL_ASSERT(PL_ValidateBatteryVoltageMeasurement(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/4: Test implementation */
    DATA_BLOCK_PACK_VALUES_s pTablePackValuesRt1 = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        pTablePackValuesRt1.invalidStringVoltage[s] = 1u;
    }
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(0u);

    /* ======= RT1/4: call function under test */
    PL_ValidateBatteryVoltageMeasurement(&pTablePackValuesRt1);

    /* ======= RT1/4: test output verification */
    TEST_ASSERT_EQUAL(1u, pTablePackValuesRt1.invalidBatteryVoltage);
    TEST_ASSERT_EQUAL(INT32_MAX, pTablePackValuesRt1.batteryVoltage_mV);

    /* ======= RT2/4: Test implementation */
    DATA_BLOCK_PACK_VALUES_s pTablePackValuesRt2 = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        pTablePackValuesRt2.invalidStringVoltage[s] = 1u;
    }
    pTablePackValuesRt2.invalidStringVoltage[0] = 0u;
    pTablePackValuesRt2.stringVoltage_mV[0]     = 10000;
    if (BS_NR_OF_STRINGS > 1u) {
        pTablePackValuesRt2.invalidStringVoltage[1] = 0u;
        pTablePackValuesRt2.stringVoltage_mV[1]     = 11000;
    }
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(0u);

    /* ======= RT2/4: call function under test */
    PL_ValidateBatteryVoltageMeasurement(&pTablePackValuesRt2);

    /* ======= RT2/4: test output verification */
    TEST_ASSERT_EQUAL(0u, pTablePackValuesRt2.invalidBatteryVoltage);
    if (BS_NR_OF_STRINGS > 1u) {
        TEST_ASSERT_EQUAL(10500, pTablePackValuesRt2.batteryVoltage_mV);
    } else {
        TEST_ASSERT_EQUAL(10000, pTablePackValuesRt2.batteryVoltage_mV);
    }

    /* ======= RT3/4: Test implementation */
    DATA_BLOCK_PACK_VALUES_s pTablePackValuesRt3 = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        pTablePackValuesRt3.invalidStringVoltage[s] = 1u;
        pTablePackValuesRt3.stringVoltage_mV[s]     = 50000;
    }
    pTablePackValuesRt3.invalidStringVoltage[0] = 0u;
    pTablePackValuesRt3.stringVoltage_mV[0]     = 10000;
    if (BS_NR_OF_STRINGS > 1u) {
        pTablePackValuesRt3.invalidStringVoltage[1] = 0u;
        pTablePackValuesRt3.stringVoltage_mV[1]     = 12000;
    }
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(1u);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        if (s == 0u) {
            BMS_IsStringClosed_ExpectAndReturn(s, true);
        } else {
            BMS_IsStringClosed_ExpectAndReturn(s, false);
        }
    }

    /* ======= RT3/4: call function under test */
    PL_ValidateBatteryVoltageMeasurement(&pTablePackValuesRt3);

    /* ======= RT3/4: test output verification */
    TEST_ASSERT_EQUAL(0u, pTablePackValuesRt3.invalidBatteryVoltage);
    TEST_ASSERT_EQUAL(10000, pTablePackValuesRt3.batteryVoltage_mV);

    /* ======= RT4/5: Test implementation */
    DATA_BLOCK_PACK_VALUES_s pTablePackValuesRt4 = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        pTablePackValuesRt4.invalidStringVoltage[s] = 1u;
        pTablePackValuesRt4.stringVoltage_mV[s]     = 10000;
    }
    /* Keep one valid voltage but make it unavailable by opening the string */
    pTablePackValuesRt4.invalidStringVoltage[0] = 0u;
    pTablePackValuesRt4.stringVoltage_mV[0]     = 11000;
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(1u);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        BMS_IsStringClosed_ExpectAndReturn(s, false);
    }

    /* ======= RT4/5: call function under test */
    PL_ValidateBatteryVoltageMeasurement(&pTablePackValuesRt4);

    /* ======= RT4/5: test output verification */
    TEST_ASSERT_EQUAL(1u, pTablePackValuesRt4.invalidBatteryVoltage);
    TEST_ASSERT_EQUAL(INT32_MAX, pTablePackValuesRt4.batteryVoltage_mV);

    /* ======= RT5/5: Test implementation */
    DATA_BLOCK_PACK_VALUES_s pTablePackValuesRt5 = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        pTablePackValuesRt5.invalidStringVoltage[s] = 1u;
        pTablePackValuesRt5.stringVoltage_mV[s]     = 13000;
    }
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(1u);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        BMS_IsStringClosed_ExpectAndReturn(s, true);
    }

    /* ======= RT5/5: call function under test */
    PL_ValidateBatteryVoltageMeasurement(&pTablePackValuesRt5);

    /* ======= RT5/5: test output verification */
    TEST_ASSERT_EQUAL(1u, pTablePackValuesRt5.invalidBatteryVoltage);
    TEST_ASSERT_EQUAL(INT32_MAX, pTablePackValuesRt5.batteryVoltage_mV);
}
