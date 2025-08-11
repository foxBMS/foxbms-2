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
 * @file    test_redundancy.c
 * @author  foxBMS Team
 * @date    2020-07-31 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the redundancy module
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockbms.h"
#include "Mockdatabase.h"
#include "Mockdatabase_helper.h"
#include "Mockdiag.h"
#include "Mockos.h"
#include "Mockplausibility.h"

#include "foxmath.h"
#include "redundancy.h"
#include "test_assert_helper.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("redundancy.c")

TEST_INCLUDE_PATH("../../src/app/application/bms")
TEST_INCLUDE_PATH("../../src/app/application/plausibility")
TEST_INCLUDE_PATH("../../src/app/application/redundancy")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/contactor")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/sps")
TEST_INCLUDE_PATH("../../src/app/engine/database")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/task/config")

/*========== Definitions and Implementations for Unit Test ==================*/

DATA_BLOCK_CELL_VOLTAGE_s testCellVoltageBase        = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
DATA_BLOCK_CELL_VOLTAGE_s testCellVoltageRedundancy0 = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_REDUNDANCY0};

DATA_BLOCK_CELL_TEMPERATURE_s testCellTemperatureBase        = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
DATA_BLOCK_CELL_TEMPERATURE_s testCellTemperatureRedundancy0 = {
    .header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_REDUNDANCY0};

static inline void injectDatabaseEntries(void) {
    DATA_Read4DataBlocks_ExpectAndReturn(
        &testCellVoltageBase,
        &testCellVoltageRedundancy0,
        &testCellTemperatureBase,
        &testCellTemperatureRedundancy0,
        STD_OK);
}

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    testCellVoltageBase.header.timestamp            = 0;
    testCellVoltageRedundancy0.header.timestamp     = 0;
    testCellTemperatureBase.header.timestamp        = 0;
    testCellTemperatureRedundancy0.header.timestamp = 0;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/* test for null pointer */

void testMRC_MeasurementUpdatedAtLeastOnce(void) {
    /* Measurement has been updated once, after 10ms */
    uint32_t timestamp         = 10u;
    uint32_t previousTimestamp = 0u;
    TEST_ASSERT_TRUE(TEST_MRC_MeasurementUpdatedAtLeastOnce(timestamp, previousTimestamp));

    /* Measurement has been updated twice, first after 10ms, then after 50ms */
    timestamp         = 60u;
    previousTimestamp = 10u;
    TEST_ASSERT_TRUE(TEST_MRC_MeasurementUpdatedAtLeastOnce(timestamp, previousTimestamp));

    /* Measurement has been updated three times, first after 10ms, then after 50ms, then after 10ms */
    timestamp         = 70u;
    previousTimestamp = 60u;
    TEST_ASSERT_TRUE(TEST_MRC_MeasurementUpdatedAtLeastOnce(timestamp, previousTimestamp));

    /* Measurement has never been updated */
    timestamp         = 0u;
    previousTimestamp = 0u;
    TEST_ASSERT_FALSE(TEST_MRC_MeasurementUpdatedAtLeastOnce(timestamp, previousTimestamp));
}

void testMRC_MeasurementUpdatedRecently(void) {
    /* Always check always if database entry has been updated within the last 100ms */
    uint32_t timeDifference = 100u;

    /* Time difference: 50ms -> true */
    uint32_t timestamp         = 50u;
    uint32_t previousTimestamp = 0u;
    OS_GetTickCount_ExpectAndReturn(100u);
    TEST_ASSERT_EQUAL(TEST_MRC_MeasurementUpdatedRecently(timestamp, previousTimestamp, timeDifference), STD_OK);

    /* Time difference: 100ms -> true, but never updated */
    timestamp = 0u;
    OS_GetTickCount_ExpectAndReturn(100u);
    TEST_ASSERT_EQUAL(TEST_MRC_MeasurementUpdatedRecently(timestamp, previousTimestamp, timeDifference), STD_NOT_OK);

    /* Time difference: 101ms -> false */
    timestamp = 0u;
    OS_GetTickCount_ExpectAndReturn(101u);
    TEST_ASSERT_EQUAL(TEST_MRC_MeasurementUpdatedRecently(timestamp, previousTimestamp, timeDifference), STD_NOT_OK);

    /* Time difference: 63ms -> true */
    timestamp = 4937u;
    OS_GetTickCount_ExpectAndReturn(5000u);
    TEST_ASSERT_EQUAL(TEST_MRC_MeasurementUpdatedRecently(timestamp, previousTimestamp, timeDifference), STD_OK);

    /* Time difference: 50ms -> true */
    timestamp = UINT32_MAX;
    OS_GetTickCount_ExpectAndReturn(50u);
    TEST_ASSERT_EQUAL(TEST_MRC_MeasurementUpdatedRecently(timestamp, previousTimestamp, timeDifference), STD_OK);

    /* Time difference: 100ms -> true */
    timestamp = UINT32_MAX - 50u;
    OS_GetTickCount_ExpectAndReturn(49u);
    TEST_ASSERT_EQUAL(TEST_MRC_MeasurementUpdatedRecently(timestamp, previousTimestamp, timeDifference), STD_OK);

    /* Time difference: 101ms -> false */
    timestamp = UINT32_MAX - 50u;
    OS_GetTickCount_ExpectAndReturn(50u);
    TEST_ASSERT_EQUAL(TEST_MRC_MeasurementUpdatedRecently(timestamp, previousTimestamp, timeDifference), STD_NOT_OK);

    /* Time difference: UINT32_MAX - 50 -> false */
    timestamp = 50u;
    OS_GetTickCount_ExpectAndReturn(UINT32_MAX);
    TEST_ASSERT_EQUAL(TEST_MRC_MeasurementUpdatedRecently(timestamp, previousTimestamp, timeDifference), STD_NOT_OK);
}

void testMRC_CalculateCellVoltageMinMaxAverageNullPointer(void) {
    DATA_BLOCK_MIN_MAX_s dummy0 = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_CalculateCellVoltageMinMaxAverage(NULL_PTR, &dummy0));
    DATA_BLOCK_CELL_VOLTAGE_s dummy1 = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_CalculateCellVoltageMinMaxAverage(&dummy1, NULL_PTR));
}

void testMRC_CalculateCellTemperatureMinMaxAverageNullPointer(void) {
    DATA_BLOCK_MIN_MAX_s dummy0 = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_CalculateCellTemperatureMinMaxAverage(NULL_PTR, &dummy0));
    DATA_BLOCK_CELL_TEMPERATURE_s dummy1 = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_CalculateCellTemperatureMinMaxAverage(&dummy1, NULL_PTR));
}

void testMRC_ValidateCellVoltageNullPointer(void) {
    DATA_BLOCK_CELL_VOLTAGE_s dummy = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_ValidateCellVoltage(NULL_PTR, &dummy, &dummy));
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_ValidateCellVoltage(&dummy, NULL_PTR, &dummy));
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_ValidateCellVoltage(&dummy, &dummy, NULL_PTR));
}

void testMRC_UpdateCellVoltageValidationNullPointer(void) {
    DATA_BLOCK_CELL_VOLTAGE_s dummy = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_UpdateCellVoltageValidation(NULL_PTR, &dummy));
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_UpdateCellVoltageValidation(&dummy, NULL_PTR));
}

void testMRC_ValidateCellTemperatureNullPointer(void) {
    DATA_BLOCK_CELL_TEMPERATURE_s dummy = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_ValidateCellTemperature(NULL_PTR, &dummy, &dummy));
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_ValidateCellTemperature(&dummy, NULL_PTR, &dummy));
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_ValidateCellTemperature(&dummy, &dummy, NULL_PTR));
}

void testMRC_UpdateCellTemperatureValidationNullPointer(void) {
    DATA_BLOCK_CELL_TEMPERATURE_s dummy = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_UpdateCellTemperatureValidation(NULL_PTR, &dummy));
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_UpdateCellTemperatureValidation(&dummy, NULL_PTR));
}

/* test main function */
void testMRC_AfeMeasurementValidationTickZeroNothingToDo(void) {
    /* inject database entries into function */
    injectDatabaseEntries();

    /* database entries never written - */
    DATA_DatabaseEntryUpdatedAtLeastOnce_ExpectAndReturn(testCellVoltageRedundancy0.header, false);
    DATA_EntryUpdatedWithinInterval_ExpectAndReturn(
        testCellVoltageBase.header, MRC_AFE_MEASUREMENT_PERIOD_TIMEOUT_ms, false);
    DIAG_Handler_ExpectAndReturn(
        DIAG_ID_BASE_CELL_VOLTAGE_MEASUREMENT_TIMEOUT, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u, STD_OK);
    DATA_EntryUpdatedWithinInterval_ExpectAndReturn(
        testCellVoltageRedundancy0.header, MRC_AFE_MEASUREMENT_PERIOD_TIMEOUT_ms, false);
    DIAG_Handler_ExpectAndReturn(
        DIAG_ID_REDUNDANCY0_CELL_VOLTAGE_MEASUREMENT_TIMEOUT, DIAG_EVENT_OK, DIAG_SYSTEM, 0u, STD_OK);

    DATA_DatabaseEntryUpdatedAtLeastOnce_ExpectAndReturn(testCellTemperatureRedundancy0.header, false);
    DATA_EntryUpdatedWithinInterval_ExpectAndReturn(
        testCellTemperatureBase.header, MRC_AFE_MEASUREMENT_PERIOD_TIMEOUT_ms, false);
    DIAG_Handler_ExpectAndReturn(
        DIAG_ID_BASE_CELL_TEMPERATURE_MEASUREMENT_TIMEOUT, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u, STD_OK);
    DATA_EntryUpdatedWithinInterval_ExpectAndReturn(
        testCellTemperatureRedundancy0.header, MRC_AFE_MEASUREMENT_PERIOD_TIMEOUT_ms, false);
    DIAG_Handler_ExpectAndReturn(
        DIAG_ID_REDUNDANCY0_CELL_TEMPERATURE_MEASUREMENT_TIMEOUT, DIAG_EVENT_OK, DIAG_SYSTEM, 0u, STD_OK);

    TEST_ASSERT_PASS_ASSERT(TEST_ASSERT_EQUAL(STD_NOT_OK, MRC_ValidateAfeMeasurement()));
}

void testMRC_ValidateCellVoltageMeasurement(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_ValidateCellVoltageMeasurement(NULL_PTR, &testCellVoltageRedundancy0));
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_ValidateCellVoltageMeasurement(&testCellVoltageBase, NULL_PTR));
}
