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
 * @file    test_validate-high-voltage-bus.c
 * @author  foxBMS Team
 * @date    2026-07-23 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Validate high-voltage-bus behavior
 * @details Test #PL_ValidateHighVoltageBusMeasurement.
 *          It verifies:
 *          - assertion handling for invalid input pointers,
 *          - selection of eligible strings based on closed/precharging state,
 *          - timeout filtering of outdated measurements,
 *          - exclusion of invalid string measurements, and
 *          - resulting high-voltage-bus averaging and invalidation behavior.
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockbms.h"
#include "Mockdatabase_helper.h"
#include "Mockdiag.h"

#include "fstd_types.h"
#include "plausibility.h"
#include "test_assert_helper.h"

#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/

/*========== Definitions and Implementations for Unit Test ==================*/
static DATA_BLOCK_PACK_VALUES_s test_tablePackValues          = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
static DATA_BLOCK_SYSTEM_VOLTAGE_3_s test_tableSystemVoltage3 = {.header.uniqueId = DATA_BLOCK_ID_SYSTEM_VOLTAGE_3};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/* Extern Functions */
/**
 * @brief   Test PL_ValidateHighVoltageBusMeasurement
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: pTablePackValues is NULL_PTR -> assert
 *            - AT2/2: pTableSystemVoltage3 is NULL_PTR -> assert
 *          - Routine validation: see dedicated test functions below.
 *            - RT1/5: All strings closed, updated and measurement valid
 *                     -> HV bus voltage valid and equal to the average of the
 *                        string high voltages.
 *            - RT2/5: All strings precharging, updated and measurement valid
 *                     -> HV bus voltage valid (precharging strings are used).
 *            - RT3/5: All strings closed and updated but measurement invalid
 *                     -> no valid voltages -> HV bus voltage invalid.
 *            - RT4/5: All strings closed and measurement valid but not updated
 *                     within timeout -> no valid voltages -> HV bus invalid.
 *            - RT5/5: All strings updated and measurement valid but neither
 *                     closed nor precharging -> disconnected strings are not
 *                     used -> HV bus voltage invalid.
 */
void testPL_ValidateHighVoltageBusMeasurement(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 */
    TEST_ASSERT_FAIL_ASSERT(PL_ValidateHighVoltageBusMeasurement(NULL_PTR, NULL_PTR));
    /* ======= AT2/2 */
    TEST_ASSERT_FAIL_ASSERT(PL_ValidateHighVoltageBusMeasurement(&test_tablePackValues, NULL_PTR));

    /* ======= RT1/5: Test implementation */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_tableSystemVoltage3.highVoltage_mV[s]     = 48000;
        test_tableSystemVoltage3.invalidMeasurement[s] = 0u;

        DATA_DatabaseBlockUpdatedWithinInterval_ExpectAndReturn(
            &test_tableSystemVoltage3.timestamp[s],
            &test_tableSystemVoltage3.previousTimestamp[s],
            s,
            PL_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms,
            true);
        DIAG_ReportResultToHandler_ExpectAndReturn(
            STD_OK, DIAG_ID_CURRENT_SENSOR_V3_MEASUREMENT_TIMEOUT, DIAG_STRING, s, STD_OK);
        BMS_IsStringClosed_ExpectAndReturn(s, true);
        BMS_IsStringPrecharging_ExpectAndReturn(s, false);
    }

    /* ======= RT1/5: call function under test */
    PL_ValidateHighVoltageBusMeasurement(&test_tablePackValues, &test_tableSystemVoltage3);

    /* ======= RT1/5: test output verification */
    TEST_ASSERT_EQUAL(0u, test_tablePackValues.invalidHvBusVoltage);
    /* All strings equal -> average == single value */
    TEST_ASSERT_EQUAL(48000, test_tablePackValues.highVoltageBusVoltage_mV);

    /* ======= RT2/5: Test implementation */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_tableSystemVoltage3.highVoltage_mV[s]     = 24000;
        test_tableSystemVoltage3.invalidMeasurement[s] = 0u;

        DATA_DatabaseBlockUpdatedWithinInterval_ExpectAndReturn(
            &test_tableSystemVoltage3.timestamp[s],
            &test_tableSystemVoltage3.previousTimestamp[s],
            s,
            PL_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms,
            true);
        DIAG_ReportResultToHandler_ExpectAndReturn(
            STD_OK, DIAG_ID_CURRENT_SENSOR_V3_MEASUREMENT_TIMEOUT, DIAG_STRING, s, STD_OK);
        BMS_IsStringClosed_ExpectAndReturn(s, false);
        BMS_IsStringPrecharging_ExpectAndReturn(s, true);
    }

    /* ======= RT2/5: call function under test */
    PL_ValidateHighVoltageBusMeasurement(&test_tablePackValues, &test_tableSystemVoltage3);

    /* ======= RT2/5: test output verification */
    TEST_ASSERT_EQUAL(0u, test_tablePackValues.invalidHvBusVoltage);
    TEST_ASSERT_EQUAL(24000, test_tablePackValues.highVoltageBusVoltage_mV);

    /* ======= RT3/5: Test implementation */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_tableSystemVoltage3.highVoltage_mV[s]     = 48000;
        test_tableSystemVoltage3.invalidMeasurement[s] = 1u; /* measurement invalid */

        DATA_DatabaseBlockUpdatedWithinInterval_ExpectAndReturn(
            &test_tableSystemVoltage3.timestamp[s],
            &test_tableSystemVoltage3.previousTimestamp[s],
            s,
            PL_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms,
            true);
        DIAG_ReportResultToHandler_ExpectAndReturn(
            STD_OK, DIAG_ID_CURRENT_SENSOR_V3_MEASUREMENT_TIMEOUT, DIAG_STRING, s, STD_OK);
        BMS_IsStringClosed_ExpectAndReturn(s, true);
        BMS_IsStringPrecharging_ExpectAndReturn(s, false);
    }

    /* ======= RT3/5: call function under test */
    PL_ValidateHighVoltageBusMeasurement(&test_tablePackValues, &test_tableSystemVoltage3);

    /* ======= RT3/5: test output verification */
    TEST_ASSERT_EQUAL(1u, test_tablePackValues.invalidHvBusVoltage);

    /* ======= RT4/5: Test implementation */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_tableSystemVoltage3.highVoltage_mV[s]     = 48000;
        test_tableSystemVoltage3.invalidMeasurement[s] = 0u;

        DATA_DatabaseBlockUpdatedWithinInterval_ExpectAndReturn(
            &test_tableSystemVoltage3.timestamp[s],
            &test_tableSystemVoltage3.previousTimestamp[s],
            s,
            PL_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms,
            false);
        /* STD_BoolToStdReturnType(false) == STD_NOT_OK */
        DIAG_ReportResultToHandler_ExpectAndReturn(
            STD_NOT_OK, DIAG_ID_CURRENT_SENSOR_V3_MEASUREMENT_TIMEOUT, DIAG_STRING, s, STD_OK);
        BMS_IsStringClosed_ExpectAndReturn(s, true);
        BMS_IsStringPrecharging_ExpectAndReturn(s, false);
    }

    /* ======= RT4/5: call function under test */
    PL_ValidateHighVoltageBusMeasurement(&test_tablePackValues, &test_tableSystemVoltage3);

    /* ======= RT4/5: test output verification */
    TEST_ASSERT_EQUAL(1u, test_tablePackValues.invalidHvBusVoltage);

    /* ======= RT5/5: Test implementation */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_tableSystemVoltage3.highVoltage_mV[s]     = 48000;
        test_tableSystemVoltage3.invalidMeasurement[s] = 0u;

        DATA_DatabaseBlockUpdatedWithinInterval_ExpectAndReturn(
            &test_tableSystemVoltage3.timestamp[s],
            &test_tableSystemVoltage3.previousTimestamp[s],
            s,
            PL_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms,
            true);
        DIAG_ReportResultToHandler_ExpectAndReturn(
            STD_OK, DIAG_ID_CURRENT_SENSOR_V3_MEASUREMENT_TIMEOUT, DIAG_STRING, s, STD_OK);
        BMS_IsStringClosed_ExpectAndReturn(s, false);
        BMS_IsStringPrecharging_ExpectAndReturn(s, false);
    }

    /* ======= RT5/5: call function under test */
    PL_ValidateHighVoltageBusMeasurement(&test_tablePackValues, &test_tableSystemVoltage3);

    /* ======= RT5/5: test output verification */
    TEST_ASSERT_EQUAL(1u, test_tablePackValues.invalidHvBusVoltage);
}
