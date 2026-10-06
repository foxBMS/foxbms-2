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
 * @file    test_validate-string-voltage.c
 * @author  foxBMS Team
 * @date    2026-07-23 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Validate string-voltage behavior
 * @details Test #PL_ValidateStringVoltageMeasurement.
 *          It verifies:
 *          - assertion handling for invalid input pointers,
 *          - plausibility checks between current-sensor and AFE-based values,
 *          - source-selection fallback when one measurement source is not
 *            usable,
 *          - reconstructed string-voltage behavior based on average cell
 *            voltage and number of invalid cell voltages,
 *          - validity decisions around allowed invalid-cell thresholds, and
 *          - expected diagnostic outcomes per string.
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

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/

/*========== Definitions and Implementations for Unit Test ==================*/
static DATA_BLOCK_PACK_VALUES_s test_tablePackValues          = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
static DATA_BLOCK_MIN_MAX_s test_tableMinimumMaximum          = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
static DATA_BLOCK_SYSTEM_VOLTAGE_1_s test_tableSystemVoltage1 = {.header.uniqueId = DATA_BLOCK_ID_SYSTEM_VOLTAGE_1};
static DATA_BLOCK_CELL_VOLTAGE_s test_tableCellVoltage        = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/**
 * @brief   Test #TEST_PL_CheckStringVoltage
 * @details Covers equal measurements, tolerance boundaries, and large values.
 */
void testPL_CheckStringVoltage(void) {
    int32_t voltageAfe_mV           = 0;
    int32_t voltageCurrentSensor_mV = 0;
    TEST_ASSERT_EQUAL(STD_OK, TEST_PL_CheckStringVoltage(voltageAfe_mV, voltageCurrentSensor_mV));

    voltageAfe_mV           = INT32_MAX;
    voltageCurrentSensor_mV = INT32_MAX;
    TEST_ASSERT_EQUAL(STD_OK, TEST_PL_CheckStringVoltage(voltageAfe_mV, voltageCurrentSensor_mV));

    voltageAfe_mV           = 0;
    voltageCurrentSensor_mV = PL_STRING_VOLTAGE_TOLERANCE_mV;
    TEST_ASSERT_EQUAL(STD_NOT_OK, TEST_PL_CheckStringVoltage(voltageAfe_mV, voltageCurrentSensor_mV));

    voltageCurrentSensor_mV = PL_STRING_VOLTAGE_TOLERANCE_mV - 1;
    TEST_ASSERT_EQUAL(STD_OK, TEST_PL_CheckStringVoltage(voltageAfe_mV, voltageCurrentSensor_mV));

    voltageCurrentSensor_mV = PL_STRING_VOLTAGE_TOLERANCE_mV + 1;
    TEST_ASSERT_EQUAL(STD_NOT_OK, TEST_PL_CheckStringVoltage(voltageAfe_mV, voltageCurrentSensor_mV));

    voltageAfe_mV           = 4242;
    voltageCurrentSensor_mV = 4242;
    TEST_ASSERT_EQUAL(STD_OK, TEST_PL_CheckStringVoltage(voltageAfe_mV, voltageCurrentSensor_mV));

    voltageAfe_mV           = 0;
    voltageCurrentSensor_mV = INT32_MAX;
    TEST_ASSERT_EQUAL(STD_NOT_OK, TEST_PL_CheckStringVoltage(voltageAfe_mV, voltageCurrentSensor_mV));

    voltageAfe_mV           = INT32_MAX - (PL_STRING_VOLTAGE_TOLERANCE_mV + 100);
    voltageCurrentSensor_mV = INT32_MAX;
    TEST_ASSERT_EQUAL(STD_NOT_OK, TEST_PL_CheckStringVoltage(voltageAfe_mV, voltageCurrentSensor_mV));

    voltageAfe_mV = INT32_MAX - (PL_STRING_VOLTAGE_TOLERANCE_mV + 1);
    TEST_ASSERT_EQUAL(STD_NOT_OK, TEST_PL_CheckStringVoltage(voltageAfe_mV, voltageCurrentSensor_mV));
}

/**
 * @brief   Test PL_ValidateStringVoltageMeasurement
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/4: pTablePackValues is NULL_PTR -> assert
 *            - AT2/4: pTableMinimumMaximumValues is NULL_PTR -> assert
 *            - AT3/4: pTableSystemVoltage1 is NULL_PTR -> assert
 *            - AT4/4: pTableCellVoltage is NULL_PTR -> assert
 *          - Routine validation:
 *            - RT1/6: Updated and all measurements valid, plausibility OK
 *                     -> current sensor string voltage is used and valid.
 *            - RT2/6: Updated and all measurements valid, plausibility NOT_OK
 *                     -> current sensor string voltage is used but marked invalid.
 *            - RT3/6: Plausibility precondition fails, current sensor valid
 *                     -> current sensor string voltage is used and valid.
 *            - RT4/6: Current sensor invalid, AFE valid
 *                     -> AFE string voltage is used and valid.
 *            - RT5/6: Current sensor and AFE not fully valid, reconstruction
 *                     with <= allowed invalid cells
 *                     -> reconstructed string voltage is used and valid.
 *            - RT6/6: Current sensor and AFE not fully valid, reconstruction
 *                     with > allowed invalid cells
 *                     -> reconstructed string voltage is used and invalid.
 */
void testPL_ValidateStringVoltageMeasurement(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/4 */
    TEST_ASSERT_FAIL_ASSERT(PL_ValidateStringVoltageMeasurement(NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR));
    /* ======= AT2/4 */
    TEST_ASSERT_FAIL_ASSERT(PL_ValidateStringVoltageMeasurement(&test_tablePackValues, NULL_PTR, NULL_PTR, NULL_PTR));
    /* ======= AT3/4 */
    TEST_ASSERT_FAIL_ASSERT(
        PL_ValidateStringVoltageMeasurement(&test_tablePackValues, &test_tableMinimumMaximum, NULL_PTR, NULL_PTR));
    /* ======= AT4/4 */
    TEST_ASSERT_FAIL_ASSERT(PL_ValidateStringVoltageMeasurement(
        &test_tablePackValues, &test_tableMinimumMaximum, &test_tableSystemVoltage1, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/6: Test implementation */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_tableSystemVoltage1.highVoltage_mV[s]     = 50000;
        test_tableSystemVoltage1.invalidMeasurement[s] = 0u;
        test_tableCellVoltage.stringVoltage_mV[s]      = 49800;
        test_tableCellVoltage.nrValidCellVoltages[s]   = BS_NR_OF_CELL_BLOCKS_PER_STRING;
        test_tablePackValues.invalidStringVoltage[s]   = 1u;
        test_tablePackValues.stringVoltage_mV[s]       = 0;

        DATA_DatabaseBlockUpdatedWithinInterval_ExpectAndReturn(
            &test_tableSystemVoltage1.timestamp[s],
            &test_tableSystemVoltage1.previousTimestamp[s],
            s,
            PL_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms,
            true);
        DIAG_ReportResultToHandler_ExpectAndReturn(
            STD_OK, DIAG_ID_CURRENT_SENSOR_V1_MEASUREMENT_TIMEOUT, DIAG_STRING, s, STD_OK);
        DIAG_ReportResultToHandler_ExpectAndReturn(STD_OK, DIAG_ID_BMS_VALUES_PACK_VOLTAGE, DIAG_STRING, s, STD_OK);
    }

    /* ======= RT1/6: call function under test */
    PL_ValidateStringVoltageMeasurement(
        &test_tablePackValues, &test_tableMinimumMaximum, &test_tableSystemVoltage1, &test_tableCellVoltage);

    /* ======= RT1/6: test output verification */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL(50000, test_tablePackValues.stringVoltage_mV[s]);
        TEST_ASSERT_EQUAL(0u, test_tablePackValues.invalidStringVoltage[s]);
    }

    /* ======= RT2/6: Test implementation */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_tableSystemVoltage1.highVoltage_mV[s]     = 50000;
        test_tableSystemVoltage1.invalidMeasurement[s] = 0u;
        test_tableCellVoltage.stringVoltage_mV[s]      = 50000 - (PL_STRING_VOLTAGE_TOLERANCE_mV + 1);
        test_tableCellVoltage.nrValidCellVoltages[s]   = BS_NR_OF_CELL_BLOCKS_PER_STRING;

        DATA_DatabaseBlockUpdatedWithinInterval_ExpectAndReturn(
            &test_tableSystemVoltage1.timestamp[s],
            &test_tableSystemVoltage1.previousTimestamp[s],
            s,
            PL_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms,
            true);
        DIAG_ReportResultToHandler_ExpectAndReturn(
            STD_OK, DIAG_ID_CURRENT_SENSOR_V1_MEASUREMENT_TIMEOUT, DIAG_STRING, s, STD_OK);
        DIAG_ReportResultToHandler_ExpectAndReturn(STD_NOT_OK, DIAG_ID_BMS_VALUES_PACK_VOLTAGE, DIAG_STRING, s, STD_OK);
    }

    /* ======= RT2/6: call function under test */
    PL_ValidateStringVoltageMeasurement(
        &test_tablePackValues, &test_tableMinimumMaximum, &test_tableSystemVoltage1, &test_tableCellVoltage);

    /* ======= RT2/6: test output verification */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL(50000, test_tablePackValues.stringVoltage_mV[s]);
        TEST_ASSERT_EQUAL(1u, test_tablePackValues.invalidStringVoltage[s]);
    }

    /* ======= RT3/6: Test implementation */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_tableSystemVoltage1.highVoltage_mV[s]     = 48000;
        test_tableSystemVoltage1.invalidMeasurement[s] = 0u;
        test_tableCellVoltage.stringVoltage_mV[s]      = 47000;
        test_tableCellVoltage.nrValidCellVoltages[s]   = (uint16_t)(BS_NR_OF_CELL_BLOCKS_PER_STRING - 1u);

        DATA_DatabaseBlockUpdatedWithinInterval_ExpectAndReturn(
            &test_tableSystemVoltage1.timestamp[s],
            &test_tableSystemVoltage1.previousTimestamp[s],
            s,
            PL_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms,
            true);
        DIAG_ReportResultToHandler_ExpectAndReturn(
            STD_OK, DIAG_ID_CURRENT_SENSOR_V1_MEASUREMENT_TIMEOUT, DIAG_STRING, s, STD_OK);
        DIAG_ReportResultToHandler_ExpectAndReturn(STD_NOT_OK, DIAG_ID_BMS_VALUES_PACK_VOLTAGE, DIAG_STRING, s, STD_OK);
    }

    /* ======= RT3/6: call function under test */
    PL_ValidateStringVoltageMeasurement(
        &test_tablePackValues, &test_tableMinimumMaximum, &test_tableSystemVoltage1, &test_tableCellVoltage);

    /* ======= RT3/6: test output verification */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL(48000, test_tablePackValues.stringVoltage_mV[s]);
        TEST_ASSERT_EQUAL(0u, test_tablePackValues.invalidStringVoltage[s]);
    }

    /* ======= RT4/6: Test implementation */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_tableSystemVoltage1.highVoltage_mV[s]     = 47000;
        test_tableSystemVoltage1.invalidMeasurement[s] = 1u;
        test_tableCellVoltage.stringVoltage_mV[s]      = 46500;
        test_tableCellVoltage.nrValidCellVoltages[s]   = BS_NR_OF_CELL_BLOCKS_PER_STRING;

        DATA_DatabaseBlockUpdatedWithinInterval_ExpectAndReturn(
            &test_tableSystemVoltage1.timestamp[s],
            &test_tableSystemVoltage1.previousTimestamp[s],
            s,
            PL_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms,
            true);
        DIAG_ReportResultToHandler_ExpectAndReturn(
            STD_OK, DIAG_ID_CURRENT_SENSOR_V1_MEASUREMENT_TIMEOUT, DIAG_STRING, s, STD_OK);
        DIAG_ReportResultToHandler_ExpectAndReturn(STD_NOT_OK, DIAG_ID_BMS_VALUES_PACK_VOLTAGE, DIAG_STRING, s, STD_OK);
    }

    /* ======= RT4/6: call function under test */
    PL_ValidateStringVoltageMeasurement(
        &test_tablePackValues, &test_tableMinimumMaximum, &test_tableSystemVoltage1, &test_tableCellVoltage);

    /* ======= RT4/6: test output verification */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL(46500, test_tablePackValues.stringVoltage_mV[s]);
        TEST_ASSERT_EQUAL(0u, test_tablePackValues.invalidStringVoltage[s]);
    }

    /* ======= RT5/6: Test implementation */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_tableSystemVoltage1.highVoltage_mV[s]     = 46000;
        test_tableSystemVoltage1.invalidMeasurement[s] = 1u;
        test_tableCellVoltage.stringVoltage_mV[s]      = 40000;
        test_tableCellVoltage.nrValidCellVoltages[s] =
            (uint16_t)(BS_NR_OF_CELL_BLOCKS_PER_STRING - PL_ALLOWED_NUMBER_OF_INVALID_CELL_VOLTAGES);
        test_tableMinimumMaximum.averageCellVoltage_mV[s] = 2000;

        DATA_DatabaseBlockUpdatedWithinInterval_ExpectAndReturn(
            &test_tableSystemVoltage1.timestamp[s],
            &test_tableSystemVoltage1.previousTimestamp[s],
            s,
            PL_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms,
            false);
        DIAG_ReportResultToHandler_ExpectAndReturn(
            STD_NOT_OK, DIAG_ID_CURRENT_SENSOR_V1_MEASUREMENT_TIMEOUT, DIAG_STRING, s, STD_OK);
        DIAG_ReportResultToHandler_ExpectAndReturn(STD_NOT_OK, DIAG_ID_BMS_VALUES_PACK_VOLTAGE, DIAG_STRING, s, STD_OK);
    }

    /* ======= RT5/6: call function under test */
    PL_ValidateStringVoltageMeasurement(
        &test_tablePackValues, &test_tableMinimumMaximum, &test_tableSystemVoltage1, &test_tableCellVoltage);

    /* ======= RT5/6: test output verification */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL(50000, test_tablePackValues.stringVoltage_mV[s]);
        TEST_ASSERT_EQUAL(0u, test_tablePackValues.invalidStringVoltage[s]);
    }

    /* ======= RT6/6: Test implementation */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_tableSystemVoltage1.highVoltage_mV[s]     = 46000;
        test_tableSystemVoltage1.invalidMeasurement[s] = 1u;
        test_tableCellVoltage.stringVoltage_mV[s]      = 40000;
        test_tableCellVoltage.nrValidCellVoltages[s] =
            (uint16_t)(BS_NR_OF_CELL_BLOCKS_PER_STRING - (PL_ALLOWED_NUMBER_OF_INVALID_CELL_VOLTAGES + 1u));
        test_tableMinimumMaximum.averageCellVoltage_mV[s] = 2000;

        DATA_DatabaseBlockUpdatedWithinInterval_ExpectAndReturn(
            &test_tableSystemVoltage1.timestamp[s],
            &test_tableSystemVoltage1.previousTimestamp[s],
            s,
            PL_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms,
            false);
        DIAG_ReportResultToHandler_ExpectAndReturn(
            STD_NOT_OK, DIAG_ID_CURRENT_SENSOR_V1_MEASUREMENT_TIMEOUT, DIAG_STRING, s, STD_OK);
        DIAG_ReportResultToHandler_ExpectAndReturn(STD_NOT_OK, DIAG_ID_BMS_VALUES_PACK_VOLTAGE, DIAG_STRING, s, STD_OK);
    }

    /* ======= RT6/6: call function under test */
    PL_ValidateStringVoltageMeasurement(
        &test_tablePackValues, &test_tableMinimumMaximum, &test_tableSystemVoltage1, &test_tableCellVoltage);

    /* ======= RT6/6: test output verification */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL(52000, test_tablePackValues.stringVoltage_mV[s]);
        TEST_ASSERT_EQUAL(1u, test_tablePackValues.invalidStringVoltage[s]);
    }
}
