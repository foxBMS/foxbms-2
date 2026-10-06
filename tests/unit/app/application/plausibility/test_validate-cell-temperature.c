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
 * @file    test_validate-cell-temperature.c
 * @author  foxBMS Team
 * @date    2026-07-23 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Validate cell-temperature behavior
 * @details Test #PL_CheckTemperatureSpread.
 *          It validates:
 *          - assertion handling for invalid function arguments,
 *          - spread-based invalidation for values above tolerance,
 *          - preservation of valid samples within tolerance,
 *          - updates of per-string valid-temperature counters, and
 *          - per-string diagnostic signaling for detected plausibility issues.
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdiag.h"

#include "battery_system_cfg.h"
#include "database_cfg.h"
#include "diag_cfg.h"

#include "fstd_types.h"
#include "plausibility.h"
#include "test_assert_helper.h"

#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/

/*========== Definitions and Implementations for Unit Test ==================*/
static DATA_BLOCK_CELL_TEMPERATURE_s test_cellTemperatures = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
static DATA_BLOCK_MIN_MAX_s test_minMax                    = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    /* Bring all temperatures to a defined, valid state (== average) */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_minMax.averageTemperature_ddegC[s]      = 250;
        test_cellTemperatures.nrValidTemperatures[s] = 0u;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                test_cellTemperatures.cellTemperature_ddegC[s][m][ts]  = 250;
                test_cellTemperatures.invalidCellTemperature[s][m][ts] = false;
            }
        }
    }
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/* Externalized Static Functions */

/**
 * @brief   Test PL_CheckIndividualCellTemperature
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - none
 *          - Routine validation:
 *            - RT1/2: The absolute difference between cell and average
 *                     temperature is within
 *                     #PL_CELL_TEMPERATURE_SPREAD_TOLERANCE_dK, therefore the
 *                     function must return #STD_OK.
 *            - RT2/2: The absolute difference between cell and average
 *                     temperature is greater than
 *                     #PL_CELL_TEMPERATURE_SPREAD_TOLERANCE_dK, therefore
 *                     the function must return #STD_NOT_OK.
 */
void testPL_CheckIndividualCellTemperatureWithinTolerance(void) {
    /* ======= Assertion tests ============================================= */
    /* none */

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    int16_t averageTemperature_ddegC = 250;
    /* Difference exactly at the tolerance limit is still valid (not '>') */
    const int16_t cellTemperatureOk0_ddegC = averageTemperature_ddegC + PL_CELL_TEMPERATURE_SPREAD_TOLERANCE_dK;
    /* ======= RT1/2: call function under test */
    const STD_RETURN_TYPE_e resultOk0 =
        TEST_PL_CheckIndividualCellTemperature(cellTemperatureOk0_ddegC, averageTemperature_ddegC);
    /* ======= RT1/2: test output verification */
    TEST_ASSERT_EQUAL(STD_OK, resultOk0);

    /* Also check clearly within tolerance */
    const int16_t cellTemperatureOk1_ddegC = averageTemperature_ddegC;
    const STD_RETURN_TYPE_e resultOk1 =
        TEST_PL_CheckIndividualCellTemperature(cellTemperatureOk1_ddegC, averageTemperature_ddegC);
    TEST_ASSERT_EQUAL(STD_OK, resultOk1);

    /* ======= RT2/2: Test implementation */
    /* Positive deviation above tolerance */
    const int16_t cellTemperatureNotOkAbove_ddegC = averageTemperature_ddegC + PL_CELL_TEMPERATURE_SPREAD_TOLERANCE_dK +
                                                    1;
    /* ======= RT2/2: call function under test */
    const STD_RETURN_TYPE_e resultNotOkAbove =
        TEST_PL_CheckIndividualCellTemperature(cellTemperatureNotOkAbove_ddegC, averageTemperature_ddegC);
    TEST_ASSERT_EQUAL(STD_NOT_OK, resultNotOkAbove);

    /* Negative deviation above tolerance (tests abs()) */
    const int16_t cellTemperatureNg_ddegC = averageTemperature_ddegC - PL_CELL_TEMPERATURE_SPREAD_TOLERANCE_dK - 1;
    const STD_RETURN_TYPE_e resultNotOkBelow =
        TEST_PL_CheckIndividualCellTemperature(cellTemperatureNg_ddegC, averageTemperature_ddegC);
    TEST_ASSERT_EQUAL(STD_NOT_OK, resultNotOkBelow);
}

/* Extern Functions */
/**
 * @brief   Test PL_CheckTemperatureSpread
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: pCellTemperatures is NULL_PTR -> assert
 *            - AT2/2: pMinMaxAverageValues is NULL_PTR -> assert
 *          - Routine validation: see dedicated test functions below.
 *            - RT1/3: The function must return #STD_OK, must not mark any
 *                     temperature invalid and must count all sensors as valid.
 *                     DIAG_ReportResultToHandler must be called once per string with
 *                     #STD_OK.
 *            - RT2/3: The affected sensor must be marked invalid, the function
 *                     must return #STD_NOT_OK and the sensor must not be
 *                     counted as valid.
 *            - RT3/3: A temperature that is already flagged invalid must not
 *                     be checked nor counted as valid, and the overall result
 *                     stays #STD_OK when all remaining valid temperatures are
 *                     within tolerance.
 */
void testPL_CheckTemperatureSpread(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 */
    TEST_ASSERT_FAIL_ASSERT(PL_CheckTemperatureSpread(NULL_PTR, &test_minMax));
    /* ======= AT2/2 */
    TEST_ASSERT_FAIL_ASSERT(PL_CheckTemperatureSpread(&test_cellTemperatures, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/3: Test implementation */
    /* All temperatures equal the average -> no issue in any string */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        DIAG_ReportResultToHandler_ExpectAndReturn(
            STD_OK, DIAG_ID_BMS_VALUES_CELL_TEMPERATURE_SPREAD, DIAG_STRING, s, STD_OK);
    }

    /* ======= RT1/3: call function under test */
    const STD_RETURN_TYPE_e result1 = PL_CheckTemperatureSpread(&test_cellTemperatures, &test_minMax);

    /* ======= RT1/3: test output verification */
    TEST_ASSERT_EQUAL(STD_OK, result1);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL(
            (uint16_t)(BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_TEMP_SENSORS_PER_MODULE),
            test_cellTemperatures.nrValidTemperatures[s]);
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                TEST_ASSERT_FALSE(test_cellTemperatures.invalidCellTemperature[s][m][ts]);
            }
        }
    }

    /* ======= RT2/3: Test implementation */
    /* Push string 0, module 0, sensor 0 out of tolerance */
    test_cellTemperatures.cellTemperature_ddegC[0][0][0] = test_minMax.averageTemperature_ddegC[0] +
                                                           PL_CELL_TEMPERATURE_SPREAD_TOLERANCE_dK + 1;

    /* String 0 reports STD_NOT_OK, all other strings report STD_OK */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        STD_RETURN_TYPE_e expectedStringResult = STD_OK;
        if (s == 0u) {
            expectedStringResult = STD_NOT_OK;
        }
        DIAG_ReportResultToHandler_ExpectAndReturn(
            expectedStringResult, DIAG_ID_BMS_VALUES_CELL_TEMPERATURE_SPREAD, DIAG_STRING, s, STD_OK);
    }

    /* ======= RT2/3: call function under test */
    const STD_RETURN_TYPE_e result2 = PL_CheckTemperatureSpread(&test_cellTemperatures, &test_minMax);

    /* ======= RT2/3: test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, result2);
    TEST_ASSERT_TRUE(test_cellTemperatures.invalidCellTemperature[0][0][0]);
    TEST_ASSERT_EQUAL(
        (uint16_t)((BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_TEMP_SENSORS_PER_MODULE) - 1u),
        test_cellTemperatures.nrValidTemperatures[0]);

    /* ======= RT3/3: Test implementation */
    /* Sensor is already invalid and holds a value that WOULD fail the spread
     * check -> it must be skipped, so no issue is detected any more */
    test_cellTemperatures.invalidCellTemperature[0][0][0] = true;
    test_cellTemperatures.cellTemperature_ddegC[0][0][0]  = test_minMax.averageTemperature_ddegC[0] +
                                                            PL_CELL_TEMPERATURE_SPREAD_TOLERANCE_dK + 100;

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        DIAG_ReportResultToHandler_ExpectAndReturn(
            STD_OK, DIAG_ID_BMS_VALUES_CELL_TEMPERATURE_SPREAD, DIAG_STRING, s, STD_OK);
    }

    /* ======= RT3/3: call function under test */
    const STD_RETURN_TYPE_e result3 = PL_CheckTemperatureSpread(&test_cellTemperatures, &test_minMax);

    /* ======= RT3/3: test output verification */
    TEST_ASSERT_EQUAL(STD_OK, result3);
    TEST_ASSERT_TRUE(test_cellTemperatures.invalidCellTemperature[0][0][0]);
    TEST_ASSERT_EQUAL(
        (uint16_t)((BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_TEMP_SENSORS_PER_MODULE) - 1u),
        test_cellTemperatures.nrValidTemperatures[0]);
}
