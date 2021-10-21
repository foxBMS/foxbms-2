/**
 *
 * @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * - &Prime;This product uses parts of foxBMS&reg;&Prime;
 * - &Prime;This product includes parts of foxBMS&reg;&Prime;
 * - &Prime;This product is derived from foxBMS&reg;&Prime;
 *
 */

/**
 * @file    test_redundancy.c
 * @author  foxBMS Team
 * @date    2020-07-31 (date of creation)
 * @updated 2020-07-31 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the redundancy module
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

/*========== Definitions and Implementations for Unit Test ==================*/

DATA_BLOCK_CELL_VOLTAGE_s testCellvoltageBase        = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
DATA_BLOCK_CELL_VOLTAGE_s testCellvoltageRedundancy0 = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_REDUNDANCY0};

DATA_BLOCK_CELL_TEMPERATURE_s testCelltemperatureBase        = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
DATA_BLOCK_CELL_TEMPERATURE_s testCelltemperatureRedundancy0 = {
    .header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_REDUNDANCY0};

static inline void injectDatabaseEntries(void) {
    DATA_Read_4_DataBlocks_ExpectAndReturn(
        &testCellvoltageBase,
        &testCellvoltageRedundancy0,
        &testCelltemperatureBase,
        &testCelltemperatureRedundancy0,
        STD_OK);
    DATA_Read_4_DataBlocks_IgnoreArg_pDataToReceiver0();
    DATA_Read_4_DataBlocks_IgnoreArg_pDataToReceiver1();
    DATA_Read_4_DataBlocks_IgnoreArg_pDataToReceiver2();
    DATA_Read_4_DataBlocks_IgnoreArg_pDataToReceiver3();
    DATA_Read_4_DataBlocks_ReturnThruPtr_pDataToReceiver0(&testCellvoltageBase);
    DATA_Read_4_DataBlocks_ReturnThruPtr_pDataToReceiver1(&testCellvoltageRedundancy0);
    DATA_Read_4_DataBlocks_ReturnThruPtr_pDataToReceiver2(&testCelltemperatureBase);
    DATA_Read_4_DataBlocks_ReturnThruPtr_pDataToReceiver3(&testCelltemperatureRedundancy0);
}

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    testCellvoltageBase.header.timestamp            = 0;
    testCellvoltageRedundancy0.header.timestamp     = 0;
    testCelltemperatureBase.header.timestamp        = 0;
    testCelltemperatureRedundancy0.header.timestamp = 0;
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
    DATA_EntryUpdatedWithinInterval_IgnoreAndReturn(false);
    DATA_DatabaseEntryUpdatedAtLeastOnce_IgnoreAndReturn(false);

    /* tick zero, database starts also with zero --> nothing to do */
    OS_GetTickCount_IgnoreAndReturn(0);

    DIAG_Handler_IgnoreAndReturn(STD_OK);

    TEST_ASSERT_PASS_ASSERT(TEST_ASSERT_EQUAL(STD_NOT_OK, MRC_ValidateAfeMeasurement()));
}
