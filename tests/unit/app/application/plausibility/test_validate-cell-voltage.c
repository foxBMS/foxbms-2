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
 * @file    test_validate-cell-voltage.c
 * @author  foxBMS Team
 * @date    2026-07-23 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Validate cell-voltage behavior
 * @details Test #PL_CheckVoltageSpread.
 *          It verifies:
 *          - assertion handling for invalid function arguments,
 *          - spread-based invalidation for voltages above tolerance,
 *          - preservation of valid voltages within tolerance,
 *          - updates of per-string valid-cell counters, and
 *          - per-string diagnostic signaling for plausibility outcomes.
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
static DATA_BLOCK_CELL_VOLTAGE_s test_cellVoltages = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
static DATA_BLOCK_MIN_MAX_s test_minMax            = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    /* Bring all voltages to a defined, valid state (== average) */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_minMax.averageCellVoltage_mV[s]     = 2250;
        test_cellVoltages.nrValidCellVoltages[s] = 0u;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                test_cellVoltages.cellVoltage_mV[s][m][cb]     = 2250;
                test_cellVoltages.invalidCellVoltage[s][m][cb] = false;
            }
        }
    }
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/* Externalized Static Functions */

/**
 * @brief   Test PL_CheckIndividualCellVoltage
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - none
 *          - Routine validation:
 *            - RT1/2: The absolute difference between cell and average
 *                     voltage is within
 *                     #PL_CELL_VOLTAGE_SPREAD_TOLERANCE_mV, therefore the
 *                     function must return #STD_OK.
 *            - RT2/2: The absolute difference between cell and average
 *                     voltage is greater than
 *                     #PL_CELL_VOLTAGE_SPREAD_TOLERANCE_mV, therefore
 *                     the function must return #STD_NOT_OK.
 */
void testPL_CheckIndividualCellVoltageWithinTolerance(void) {
    /* ======= Assertion tests ============================================= */
    /* none */

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    int16_t averageCellVoltage_mV = 2250;
    /* Difference exactly at the tolerance limit is still valid (not '>') */
    const int16_t cellVoltageOk0_mV = averageCellVoltage_mV + PL_CELL_VOLTAGE_SPREAD_TOLERANCE_mV;
    /* ======= RT1/2: call function under test */
    const STD_RETURN_TYPE_e resultOk0 = TEST_PL_CheckIndividualCellVoltage(cellVoltageOk0_mV, averageCellVoltage_mV);
    /* ======= RT1/2: test output verification */
    TEST_ASSERT_EQUAL(STD_OK, resultOk0);

    /* Also check clearly within tolerance */
    const int16_t cellVoltageOk1_mV   = averageCellVoltage_mV;
    const STD_RETURN_TYPE_e resultOk1 = TEST_PL_CheckIndividualCellVoltage(cellVoltageOk1_mV, averageCellVoltage_mV);
    TEST_ASSERT_EQUAL(STD_OK, resultOk1);

    /* ======= RT2/2: Test implementation */
    /* Positive deviation above tolerance */
    const int16_t cellVoltageNotOkAbove_mV = averageCellVoltage_mV + PL_CELL_VOLTAGE_SPREAD_TOLERANCE_mV + 1;
    /* ======= RT2/2: call function under test */
    const STD_RETURN_TYPE_e resultNotOkAbove =
        TEST_PL_CheckIndividualCellVoltage(cellVoltageNotOkAbove_mV, averageCellVoltage_mV);
    TEST_ASSERT_EQUAL(STD_NOT_OK, resultNotOkAbove);

    /* Negative deviation above tolerance (tests abs()) */
    const int16_t cellVoltageNotOkBelow_mV = averageCellVoltage_mV - PL_CELL_VOLTAGE_SPREAD_TOLERANCE_mV - 1;
    const STD_RETURN_TYPE_e resultNotOkBelow =
        TEST_PL_CheckIndividualCellVoltage(cellVoltageNotOkBelow_mV, averageCellVoltage_mV);
    TEST_ASSERT_EQUAL(STD_NOT_OK, resultNotOkBelow);
}

/* Extern Functions */
/**
 * @brief   Test PL_CheckVoltageSpread
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: pCellVoltages is NULL_PTR -> assert
 *            - AT2/2: pMinMaxAverageValues is NULL_PTR -> assert
 *          - Routine validation: see dedicated test functions below.
 *            - RT1/3: The function must return #STD_OK, must not mark any
 *                     cell voltage invalid and must count all cell blocks as
 *                     valid.
 *                     DIAG_ReportResultToHandler must be called once per string with
 *                     #STD_OK.
 *            - RT2/3: The affected cell block must be marked invalid, the
 *                     function must return #STD_NOT_OK and the cell block must
 *                     not be counted as valid.
 *            - RT3/3: A cell block that is already flagged invalid must not
 *                     be checked nor counted as valid, and the overall result
 *                     stays #STD_OK when all remaining valid cell blocks are
 *                     within tolerance.
 */
void testPL_CheckVoltageSpread(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 */
    TEST_ASSERT_FAIL_ASSERT(PL_CheckVoltageSpread(NULL_PTR, &test_minMax));
    /* ======= AT2/2 */
    TEST_ASSERT_FAIL_ASSERT(PL_CheckVoltageSpread(&test_cellVoltages, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/3: Test implementation */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        DIAG_ReportResultToHandler_ExpectAndReturn(
            STD_OK, DIAG_ID_BMS_VALUES_CELL_VOLTAGE_SPREAD, DIAG_STRING, s, STD_OK);
    }

    /* ======= RT1/3: call function under test */
    const STD_RETURN_TYPE_e result1 = PL_CheckVoltageSpread(&test_cellVoltages, &test_minMax);

    /* ======= RT1/3: test output verification */
    TEST_ASSERT_EQUAL(STD_OK, result1);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL(
            (uint16_t)(BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_CELL_BLOCKS_PER_MODULE),
            test_cellVoltages.nrValidCellVoltages[s]);
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                TEST_ASSERT_FALSE(test_cellVoltages.invalidCellVoltage[s][m][cb]);
            }
        }
    }

    /* ======= RT2/3: Test implementation */
    /* Push string 0, module 0, sensor 0 out of tolerance */
    test_cellVoltages.cellVoltage_mV[0][0][0] = test_minMax.averageCellVoltage_mV[0] +
                                                PL_CELL_VOLTAGE_SPREAD_TOLERANCE_mV + 1;

    /* String 0 reports STD_NOT_OK, all other strings report STD_OK */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        STD_RETURN_TYPE_e expectedStringResult = STD_OK;
        if (s == 0u) {
            expectedStringResult = STD_NOT_OK;
        }
        DIAG_ReportResultToHandler_ExpectAndReturn(
            expectedStringResult, DIAG_ID_BMS_VALUES_CELL_VOLTAGE_SPREAD, DIAG_STRING, s, STD_OK);
    }

    /* ======= RT2/3: call function under test */
    const STD_RETURN_TYPE_e result2 = PL_CheckVoltageSpread(&test_cellVoltages, &test_minMax);

    /* ======= RT2/3: test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, result2);
    TEST_ASSERT_TRUE(test_cellVoltages.invalidCellVoltage[0][0][0]);
    TEST_ASSERT_EQUAL(
        (uint16_t)((BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_CELL_BLOCKS_PER_MODULE) - 1u),
        test_cellVoltages.nrValidCellVoltages[0]);

    /* ======= RT3/3: Test implementation */
    /* Sensor is already invalid and holds a value that WOULD fail the spread
     * check -> it must be skipped, so no issue is detected any more */
    test_cellVoltages.invalidCellVoltage[0][0][0] = true;
    test_cellVoltages.cellVoltage_mV[0][0][0]     = test_minMax.averageCellVoltage_mV[0] +
                                                    PL_CELL_VOLTAGE_SPREAD_TOLERANCE_mV + 100;

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        DIAG_ReportResultToHandler_ExpectAndReturn(
            STD_OK, DIAG_ID_BMS_VALUES_CELL_VOLTAGE_SPREAD, DIAG_STRING, s, STD_OK);
    }

    /* ======= RT3/3: call function under test */
    const STD_RETURN_TYPE_e result3 = PL_CheckVoltageSpread(&test_cellVoltages, &test_minMax);

    /* ======= RT3/3: test output verification */
    TEST_ASSERT_EQUAL(STD_OK, result3);
    TEST_ASSERT_TRUE(test_cellVoltages.invalidCellVoltage[0][0][0]);
    TEST_ASSERT_EQUAL(
        (uint16_t)((BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_CELL_BLOCKS_PER_MODULE) - 1u),
        test_cellVoltages.nrValidCellVoltages[0]);
}
