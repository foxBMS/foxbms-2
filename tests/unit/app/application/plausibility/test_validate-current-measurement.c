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
 * @file    test_validate-current-measurement.c
 * @author  foxBMS Team
 * @date    2026-07-23 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Validate current-measurement behavior
 * @details Test #PL_ValidateCurrentMeasurement.
 *          Covered behavior includes:
 *          - assertion handling for invalid input pointers,
 *          - timeout detection and stale-data handling,
 *          - timestamp-freshness checks for update processing,
 *          - propagation of measurement validity to string and pack flags,
 *          - aggregation of pack current from valid strings only, and
 *          - expected diagnostic events for valid and invalid branches.
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdatabase_helper.h"
#include "Mockdiag.h"

#include "database_cfg.h"
#include "diag_cfg.h"

#include "bms-values.h"
#include "fstd_types.h"
#include "plausibility.h"
#include "test_assert_helper.h"

#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/

/*========== Definitions and Implementations for Unit Test ==================*/
static DATA_BLOCK_PACK_VALUES_s test_tablePackValues = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
static DATA_BLOCK_CURRENT_s test_tableCurrent        = {.header.uniqueId = DATA_BLOCK_ID_CURRENT};

BMSVL_STATE_s test_pBmsvlState;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/* Extern Functions */
/**
 * @brief   Test PL_ValidateCurrentMeasurement
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/3: pBmsvlState is NULL_PTR -> assert
 *            - AT2/3: pTablePackValues is NULL_PTR -> assert
 *            - AT3/3: pTableCurrent is NULL_PTR -> assert
 *          - Routine validation: see dedicated test functions below.
 *            - RT1/4: All strings updated within timeout and measurement valid
 *                     -> all string currents valid, pack current valid and
 *                        equal to the sum of the string currents.
 *            - RT2/4: All strings updated within timeout but measurement flagged
 *                     invalid -> all string currents invalid, pack current
 *                        invalid, pack current == 0.
 *            - RT3/4: Measurement timeout reached (not updated within interval)
 *                     -> string current invalid, pack current invalid,
 *                        DIAG_Handler not called.
 *            - RT4/4: Updated within timeout but timestamp unchanged
 *                     -> previous (valid) state is preserved, DIAG_Handler not
 *                        called.
 */
void testPL_ValidateCurrentMeasurement(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/3 */
    TEST_ASSERT_FAIL_ASSERT(PL_ValidateCurrentMeasurement(NULL_PTR, NULL_PTR, NULL_PTR));
    /* ======= AT2/3 */
    TEST_ASSERT_FAIL_ASSERT(PL_ValidateCurrentMeasurement(&test_pBmsvlState, NULL_PTR, NULL_PTR));
    /* ======= AT3/3 */
    TEST_ASSERT_FAIL_ASSERT(PL_ValidateCurrentMeasurement(&test_pBmsvlState, &test_tablePackValues, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/4: Test implementation */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_tableCurrent.timestamp[s]                 = 100u;
        test_tableCurrent.current_mA[s]                = 1000;
        test_tableCurrent.invalidMeasurement[s]        = 0u;
        test_pBmsvlState.lastStringCurrentTimestamp[s] = 0u; /* force "new" measurement */

        DATA_DatabaseBlockUpdatedWithinInterval_ExpectAndReturn(
            &test_tableCurrent.timestamp[s],
            &test_tableCurrent.previousTimestamp[s],
            s,
            PL_CURRENT_MEASUREMENT_PERIOD_TIMEOUT_ms,
            true);
        DIAG_ReportResultToHandler_ExpectAndReturn(STD_OK, DIAG_ID_CURRENT_MEASUREMENT_TIMEOUT, DIAG_STRING, s, STD_OK);
        DIAG_Handler_ExpectAndReturn(
            DIAG_ID_CURRENT_MEASUREMENT_ERROR, DIAG_EVENT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
    }

    /* ======= RT1/4: call function under test */
    PL_ValidateCurrentMeasurement(&test_pBmsvlState, &test_tablePackValues, &test_tableCurrent);

    /* ======= RT1/4: test output verification */
    TEST_ASSERT_EQUAL(0u, test_tablePackValues.invalidPackCurrent);
    TEST_ASSERT_EQUAL((int32_t)(1000 * BS_NR_OF_STRINGS), test_tablePackValues.packCurrent_mA);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL(0u, test_tablePackValues.invalidStringCurrent[s]);
        TEST_ASSERT_EQUAL(1000, test_tablePackValues.stringCurrent_mA[s]);
        TEST_ASSERT_EQUAL(100u, test_pBmsvlState.lastStringCurrentTimestamp[s]);
    }

    /* ======= RT2/4: Test implementation */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_tableCurrent.timestamp[s]                 = 100u;
        test_tableCurrent.current_mA[s]                = 1000;
        test_tableCurrent.invalidMeasurement[s]        = 1u; /* measurement invalid */
        test_pBmsvlState.lastStringCurrentTimestamp[s] = 0u;

        DATA_DatabaseBlockUpdatedWithinInterval_ExpectAndReturn(
            &test_tableCurrent.timestamp[s],
            &test_tableCurrent.previousTimestamp[s],
            s,
            PL_CURRENT_MEASUREMENT_PERIOD_TIMEOUT_ms,
            true);
        DIAG_ReportResultToHandler_ExpectAndReturn(STD_OK, DIAG_ID_CURRENT_MEASUREMENT_TIMEOUT, DIAG_STRING, s, STD_OK);
        DIAG_Handler_ExpectAndReturn(
            DIAG_ID_CURRENT_MEASUREMENT_ERROR, DIAG_EVENT_NOT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
    }

    /* ======= RT2/4: call function under test */
    PL_ValidateCurrentMeasurement(&test_pBmsvlState, &test_tablePackValues, &test_tableCurrent);

    /* ======= RT2/4: test output verification */
    TEST_ASSERT_EQUAL(1u, test_tablePackValues.invalidPackCurrent);
    TEST_ASSERT_EQUAL(0, test_tablePackValues.packCurrent_mA);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL(1u, test_tablePackValues.invalidStringCurrent[s]);
    }

    /* ======= RT3/4: Test implementation */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        DATA_DatabaseBlockUpdatedWithinInterval_ExpectAndReturn(
            &test_tableCurrent.timestamp[s],
            &test_tableCurrent.previousTimestamp[s],
            s,
            PL_CURRENT_MEASUREMENT_PERIOD_TIMEOUT_ms,
            false);
        /* STD_BoolToStdReturnType(false) == STD_NOT_OK */
        DIAG_ReportResultToHandler_ExpectAndReturn(
            STD_NOT_OK, DIAG_ID_CURRENT_MEASUREMENT_TIMEOUT, DIAG_STRING, s, STD_OK);
        /* DIAG_Handler must NOT be called in this branch */
    }

    /* ======= RT3/4: call function under test */
    PL_ValidateCurrentMeasurement(&test_pBmsvlState, &test_tablePackValues, &test_tableCurrent);

    /* ======= RT3/4: test output verification */
    TEST_ASSERT_EQUAL(1u, test_tablePackValues.invalidPackCurrent);
    TEST_ASSERT_EQUAL(0, test_tablePackValues.packCurrent_mA);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL(1u, test_tablePackValues.invalidStringCurrent[s]);
    }

    /* ======= RT4/4: Test implementation */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_tableCurrent.timestamp[s]                 = 100u;
        test_pBmsvlState.lastStringCurrentTimestamp[s] = 100u; /* unchanged */
        test_tablePackValues.invalidStringCurrent[s]   = 0u;   /* previously valid */
        test_tablePackValues.stringCurrent_mA[s]       = 500;

        DATA_DatabaseBlockUpdatedWithinInterval_ExpectAndReturn(
            &test_tableCurrent.timestamp[s],
            &test_tableCurrent.previousTimestamp[s],
            s,
            PL_CURRENT_MEASUREMENT_PERIOD_TIMEOUT_ms,
            true);
        DIAG_ReportResultToHandler_ExpectAndReturn(STD_OK, DIAG_ID_CURRENT_MEASUREMENT_TIMEOUT, DIAG_STRING, s, STD_OK);
        /* DIAG_Handler must NOT be called since the timestamp did not change */
    }

    /* ======= RT4/4: call function under test */
    PL_ValidateCurrentMeasurement(&test_pBmsvlState, &test_tablePackValues, &test_tableCurrent);

    /* ======= RT4/4: test output verification */
    TEST_ASSERT_EQUAL(0u, test_tablePackValues.invalidPackCurrent);
    TEST_ASSERT_EQUAL((int32_t)(500 * BS_NR_OF_STRINGS), test_tablePackValues.packCurrent_mA);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL(0u, test_tablePackValues.invalidStringCurrent[s]);
        TEST_ASSERT_EQUAL(500, test_tablePackValues.stringCurrent_mA[s]);
    }
}
