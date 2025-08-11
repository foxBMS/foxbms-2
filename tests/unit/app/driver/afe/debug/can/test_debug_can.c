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
 * @file    test_debug_can.c
 * @author  foxBMS Team
 * @date    2020-09-17 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the debug_can.c module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockcan_helper.h"
#include "Mockdatabase.h"
#include "Mockdatabase_cfg.h"
#include "Mockftask.h"
#include "Mockos.h"

#include "can_cfg_rx-message-definitions.h"
#include "debug_can.h"
#include "fstd_types.h"
#include "test_assert_helper.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("debug_can.c")

TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/afe/debug/can")
TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/engine/config")
TEST_INCLUDE_PATH("../../src/app/engine/database")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/
OS_QUEUE ftsk_canToAfeCellVoltagesQueue;
OS_QUEUE ftsk_canToAfeCellTemperaturesQueue;

static DATA_BLOCK_CELL_VOLTAGE_s decan_cellVoltage         = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_VOLTAGE_s decan_cellVoltageFromRead = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_TEMPERATURE_s decan_cellTemperature = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
static DATA_BLOCK_CELL_TEMPERATURE_s decan_cellTemperatureFromRead = {
    .header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/*========== Externalized Static Function Test Cases ========================*/

/**
 * @brief   Testing static function modified modulo
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: a=0, b=1; assert;
 *            - AT2/2: a=1, b=0; assert;
 *          - Routine validation:
 *            - RT1/3: a=1, b=1; c=1;
 *            - RT2/3: a=1, b=2; c=1;
 *            - RT3/3: a=2, b=2; c=2;
 */
void testModified_modulo(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2: a=0, b=1; assert */
    uint16_t a = 0;
    uint16_t b = 1;
    TEST_ASSERT_FAIL_ASSERT(TEST_DECAN_ModifiedModuloFunction(a, b));

    /* ======= AT2/2: a=1, b=0; assert */
    a = 1;
    b = 0;
    TEST_ASSERT_FAIL_ASSERT(TEST_DECAN_ModifiedModuloFunction(a, b));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/3: a=1, b=1; c=1 */
    a = 1;
    b = 1;
    TEST_ASSERT_EQUAL_INT16(1, TEST_DECAN_ModifiedModuloFunction(a, b));

    /* ======= RT2/3: a=1, b=2; c=1 */
    a = 1;
    b = 2;
    TEST_ASSERT_EQUAL_INT16(1, TEST_DECAN_ModifiedModuloFunction(a, b));

    /* ======= RT3/3: a=2, b=2; c=2 */
    a = 2;
    b = 2;
    TEST_ASSERT_EQUAL_INT16(2, TEST_DECAN_ModifiedModuloFunction(a, b));
}

/**
 * @brief   Testing static function DECAN_ConvertIndexForVoltage
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: oneNumIdxOfVoltage = BS_NR_OF_STRINGS * BS_NR_OF_MODULES_PER_STRING *
 * BS_NR_OF_CELL_BLOCKS_PER_MODULE; assert;
 *          - Routine validation:
 *            - RT1/5: oneNumIdxOfVoltage=0; s=0, m=0, cb=0;
 *            - RT2/5: oneNumIdxOfVoltage=BS_NR_OF_CELL_BLOCKS_PER_MODULE-1;
 * s=0, m=0, cb=BS_NR_OF_CELL_BLOCKS_PER_MODULE-1;
 *            - RT3/5: oneNumIdxOfVoltage=BS_NR_OF_CELL_BLOCKS_PER_MODULE; s=0, m=1, cb=0;
 *            - RT4/5: oneNumIdxOfVoltage=BS_NR_OF_CELL_BLOCKS_PER_MODULE;
 * s=0, m=BS_NR_OF_MODULES_PER_STRING-1, cb=BS_NR_OF_CELL_BLOCKS_PER_MODULE-1;
 *            - RT5/5: oneNumIdxOfVoltage=BS_NR_OF_MODULES_PER_STRING*BS_NR_OF_CELL_BLOCKS_PER_MODULE; s=1, m=0, cb=0;
 */
void testDECAN_ConvertIndexForVoltage(void) {
    uint16_t s                  = 0;
    uint16_t m                  = 0;
    uint16_t cb                 = 0;
    uint16_t oneNumIdxOfVoltage = 0;

    /* Because the following test requires: number of strings > 1;
    number of modules per string > 1; To ensure the test can be run,
    the defines will be checked first */
    TEST_ASSERT_EQUAL_INT16(4, BS_NR_OF_MODULES_PER_STRING);
    TEST_ASSERT_EQUAL_INT16(2, BS_NR_OF_STRINGS);
    TEST_ASSERT_EQUAL_INT16(18, BS_NR_OF_CELL_BLOCKS_PER_MODULE);

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1: Assertion Test of the beyond-upper-boundary case */
    oneNumIdxOfVoltage = BS_NR_OF_STRINGS * BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_CELL_BLOCKS_PER_MODULE;
    TEST_ASSERT_FAIL_ASSERT(TEST_DECAN_ConvertIndexForVoltage(&s, &m, &cb, oneNumIdxOfVoltage));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/5: oneNumIdxOfVoltage=0; s=0, m=0, cb=0 */
    oneNumIdxOfVoltage = 0;
    TEST_DECAN_ConvertIndexForVoltage(&s, &m, &cb, oneNumIdxOfVoltage);
    TEST_ASSERT_EQUAL_INT16(0, s);
    TEST_ASSERT_EQUAL_INT16(0, m);
    TEST_ASSERT_EQUAL_INT16(0, cb);

    /* ======= RT2/5: oneNumIdxOfVoltage=BS_NR_OF_CELL_BLOCKS_PER_MODULE;
    s=0, m=0, cb=BS_NR_OF_CELL_BLOCKS_PER_MODULE - 1 */
    oneNumIdxOfVoltage = BS_NR_OF_CELL_BLOCKS_PER_MODULE - 1;
    TEST_DECAN_ConvertIndexForVoltage(&s, &m, &cb, oneNumIdxOfVoltage);
    TEST_ASSERT_EQUAL_INT16(0, s);
    TEST_ASSERT_EQUAL_INT16(0, m);
    TEST_ASSERT_EQUAL_INT16(BS_NR_OF_CELL_BLOCKS_PER_MODULE - 1, cb);

    /* ======= RT3/5: oneNumIdxOfVoltage=BS_NR_OF_CELL_BLOCKS_PER_MODULE;
    s=0, m=1, cb=0 */
    oneNumIdxOfVoltage = BS_NR_OF_CELL_BLOCKS_PER_MODULE;
    TEST_DECAN_ConvertIndexForVoltage(&s, &m, &cb, oneNumIdxOfVoltage);
    TEST_ASSERT_EQUAL_INT16(0, s);
    TEST_ASSERT_EQUAL_INT16(1, m);
    TEST_ASSERT_EQUAL_INT16(0, cb);

    /* ======= RT4/5: oneNumIdxOfVoltage=BS_NR_OF_CELL_BLOCKS_PER_MODULE;
    s=0, m=BS_NR_OF_MODULES_PER_STRING-1, cb=BS_NR_OF_CELL_BLOCKS_PER_MODULE-1 */
    oneNumIdxOfVoltage = BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_CELL_BLOCKS_PER_MODULE - 1;
    TEST_DECAN_ConvertIndexForVoltage(&s, &m, &cb, oneNumIdxOfVoltage);
    TEST_ASSERT_EQUAL_INT16(0, s);
    TEST_ASSERT_EQUAL_INT16(BS_NR_OF_MODULES_PER_STRING - 1, m);
    TEST_ASSERT_EQUAL_INT16(BS_NR_OF_CELL_BLOCKS_PER_MODULE - 1, cb);

    /* ======= RT5/5: BS_NR_OF_MODULES_PER_STRING*BS_NR_OF_CELL_BLOCKS_PER_MODULE;
    s=1, m=0, cb=0 */
    oneNumIdxOfVoltage = BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_CELL_BLOCKS_PER_MODULE;
    TEST_DECAN_ConvertIndexForVoltage(&s, &m, &cb, oneNumIdxOfVoltage);
    TEST_ASSERT_EQUAL_INT16(1, s);
    TEST_ASSERT_EQUAL_INT16(0, m);
    TEST_ASSERT_EQUAL_INT16(0, cb);
}

/**
 * @brief   Testing static function DECAN_ConvertIndexForTemperature
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: oneNumIdxOfTemperature = BS_NR_OF_STRINGS * BS_NR_OF_MODULES_PER_STRING *
 * BS_NR_OF_TEMP_SENSORS_PER_MODULE; assert;
 *          - Routine validation:
 *            - RT1/5: oneNumIdxOfTemperature=0; s=0, m=0, ts=0;
 *            - RT2/5: oneNumIdxOfTemperature=BS_NR_OF_TEMP_SENSORS_PER_MODULE;
 * s=0, m=0, ts=BS_NR_OF_TEMP_SENSORS_PER_MODULE-1
 *            - RT3/5: oneNumIdxOfTemperature=BS_NR_OF_TEMP_SENSORS_PER_MODULE; s=0, m=1, ts=0;
 *            - RT4/5: oneNumIdxOfTemperature=BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_TEMP_SENSORS_PER_MODULE;
 * s=0, m=BS_NR_OF_MODULES_PER_STRING-1, ts=BS_NR_OF_TEMP_SENSORS_PER_MODULE-1
 *            - RT5/5: oneNumIdxOfTemperature=BS_NR_OF_MODULES_PER_STRING*BS_NR_OF_TEMP_SENSORS_PER_MODULE; s=1, m=0,
 * ts=0;
 */
void testDECAN_ConvertIndexForTemperature(void) {
    uint16_t s                      = 0;
    uint16_t m                      = 0;
    uint16_t ts                     = 0;
    uint16_t oneNumIdxOfTemperature = 0;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1: Assertion Test of the beyond-upper-boundary case */
    oneNumIdxOfTemperature = BS_NR_OF_STRINGS * BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_TEMP_SENSORS_PER_MODULE;
    TEST_ASSERT_FAIL_ASSERT(TEST_DECAN_ConvertIndexForTemperature(&s, &m, &ts, oneNumIdxOfTemperature));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/5: oneNumIdxOfTemperature=0; s=0, m=0, ts=0 */
    oneNumIdxOfTemperature = 0;
    TEST_DECAN_ConvertIndexForTemperature(&s, &m, &ts, oneNumIdxOfTemperature);
    TEST_ASSERT_EQUAL_INT16(0, s);
    TEST_ASSERT_EQUAL_INT16(0, m);
    TEST_ASSERT_EQUAL_INT16(0, ts);

    /* ======= RT2/5: oneNumIdxOfTemperature=BS_NR_OF_TEMP_SENSORS_PER_MODULE;
    s=0, m=0, ts=BS_NR_OF_TEMP_SENSORS_PER_MODULE-1 */
    oneNumIdxOfTemperature = BS_NR_OF_TEMP_SENSORS_PER_MODULE - 1;
    TEST_DECAN_ConvertIndexForTemperature(&s, &m, &ts, oneNumIdxOfTemperature);
    TEST_ASSERT_EQUAL_INT16(0, s);
    TEST_ASSERT_EQUAL_INT16(0, m);
    TEST_ASSERT_EQUAL_INT16(BS_NR_OF_TEMP_SENSORS_PER_MODULE - 1, ts);

    /* ======= RT3/5: oneNumIdxOfTemperature=BS_NR_OF_TEMP_SENSORS_PER_MODULE;
    s=0, m=1, ts=0 */
    oneNumIdxOfTemperature = BS_NR_OF_TEMP_SENSORS_PER_MODULE;
    TEST_DECAN_ConvertIndexForTemperature(&s, &m, &ts, oneNumIdxOfTemperature);
    TEST_ASSERT_EQUAL_INT16(0, s);
    TEST_ASSERT_EQUAL_INT16(1, m);
    TEST_ASSERT_EQUAL_INT16(0, ts);

    /* ======= RT4/5: oneNumIdxOfTemperature=BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_TEMP_SENSORS_PER_MODULE;
    s=0, m=BS_NR_OF_MODULES_PER_STRING-1, ts=BS_NR_OF_TEMP_SENSORS_PER_MODULE-1 */
    oneNumIdxOfTemperature = BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_TEMP_SENSORS_PER_MODULE - 1;
    TEST_DECAN_ConvertIndexForTemperature(&s, &m, &ts, oneNumIdxOfTemperature);
    TEST_ASSERT_EQUAL_INT16(0, s);
    TEST_ASSERT_EQUAL_INT16(BS_NR_OF_MODULES_PER_STRING - 1, m);
    TEST_ASSERT_EQUAL_INT16(BS_NR_OF_TEMP_SENSORS_PER_MODULE - 1, ts);

    /* ======= RT5/5: BS_NR_OF_MODULES_PER_STRING*BS_NR_OF_TEMP_SENSORS_PER_MODULE;
    s=1, m=0, ts=0 */
    oneNumIdxOfTemperature = BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_TEMP_SENSORS_PER_MODULE;
    TEST_DECAN_ConvertIndexForTemperature(&s, &m, &ts, oneNumIdxOfTemperature);
    TEST_ASSERT_EQUAL_INT16(1, s);
    TEST_ASSERT_EQUAL_INT16(0, m);
    TEST_ASSERT_EQUAL_INT16(0, ts);
}

/**
 * @brief   Testing static function DECAN_ReceiveCanCellVoltages
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: if function can be successfully run or not;
 */
void testDECAN_ReceiveCanCellVoltages(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1: test if the function can be successfully run or not */
    uint64_t messageData = 0u;
    OS_ReceiveFromQueue_ExpectAndReturn(
        ftsk_canToAfeCellVoltagesQueue, &messageData, DECAN_CAN2AFE_QUEUE_TIMEOUT_MS, OS_SUCCESS);

    DATA_Write1DataBlock_ExpectAndReturn(&decan_cellVoltage, STD_OK);
    DATA_Read1DataBlock_ExpectAndReturn(&decan_cellVoltageFromRead, STD_OK);
    DATA_Write1DataBlock_ExpectAndReturn(&decan_cellVoltageFromRead, STD_OK);
    TEST_ASSERT_EQUAL(STD_OK, TEST_DECAN_ReceiveCanCellVoltages());
}

/**
 * @brief   Testing static function DECAN_ReceiveCanCellVoltages
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: if function can be successfully run or not;
 */
void testDECAN_ReceiveCanCellTemperatures(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1: test if the function can be successfully run or not */
    uint64_t messageData = 0u;
    OS_ReceiveFromQueue_ExpectAndReturn(
        ftsk_canToAfeCellTemperaturesQueue, &messageData, DECAN_CAN2AFE_QUEUE_TIMEOUT_MS, OS_SUCCESS);

    DATA_Write1DataBlock_ExpectAndReturn(&decan_cellTemperature, STD_OK);
    DATA_Read1DataBlock_ExpectAndReturn(&decan_cellTemperatureFromRead, STD_OK);
    DATA_Write1DataBlock_ExpectAndReturn(&decan_cellTemperatureFromRead, STD_OK);
    TEST_ASSERT_EQUAL(STD_OK, TEST_DECAN_ReceiveCanCellTemperatures());
}
