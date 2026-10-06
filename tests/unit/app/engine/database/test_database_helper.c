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
 * @file    test_database_helper.c
 * @author  foxBMS Team
 * @date    2021-05-05 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the database helper functions
 * @details TODO
 *
 */

/*========== Includes =======================================================*/

#include "unity.h"
#include "Mockfassert.h"
#include "Mockmpu_prototypes.h"
#include "Mockos.h"

#include "database_cfg.h"

#include "database_helper.h"
#include "test_assert_helper.h"

#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/

/*========== Definitions and Implementations for Unit Test ==================*/
/** timestamp array used as test input */
static uint32_t test_timestamp[BS_NR_OF_STRINGS] = {0};
/** previous timestamp array used as test input */
static uint32_t test_previousTimestamp[BS_NR_OF_STRINGS] = {0};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_timestamp[s]         = 0u;
        test_previousTimestamp[s] = 0u;
    }
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/**
 * @brief   Test extern function #DATA_DatabaseBlockUpdatedAtLeastOnce
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/3: NULL_PTR for kpkTimestamp -> assert
 *            - AT2/3: NULL_PTR for kpkPreviousTimestamp -> assert
 *            - AT3/3: invalid string number -> assert
 *          - Routine validation:
 *            - RT1/4: timestamp != 0, previousTimestamp == 0 -> true
 *            - RT2/4: timestamp != 0, previousTimestamp != 0 -> true
 *            - RT3/4: timestamp == 0, previousTimestamp != 0 -> true
 *            - RT4/4: timestamp == 0, previousTimestamp == 0 -> false
 */
void testDATA_DatabaseBlockUpdatedAtLeastOnce(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(DATA_DatabaseBlockUpdatedAtLeastOnce(NULL_PTR, test_previousTimestamp, 0u));
    /* ======= AT2/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(DATA_DatabaseBlockUpdatedAtLeastOnce(test_timestamp, NULL_PTR, 0u));
    /* ======= AT3/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(
        DATA_DatabaseBlockUpdatedAtLeastOnce(test_timestamp, test_previousTimestamp, BS_NR_OF_STRINGS));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/4: Test implementation */
    /* ======= RT1/4: call function under test */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_timestamp[s]         = 10u;
        test_previousTimestamp[s] = 0u;
        const bool updated0       = DATA_DatabaseBlockUpdatedAtLeastOnce(test_timestamp, test_previousTimestamp, s);
        TEST_ASSERT_TRUE(updated0);
    }
    /* ======= RT1/4: test output verification */
    /* verified in loop */

    /* ======= RT2/4: Test implementation */
    /* ======= RT2/4: call function under test */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_timestamp[s]         = 60u;
        test_previousTimestamp[s] = 10u;
        const bool updated1       = DATA_DatabaseBlockUpdatedAtLeastOnce(test_timestamp, test_previousTimestamp, s);
        TEST_ASSERT_TRUE(updated1);
    }
    /* ======= RT2/4: test output verification */
    /* verified in loop */

    /* ======= RT3/4: Test implementation */
    /* ======= RT3/4: call function under test */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_timestamp[s]         = 0u;
        test_previousTimestamp[s] = 10u;
        const bool updated2       = DATA_DatabaseBlockUpdatedAtLeastOnce(test_timestamp, test_previousTimestamp, s);
        TEST_ASSERT_TRUE(updated2);
    }
    /* ======= RT3/4: test output verification */
    /* verified in loop */

    /* ======= RT4/4: Test implementation */
    /* ======= RT4/4: call function under test */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        test_timestamp[s]         = 0u;
        test_previousTimestamp[s] = 0u;
        const bool updated3       = DATA_DatabaseBlockUpdatedAtLeastOnce(test_timestamp, test_previousTimestamp, s);
        TEST_ASSERT_FALSE(updated3);
    }
    /* ======= RT4/4: test output verification */
    /* verified in loop */
}

/**
 * @brief   Test extern function #DATA_DatabaseBlockUpdatedWithinInterval
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/3: NULL_PTR for kpkTimestamp -> assert
 *            - AT2/3: NULL_PTR for kpkPreviousTimestamp -> assert
 *            - AT3/3: invalid string number -> assert
 *          - Routine validation:
 *            - RT1/5: within interval and updated at least once -> true
 *            - RT2/5: exactly at interval limit -> true
 *            - RT3/5: within interval but never updated -> false
 *            - RT4/5: outside of interval -> false
 *            - RT5/5: timer overflow, within interval -> true
 */
void testDATA_DatabaseBlockUpdatedWithinInterval(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(DATA_DatabaseBlockUpdatedWithinInterval(NULL_PTR, test_previousTimestamp, 0u, 100u));
    /* ======= AT2/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(DATA_DatabaseBlockUpdatedWithinInterval(test_timestamp, NULL_PTR, 0u, 100u));
    /* ======= AT3/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(
        DATA_DatabaseBlockUpdatedWithinInterval(test_timestamp, test_previousTimestamp, BS_NR_OF_STRINGS, 100u));

    /* ======= Routine tests =============================================== */
    const uint32_t timeInterval = 100u;
    const uint8_t s             = 0u;

    /* ======= RT1/5: Test implementation: 50ms difference, updated -> true */
    test_timestamp[s]         = 50u;
    test_previousTimestamp[s] = 10u;
    OS_GetTickCount_ExpectAndReturn(100u);
    /* ======= RT1/5: call function under test */
    TEST_ASSERT_TRUE(DATA_DatabaseBlockUpdatedWithinInterval(test_timestamp, test_previousTimestamp, s, timeInterval));

    /* ======= RT2/5: Test implementation: exactly 100ms difference -> true */
    test_timestamp[s]         = 50u;
    test_previousTimestamp[s] = 10u;
    OS_GetTickCount_ExpectAndReturn(150u);
    /* ======= RT2/5: call function under test */
    TEST_ASSERT_TRUE(DATA_DatabaseBlockUpdatedWithinInterval(test_timestamp, test_previousTimestamp, s, timeInterval));

    /* ======= RT3/5: Test implementation: within interval but never updated -> false */
    test_timestamp[s]         = 0u;
    test_previousTimestamp[s] = 0u;
    OS_GetTickCount_ExpectAndReturn(100u);
    /* ======= RT3/5: call function under test */
    TEST_ASSERT_FALSE(DATA_DatabaseBlockUpdatedWithinInterval(test_timestamp, test_previousTimestamp, s, timeInterval));

    /* ======= RT4/5: Test implementation: 101ms difference -> false */
    test_timestamp[s]         = 10u;
    test_previousTimestamp[s] = 5u;
    OS_GetTickCount_ExpectAndReturn(111u);
    /* ======= RT4/5: call function under test */
    TEST_ASSERT_FALSE(DATA_DatabaseBlockUpdatedWithinInterval(test_timestamp, test_previousTimestamp, s, timeInterval));

    /* ======= RT5/5: Test implementation: timer overflow, 50ms difference -> true */
    test_timestamp[s]         = UINT32_MAX;
    test_previousTimestamp[s] = UINT32_MAX - 10u;
    OS_GetTickCount_ExpectAndReturn(49u);
    /* ======= RT5/5: call function under test */
    TEST_ASSERT_TRUE(DATA_DatabaseBlockUpdatedWithinInterval(test_timestamp, test_previousTimestamp, s, timeInterval));
}

/**
 * @brief   Test extern function #DATA_DatabaseEntryUpdatedAtLeastOnce
 * @details The following cases need to be tested (covers all branches of the
 *          condition `if (!(initialTimestampIsZero && previousTimestampIsZero))`):
 *          - RT1/4: timestamp != 0, previousTimestamp == 0
 *                   (initialZero=false, previousZero=true)  -> true
 *          - RT2/4: timestamp != 0, previousTimestamp != 0
 *                   (initialZero=false, previousZero=false) -> true
 *          - RT3/4: timestamp == 0, previousTimestamp != 0
 *                   (initialZero=true,  previousZero=false) -> true
 *          - RT4/4: timestamp == 0, previousTimestamp == 0
 *                   (initialZero=true,  previousZero=true)  -> false
 */
void testDATA_DatabaseEntryUpdatedAtLeastOnce(void) {
    /* ======= Routine tests =============================================== */
    DATA_BLOCK_CELL_VOLTAGE_s databaseEntry = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};

    /* ======= RT1/4: Test implementation */
    /* Database entry has been updated once, after 10ms */
    databaseEntry.header.timestamp         = 10u;
    databaseEntry.header.previousTimestamp = 0u;
    /* ======= RT1/4: call function under test */
    TEST_ASSERT_TRUE(DATA_DatabaseEntryUpdatedAtLeastOnce(databaseEntry.header));

    /* ======= RT2/4: Test implementation */
    /* Database entry has been updated twice, first after 10ms, then after 50ms */
    databaseEntry.header.timestamp         = 60u;
    databaseEntry.header.previousTimestamp = 10u;
    /* ======= RT2/4: call function under test */
    TEST_ASSERT_TRUE(DATA_DatabaseEntryUpdatedAtLeastOnce(databaseEntry.header));

    /* ======= RT3/4: Test implementation */
    /* timestamp has wrapped/reset to 0 while a previous timestamp exists */
    databaseEntry.header.timestamp         = 0u;
    databaseEntry.header.previousTimestamp = 10u;
    /* ======= RT3/4: call function under test */
    TEST_ASSERT_TRUE(DATA_DatabaseEntryUpdatedAtLeastOnce(databaseEntry.header));

    /* ======= RT4/4: Test implementation */
    /* Database entry has never been updated */
    databaseEntry.header.timestamp         = 0u;
    databaseEntry.header.previousTimestamp = 0u;
    /* ======= RT4/4: call function under test */
    TEST_ASSERT_FALSE(DATA_DatabaseEntryUpdatedAtLeastOnce(databaseEntry.header));
}

/** This function tests various inputs for database helper function
 *  #DATA_EntryUpdatedWithinInterval */
void testDATA_EntryUpdatedWithinInterval(void) {
    DATA_BLOCK_CELL_VOLTAGE_s databaseEntry = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};

    /* Always check always if database entry has been updated within the last 100ms */
    uint32_t timeDifference = 100u;

    /* Time difference: 50ms -> true */
    databaseEntry.header.timestamp = 50u;
    OS_GetTickCount_ExpectAndReturn(100u);
    TEST_ASSERT_TRUE(DATA_EntryUpdatedWithinInterval(databaseEntry.header, timeDifference));

    /* Time difference: 100ms -> true, but never updated */
    databaseEntry.header.timestamp = 0u;
    OS_GetTickCount_ExpectAndReturn(100u);
    TEST_ASSERT_FALSE(DATA_EntryUpdatedWithinInterval(databaseEntry.header, timeDifference));

    /* Time difference: 101ms -> false */
    databaseEntry.header.timestamp = 0u;
    OS_GetTickCount_ExpectAndReturn(101u);
    TEST_ASSERT_FALSE(DATA_EntryUpdatedWithinInterval(databaseEntry.header, timeDifference));

    /* Time difference: 63ms -> true */
    databaseEntry.header.timestamp = 4937u;
    OS_GetTickCount_ExpectAndReturn(5000u);
    TEST_ASSERT_TRUE(DATA_EntryUpdatedWithinInterval(databaseEntry.header, timeDifference));

    /* Time difference: 50ms -> true */
    databaseEntry.header.timestamp = UINT32_MAX;
    OS_GetTickCount_ExpectAndReturn(50u);
    TEST_ASSERT_TRUE(DATA_EntryUpdatedWithinInterval(databaseEntry.header, timeDifference));

    /* Time difference: 100ms -> true */
    databaseEntry.header.timestamp = UINT32_MAX - 50u;
    OS_GetTickCount_ExpectAndReturn(49u);
    TEST_ASSERT_TRUE(DATA_EntryUpdatedWithinInterval(databaseEntry.header, timeDifference));

    /* Time difference: 101ms -> false */
    databaseEntry.header.timestamp = UINT32_MAX - 50u;
    OS_GetTickCount_ExpectAndReturn(50u);
    TEST_ASSERT_FALSE(DATA_EntryUpdatedWithinInterval(databaseEntry.header, timeDifference));

    /* Time difference: UINT32_MAX - 50 -> false */
    databaseEntry.header.timestamp = 50u;
    OS_GetTickCount_ExpectAndReturn(UINT32_MAX);
    TEST_ASSERT_FALSE(DATA_EntryUpdatedWithinInterval(databaseEntry.header, timeDifference));
}

/** This function tests various inputs for database helper function
 *  #DATA_EntryUpdatedPeriodicallyWithinInterval */
void testDATA_EntryUpdatedPeriodicallyWithinInterval(void) {
    DATA_BLOCK_CELL_VOLTAGE_s databaseEntry = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};

    /* Always check always if database entry has been periodically updated within the last 100ms */
    uint32_t timeDifference = 100u;

    /* Time difference timestamp - systick: 40ms -> true
     * Time difference timestamp - previous timestamp:  50ms -> true */
    databaseEntry.header.timestamp         = 60u;
    databaseEntry.header.previousTimestamp = 10u;
    OS_GetTickCount_ExpectAndReturn(100u);
    TEST_ASSERT_TRUE(DATA_EntryUpdatedPeriodicallyWithinInterval(databaseEntry.header, timeDifference));

    /* Within time interval but never updated -> false */
    databaseEntry.header.timestamp         = 0u;
    databaseEntry.header.previousTimestamp = 0u;
    OS_GetTickCount_ExpectAndReturn(50u);
    TEST_ASSERT_FALSE(DATA_EntryUpdatedPeriodicallyWithinInterval(databaseEntry.header, timeDifference));

    /* Time difference timestamp - systick: 100ms -> true
     * Time difference timestamp - previous timestamp:  40ms -> true */
    databaseEntry.header.timestamp         = 80u;
    databaseEntry.header.previousTimestamp = 40u;
    OS_GetTickCount_ExpectAndReturn(180u);
    TEST_ASSERT_TRUE(DATA_EntryUpdatedPeriodicallyWithinInterval(databaseEntry.header, timeDifference));

    /* Time difference timestamp - systick: 101ms -> false
     * Time difference timestamp - previous timestamp:  40ms -> true */
    databaseEntry.header.timestamp         = 80u;
    databaseEntry.header.previousTimestamp = 40u;
    OS_GetTickCount_ExpectAndReturn(181);
    TEST_ASSERT_FALSE(DATA_EntryUpdatedPeriodicallyWithinInterval(databaseEntry.header, timeDifference));

    /* Time difference timestamp - systick: 50ms -> true
     * Time difference timestamp - previous timestamp:  110ms -> false */
    databaseEntry.header.timestamp         = 150u;
    databaseEntry.header.previousTimestamp = 40u;
    OS_GetTickCount_ExpectAndReturn(200u);
    TEST_ASSERT_FALSE(DATA_EntryUpdatedPeriodicallyWithinInterval(databaseEntry.header, timeDifference));

    /* Time difference timestamp - systick: 100ms -> true
     * Time difference timestamp - previous timestamp:  100ms -> true */
    databaseEntry.header.timestamp         = 150u;
    databaseEntry.header.previousTimestamp = 50u;
    OS_GetTickCount_ExpectAndReturn(250u);
    TEST_ASSERT_TRUE(DATA_EntryUpdatedPeriodicallyWithinInterval(databaseEntry.header, timeDifference));

    /* Time difference timestamp - systick: 100ms -> true
     * Time difference timestamp - previous timestamp:  100ms -> true */
    databaseEntry.header.timestamp         = UINT32_MAX - 50u;
    databaseEntry.header.previousTimestamp = UINT32_MAX - 150u;
    OS_GetTickCount_ExpectAndReturn(49u);
    TEST_ASSERT_TRUE(DATA_EntryUpdatedPeriodicallyWithinInterval(databaseEntry.header, timeDifference));

    /* Time difference timestamp - systick: 101ms -> false
     * Time difference timestamp - previous timestamp:  100ms -> true */
    databaseEntry.header.timestamp         = UINT32_MAX - 50u;
    databaseEntry.header.previousTimestamp = UINT32_MAX - 150u;
    OS_GetTickCount_ExpectAndReturn(50u);
    TEST_ASSERT_FALSE(DATA_EntryUpdatedPeriodicallyWithinInterval(databaseEntry.header, timeDifference));

    /* Time difference timestamp - systick: 100ms -> true
     * Time difference timestamp - previous timestamp:  100ms -> true */
    databaseEntry.header.timestamp         = 49u;
    databaseEntry.header.previousTimestamp = UINT32_MAX - 50u;
    OS_GetTickCount_ExpectAndReturn(150u);
    TEST_ASSERT_FALSE(DATA_EntryUpdatedPeriodicallyWithinInterval(databaseEntry.header, timeDifference));

    /* Time difference timestamp - systick: 100ms -> true
     * Time difference timestamp - previous timestamp:  101ms -> false */
    databaseEntry.header.timestamp         = UINT32_MAX - 50u;
    databaseEntry.header.previousTimestamp = UINT32_MAX - 151u;
    OS_GetTickCount_ExpectAndReturn(49u);
    TEST_ASSERT_FALSE(DATA_EntryUpdatedPeriodicallyWithinInterval(databaseEntry.header, timeDifference));

    /* Time difference timestamp - systick: 100ms -> true
     * Time difference timestamp - previous timestamp:  101ms -> false */
    databaseEntry.header.timestamp         = 50u;
    databaseEntry.header.previousTimestamp = UINT32_MAX - 50u;
    OS_GetTickCount_ExpectAndReturn(150u);
    TEST_ASSERT_FALSE(DATA_EntryUpdatedPeriodicallyWithinInterval(databaseEntry.header, timeDifference));

    /* Time difference: UINT32_MAX - 50 -> false */
    databaseEntry.header.timestamp = 50u;
    OS_GetTickCount_ExpectAndReturn(UINT32_MAX);
    TEST_ASSERT_FALSE(DATA_EntryUpdatedPeriodicallyWithinInterval(databaseEntry.header, timeDifference));
}

/** This function tests various inputs for database helper function
 *  #DATA_GetStringNumberFromVoltageIndex */
void testDATA_GetStringNumberFromVoltageIndex(void) {
    /* Last cell in string 0 */
    uint16_t voltageIndex = (BS_NR_OF_CELL_BLOCKS_PER_MODULE * BS_NR_OF_MODULES_PER_STRING) - 1u;
    TEST_ASSERT_EQUAL(DATA_GetStringNumberFromVoltageIndex(voltageIndex), 0u);

    /* Last cell in last string */
    voltageIndex = (BS_NR_OF_CELL_BLOCKS_PER_MODULE * BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_STRINGS) - 1u;
    TEST_ASSERT_EQUAL(DATA_GetStringNumberFromVoltageIndex(voltageIndex), BS_NR_OF_STRINGS - 1u);

    /* Test is function passes if index 0 is passed */
    TEST_ASSERT_PASS_ASSERT(DATA_GetStringNumberFromVoltageIndex(0u));

    /* Test is function asserts if invalid index is passed */
    TEST_ASSERT_FAIL_ASSERT(DATA_GetStringNumberFromVoltageIndex(
        BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_CELL_BLOCKS_PER_MODULE * BS_NR_OF_STRINGS));
    TEST_ASSERT_FAIL_ASSERT(DATA_GetStringNumberFromVoltageIndex(UINT16_MAX));
}

/** This function tests various inputs for database helper function
 *  #DATA_GetModuleNumberFromVoltageIndex */
void testDATA_GetModuleNumberFromVoltageIndex(void) {
    uint16_t voltageIndex = (BS_NR_OF_CELL_BLOCKS_PER_MODULE * BS_NR_OF_MODULES_PER_STRING) - 1u;
    TEST_ASSERT_EQUAL(DATA_GetModuleNumberFromVoltageIndex(voltageIndex), BS_NR_OF_MODULES_PER_STRING - 1u);

    /* Test is function passes if index 0 is passed */
    TEST_ASSERT_PASS_ASSERT(DATA_GetModuleNumberFromVoltageIndex(0u));

    /* Test is function asserts if invalid index is passed */
    TEST_ASSERT_FAIL_ASSERT(DATA_GetModuleNumberFromVoltageIndex(
        BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_CELL_BLOCKS_PER_MODULE * BS_NR_OF_STRINGS));
    TEST_ASSERT_FAIL_ASSERT(DATA_GetModuleNumberFromVoltageIndex(UINT16_MAX));
}

/** This function tests various inputs for database helper function
 *  #testDATA_GetCellNumberFromVoltageIndex */
void testDATA_GetCellNumberFromVoltageIndex(void) {
    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
            uint16_t voltageIndex = (m * BS_NR_OF_CELL_BLOCKS_PER_MODULE) + cb;
            TEST_ASSERT_EQUAL(DATA_GetCellNumberFromVoltageIndex(voltageIndex), cb);
        }
    }

    /* Test is function passes if index 0 is passed */
    TEST_ASSERT_PASS_ASSERT(DATA_GetCellNumberFromVoltageIndex(0u));

    /* Test is function asserts if invalid index is passed */
    TEST_ASSERT_FAIL_ASSERT(DATA_GetCellNumberFromVoltageIndex(
        BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_CELL_BLOCKS_PER_MODULE * BS_NR_OF_STRINGS));
    TEST_ASSERT_FAIL_ASSERT(DATA_GetCellNumberFromVoltageIndex(UINT16_MAX));
}

/** This function tests various inputs for database helper function
 *  #DATA_GetStringNumberFromTemperatureIndex */
void testDATA_GetStringNumberFromTemperatureIndex(void) {
    /* last sensor in string 0 */
    uint16_t sensorIndex = (BS_NR_OF_TEMP_SENSORS_PER_MODULE * BS_NR_OF_MODULES_PER_STRING) - 1u;
    TEST_ASSERT_EQUAL(DATA_GetStringNumberFromTemperatureIndex(sensorIndex), 0u);

    /* last sensor in last string */
    sensorIndex = (BS_NR_OF_TEMP_SENSORS_PER_MODULE * BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_STRINGS) - 1u;
    TEST_ASSERT_EQUAL(DATA_GetStringNumberFromTemperatureIndex(sensorIndex), BS_NR_OF_STRINGS - 1u);

    /* Test is function passes if index 0 is passed */
    TEST_ASSERT_PASS_ASSERT(DATA_GetStringNumberFromTemperatureIndex(0u));

    /* Test is function asserts if invalid index is passed */
    TEST_ASSERT_FAIL_ASSERT(DATA_GetStringNumberFromTemperatureIndex(
        BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_TEMP_SENSORS_PER_MODULE * BS_NR_OF_STRINGS));
    TEST_ASSERT_FAIL_ASSERT(DATA_GetStringNumberFromTemperatureIndex(UINT16_MAX));
}

/** This function tests various inputs for database helper function
 *  #DATA_GetModuleNumberFromTemperatureIndex */
void testDATA_GetModuleNumberFromTemperatureIndex(void) {
    uint16_t sensorIndex = (BS_NR_OF_TEMP_SENSORS_PER_MODULE * BS_NR_OF_MODULES_PER_STRING) - 1u;
    TEST_ASSERT_EQUAL(DATA_GetModuleNumberFromTemperatureIndex(sensorIndex), BS_NR_OF_MODULES_PER_STRING - 1u);

    /* Test is function passes if index 0 is passed */
    TEST_ASSERT_PASS_ASSERT(DATA_GetModuleNumberFromTemperatureIndex(0u));

    /* Test is function asserts if invalid index is passed */
    TEST_ASSERT_FAIL_ASSERT(DATA_GetModuleNumberFromTemperatureIndex(
        BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_TEMP_SENSORS_PER_MODULE * BS_NR_OF_STRINGS));
    TEST_ASSERT_FAIL_ASSERT(DATA_GetModuleNumberFromTemperatureIndex(UINT16_MAX));
}

/** This function tests various inputs for database helper function
 *  #DATA_GetSensorNumberFromTemperatureIndex */
void testDATA_GetSensorNumberFromTemperatureIndex(void) {
    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        for (uint8_t sensor = 0u; sensor < BS_NR_OF_TEMP_SENSORS_PER_MODULE; sensor++) {
            uint16_t sensorIndex = (m * BS_NR_OF_TEMP_SENSORS_PER_MODULE) + sensor;
            TEST_ASSERT_EQUAL(DATA_GetSensorNumberFromTemperatureIndex(sensorIndex), sensor);
        }
    }

    /* Test is function passes if index 0 is passed */
    TEST_ASSERT_PASS_ASSERT(DATA_GetSensorNumberFromTemperatureIndex(0u));

    /* Test is function asserts if invalid index is passed */
    TEST_ASSERT_FAIL_ASSERT(DATA_GetSensorNumberFromTemperatureIndex(
        BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_TEMP_SENSORS_PER_MODULE * BS_NR_OF_STRINGS));
    TEST_ASSERT_FAIL_ASSERT(DATA_GetSensorNumberFromTemperatureIndex(UINT16_MAX));
}
