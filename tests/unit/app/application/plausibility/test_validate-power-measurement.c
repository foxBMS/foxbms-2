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
 * @file    test_validate-power-measurement.c
 * @author  foxBMS Team
 * @date    2026-07-23 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Validate power-measurement behavior
 * @details Test #PL_ValidatePowerMeasurement.
 *          It covers:
 *          - assertion handling for invalid input pointers,
 *          - direct measured-value usage when fresh and valid,
 *          - fallback power calculation from string current and string
 *            voltage when measurement values are stale or invalid,
 *          - branch behavior when fallback prerequisites are not met,
 *          - mixed-string scenarios combining fallback and measured values,
 *          - string/pack power validity propagation and aggregation, and
 *          - expected diagnostic events for valid and invalid paths.
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdatabase_helper.h"
#include "Mockdiag.h"

#include "database_cfg.h"
#include "diag_cfg.h"

#include "fstd_types.h"
#include "plausibility.h"
#include "test_assert_helper.h"

#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/

/*========== Definitions and Implementations for Unit Test ==================*/
static DATA_BLOCK_PACK_VALUES_s test_tablePackValues = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
static DATA_BLOCK_POWER_s test_tablePower            = {.header.uniqueId = DATA_BLOCK_ID_POWER};

BMSVL_STATE_s test_pBmsvlState;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/**
 * @brief   Test PL_ValidatePowerMeasurement
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/3: pVsState is NULL_PTR -> assert
 *            - AT2/3: pTablePackValues is NULL_PTR -> assert
 *            - AT3/3: pTablePower is NULL_PTR -> assert
 *          - Routine validation: see dedicated test functions below.
 *            - RT1/5: Updated within timeout and power measurement valid
 *                     -> string power valid and copied from measurement,
 *                        pack power valid and equal to sum of string powers.
 *            - RT2/5: Updated within timeout, power measurement invalid but
 *                     current and voltage valid
 *                     -> string power calculated from I*U, pack power valid.
 *            - RT3/5: Not updated within timeout, current and voltage valid
 *                     -> string power calculated from I*U, pack power valid.
 *            - RT4/5: Not updated within timeout and no fallback calculation
 *                     possible
 *                     -> string power invalid, pack power invalid.
 *            - RT5/5: Updated within timeout but timestamp unchanged
 *                     -> previous valid values are preserved.
 *            - RT6/6: Mixed string case (first string fallback calculation,
 *                     following strings valid measured power)
 *                     -> fallback must not affect following strings.
 *            - RT7/7: Fallback requested with valid current but invalid
 *                     string voltage
 *                     -> no fallback calculation, string and pack power stay
 *                        invalid.
 */
void testPL_ValidatePowerMeasurement(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/3 */
    TEST_ASSERT_FAIL_ASSERT(PL_ValidatePowerMeasurement(NULL_PTR, NULL_PTR, NULL_PTR));
    /* ======= AT2/3 */
    TEST_ASSERT_FAIL_ASSERT(PL_ValidatePowerMeasurement(&test_pBmsvlState, NULL_PTR, NULL_PTR));
    /* ======= AT3/3 */
    TEST_ASSERT_FAIL_ASSERT(PL_ValidatePowerMeasurement(&test_pBmsvlState, &test_tablePackValues, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/5: Test implementation */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_tablePower.timestamp[s]                 = 100u;
        test_tablePower.power_W[s]                   = 100;
        test_tablePower.invalidMeasurement[s]        = 0u;
        test_pBmsvlState.lastStringPowerTimestamp[s] = 0u;
        test_tablePackValues.invalidStringPower[s]   = 1u;
        test_tablePackValues.stringPower_W[s]        = 0;
        test_tablePackValues.invalidStringCurrent[s] = 0u;
        test_tablePackValues.invalidStringVoltage[s] = 0u;

        DATA_DatabaseBlockUpdatedWithinInterval_ExpectAndReturn(
            &test_tablePower.timestamp[s],
            &test_tablePower.previousTimestamp[s],
            s,
            PL_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms,
            true);
        DIAG_ReportResultToHandler_ExpectAndReturn(
            STD_OK, DIAG_ID_CURRENT_SENSOR_POWER_MEASUREMENT_TIMEOUT, DIAG_STRING, s, STD_OK);
        DIAG_Handler_ExpectAndReturn(
            DIAG_ID_POWER_MEASUREMENT_ERROR, DIAG_EVENT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
    }

    /* ======= RT1/5: call function under test */
    PL_ValidatePowerMeasurement(&test_pBmsvlState, &test_tablePackValues, &test_tablePower);

    /* ======= RT1/5: test output verification */
    TEST_ASSERT_EQUAL(0u, test_tablePackValues.invalidPackPower);
    TEST_ASSERT_EQUAL((int32_t)(100 * BS_NR_OF_STRINGS), test_tablePackValues.packPower_W);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL(0u, test_tablePackValues.invalidStringPower[s]);
        TEST_ASSERT_EQUAL(100, test_tablePackValues.stringPower_W[s]);
        TEST_ASSERT_EQUAL(100u, test_pBmsvlState.lastStringPowerTimestamp[s]);
    }

    /* ======= RT2/5: Test implementation */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_tablePower.timestamp[s]                 = 200u;
        test_tablePower.power_W[s]                   = 999;
        test_tablePower.invalidMeasurement[s]        = 1u;
        test_pBmsvlState.lastStringPowerTimestamp[s] = 0u;
        test_tablePackValues.stringCurrent_mA[s]     = 2000;
        test_tablePackValues.stringVoltage_mV[s]     = 50000;
        test_tablePackValues.invalidStringCurrent[s] = 0u;
        test_tablePackValues.invalidStringVoltage[s] = 0u;
        test_tablePackValues.invalidStringPower[s]   = 1u;

        DATA_DatabaseBlockUpdatedWithinInterval_ExpectAndReturn(
            &test_tablePower.timestamp[s],
            &test_tablePower.previousTimestamp[s],
            s,
            PL_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms,
            true);
        DIAG_ReportResultToHandler_ExpectAndReturn(
            STD_OK, DIAG_ID_CURRENT_SENSOR_POWER_MEASUREMENT_TIMEOUT, DIAG_STRING, s, STD_OK);
        DIAG_Handler_ExpectAndReturn(
            DIAG_ID_POWER_MEASUREMENT_ERROR, DIAG_EVENT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
    }

    /* ======= RT2/5: call function under test */
    PL_ValidatePowerMeasurement(&test_pBmsvlState, &test_tablePackValues, &test_tablePower);

    /* ======= RT2/5: test output verification */
    TEST_ASSERT_EQUAL(0u, test_tablePackValues.invalidPackPower);
    TEST_ASSERT_EQUAL((int32_t)(100 * BS_NR_OF_STRINGS), test_tablePackValues.packPower_W);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL(0u, test_tablePackValues.invalidStringPower[s]);
        TEST_ASSERT_EQUAL(100, test_tablePackValues.stringPower_W[s]);
        TEST_ASSERT_EQUAL(200u, test_pBmsvlState.lastStringPowerTimestamp[s]);
    }

    /* ======= RT3/5: Test implementation */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_tablePackValues.stringCurrent_mA[s]     = 3000;
        test_tablePackValues.stringVoltage_mV[s]     = 40000;
        test_tablePackValues.invalidStringCurrent[s] = 0u;
        test_tablePackValues.invalidStringVoltage[s] = 0u;
        test_tablePackValues.invalidStringPower[s]   = 1u;

        DATA_DatabaseBlockUpdatedWithinInterval_ExpectAndReturn(
            &test_tablePower.timestamp[s],
            &test_tablePower.previousTimestamp[s],
            s,
            PL_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms,
            false);
        DIAG_ReportResultToHandler_ExpectAndReturn(
            STD_NOT_OK, DIAG_ID_CURRENT_SENSOR_POWER_MEASUREMENT_TIMEOUT, DIAG_STRING, s, STD_OK);
        DIAG_Handler_ExpectAndReturn(
            DIAG_ID_POWER_MEASUREMENT_ERROR, DIAG_EVENT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
    }

    /* ======= RT3/5: call function under test */
    PL_ValidatePowerMeasurement(&test_pBmsvlState, &test_tablePackValues, &test_tablePower);

    /* ======= RT3/5: test output verification */
    TEST_ASSERT_EQUAL(0u, test_tablePackValues.invalidPackPower);
    TEST_ASSERT_EQUAL((int32_t)(120 * BS_NR_OF_STRINGS), test_tablePackValues.packPower_W);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL(0u, test_tablePackValues.invalidStringPower[s]);
        TEST_ASSERT_EQUAL(120, test_tablePackValues.stringPower_W[s]);
    }

    /* ======= RT4/5: Test implementation */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_tablePackValues.invalidStringCurrent[s] = 1u;
        test_tablePackValues.invalidStringVoltage[s] = 0u;
        test_tablePackValues.invalidStringPower[s]   = 1u;

        DATA_DatabaseBlockUpdatedWithinInterval_ExpectAndReturn(
            &test_tablePower.timestamp[s],
            &test_tablePower.previousTimestamp[s],
            s,
            PL_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms,
            false);
        DIAG_ReportResultToHandler_ExpectAndReturn(
            STD_NOT_OK, DIAG_ID_CURRENT_SENSOR_POWER_MEASUREMENT_TIMEOUT, DIAG_STRING, s, STD_OK);
        DIAG_Handler_ExpectAndReturn(
            DIAG_ID_POWER_MEASUREMENT_ERROR, DIAG_EVENT_NOT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
    }

    /* ======= RT4/5: call function under test */
    PL_ValidatePowerMeasurement(&test_pBmsvlState, &test_tablePackValues, &test_tablePower);

    /* ======= RT4/5: test output verification */
    TEST_ASSERT_EQUAL(1u, test_tablePackValues.invalidPackPower);
    TEST_ASSERT_EQUAL(0, test_tablePackValues.packPower_W);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL(1u, test_tablePackValues.invalidStringPower[s]);
    }

    /* ======= RT5/5: Test implementation */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_tablePower.timestamp[s]                 = 300u;
        test_tablePower.power_W[s]                   = 123;
        test_tablePower.invalidMeasurement[s]        = 0u;
        test_pBmsvlState.lastStringPowerTimestamp[s] = 300u;
        test_tablePackValues.stringPower_W[s]        = 55;
        test_tablePackValues.invalidStringPower[s]   = 0u;
        test_tablePackValues.invalidStringCurrent[s] = 0u;
        test_tablePackValues.invalidStringVoltage[s] = 0u;

        DATA_DatabaseBlockUpdatedWithinInterval_ExpectAndReturn(
            &test_tablePower.timestamp[s],
            &test_tablePower.previousTimestamp[s],
            s,
            PL_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms,
            true);
        DIAG_ReportResultToHandler_ExpectAndReturn(
            STD_OK, DIAG_ID_CURRENT_SENSOR_POWER_MEASUREMENT_TIMEOUT, DIAG_STRING, s, STD_OK);
        DIAG_Handler_ExpectAndReturn(
            DIAG_ID_POWER_MEASUREMENT_ERROR, DIAG_EVENT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
    }

    /* ======= RT5/5: call function under test */
    PL_ValidatePowerMeasurement(&test_pBmsvlState, &test_tablePackValues, &test_tablePower);

    /* ======= RT5/5: test output verification */
    TEST_ASSERT_EQUAL(0u, test_tablePackValues.invalidPackPower);
    TEST_ASSERT_EQUAL((int32_t)(55 * BS_NR_OF_STRINGS), test_tablePackValues.packPower_W);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL(0u, test_tablePackValues.invalidStringPower[s]);
        TEST_ASSERT_EQUAL(55, test_tablePackValues.stringPower_W[s]);
        TEST_ASSERT_EQUAL(300u, test_pBmsvlState.lastStringPowerTimestamp[s]);
    }

    /* ======= RT6/6: Test implementation */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_tablePower.timestamp[s]                 = (uint32_t)(400u + s);
        test_pBmsvlState.lastStringPowerTimestamp[s] = 0u;
        test_tablePackValues.invalidStringCurrent[s] = 0u;
        test_tablePackValues.invalidStringVoltage[s] = 0u;
        test_tablePackValues.invalidStringPower[s]   = 1u;

        if (s == 0u) {
            test_tablePower.power_W[s]               = 999;
            test_tablePower.invalidMeasurement[s]    = 1u;
            test_tablePackValues.stringCurrent_mA[s] = 1000;
            test_tablePackValues.stringVoltage_mV[s] = 30000;
        } else {
            test_tablePower.power_W[s]            = 200;
            test_tablePower.invalidMeasurement[s] = 0u;
            /* Chosen to differ from measured power in case of unintended fallback */
            test_tablePackValues.stringCurrent_mA[s] = 500;
            test_tablePackValues.stringVoltage_mV[s] = 100000;
        }

        DATA_DatabaseBlockUpdatedWithinInterval_ExpectAndReturn(
            &test_tablePower.timestamp[s],
            &test_tablePower.previousTimestamp[s],
            s,
            PL_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms,
            true);
        DIAG_ReportResultToHandler_ExpectAndReturn(
            STD_OK, DIAG_ID_CURRENT_SENSOR_POWER_MEASUREMENT_TIMEOUT, DIAG_STRING, s, STD_OK);
        DIAG_Handler_ExpectAndReturn(
            DIAG_ID_POWER_MEASUREMENT_ERROR, DIAG_EVENT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
    }

    /* ======= RT6/6: call function under test */
    PL_ValidatePowerMeasurement(&test_pBmsvlState, &test_tablePackValues, &test_tablePower);

    /* ======= RT6/6: test output verification */
    TEST_ASSERT_EQUAL(0u, test_tablePackValues.invalidPackPower);
    TEST_ASSERT_EQUAL((int32_t)(30 + (200 * (BS_NR_OF_STRINGS - 1u))), test_tablePackValues.packPower_W);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL(0u, test_tablePackValues.invalidStringPower[s]);
        TEST_ASSERT_EQUAL((uint32_t)(400u + s), test_pBmsvlState.lastStringPowerTimestamp[s]);
        if (s == 0u) {
            TEST_ASSERT_EQUAL(30, test_tablePackValues.stringPower_W[s]);
        } else {
            TEST_ASSERT_EQUAL(200, test_tablePackValues.stringPower_W[s]);
        }
    }

    /* ======= RT7/7: Test implementation */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_tablePackValues.invalidStringCurrent[s] = 0u;
        test_tablePackValues.invalidStringVoltage[s] = 1u;
        test_tablePackValues.invalidStringPower[s]   = 1u;
        test_tablePackValues.stringPower_W[s]        = 999;

        DATA_DatabaseBlockUpdatedWithinInterval_ExpectAndReturn(
            &test_tablePower.timestamp[s],
            &test_tablePower.previousTimestamp[s],
            s,
            PL_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms,
            false);
        DIAG_ReportResultToHandler_ExpectAndReturn(
            STD_NOT_OK, DIAG_ID_CURRENT_SENSOR_POWER_MEASUREMENT_TIMEOUT, DIAG_STRING, s, STD_OK);
        DIAG_Handler_ExpectAndReturn(
            DIAG_ID_POWER_MEASUREMENT_ERROR, DIAG_EVENT_NOT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
    }

    /* ======= RT7/7: call function under test */
    PL_ValidatePowerMeasurement(&test_pBmsvlState, &test_tablePackValues, &test_tablePower);

    /* ======= RT7/7: test output verification */
    TEST_ASSERT_EQUAL(1u, test_tablePackValues.invalidPackPower);
    TEST_ASSERT_EQUAL(0, test_tablePackValues.packPower_W);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL(1u, test_tablePackValues.invalidStringPower[s]);
        TEST_ASSERT_EQUAL(999, test_tablePackValues.stringPower_W[s]);
    }
}
