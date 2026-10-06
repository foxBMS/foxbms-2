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
 * @file    test_debug_can.c
 * @author  foxBMS Team
 * @date    2020-09-17 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
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

/*========== Definitions and Implementations for Unit Test ==================*/
OS_QUEUE ftsk_canToAfeCellVoltagesQueue;
OS_QUEUE ftsk_canToAfeCellTemperaturesQueue;

/* The production functions use local queue and database objects.
 * These test-local values keep expected queue order and interaction counts
 * without depending on the addresses of those local objects. */
static QueueHandle_t decan_expectedQueues[2];
static uint8_t decan_expectedQueueCount;
static uint8_t decan_receivedQueueCount;
static uint8_t decan_readDataBlockCount;
static uint8_t decan_writeDataBlockCount;

/* Stubs accept production-local destinations while validating stable call
 * arguments that are relevant to the test. */
static OS_STD_RETURN_e testReceiveFromQueue(
    QueueHandle_t xQueue,
    void *const pvBuffer,
    uint32_t ticksToWait,
    int cmock_num_calls) {
    (void)cmock_num_calls;
    /* One receive operation is configured per test, so this also rejects an
     * unexpected extra queue call before indexing the expected queue array. */
    TEST_ASSERT_TRUE(decan_receivedQueueCount < decan_expectedQueueCount);
    TEST_ASSERT_EQUAL(decan_expectedQueues[decan_receivedQueueCount], xQueue);
    TEST_ASSERT_NOT_NULL(pvBuffer);
    TEST_ASSERT_EQUAL(DECAN_CAN2AFE_QUEUE_TIMEOUT_MS, ticksToWait);
    decan_receivedQueueCount++;
    return OS_SUCCESS;
}

static STD_RETURN_TYPE_e testReadDataBlock(void *pDataToReceiver0, int cmock_num_calls) {
    (void)cmock_num_calls;
    TEST_ASSERT_NOT_NULL(pDataToReceiver0);
    decan_readDataBlockCount++;
    return STD_OK;
}

static STD_RETURN_TYPE_e testWriteDataBlock(void *pDataFromSender0, int cmock_num_calls) {
    (void)cmock_num_calls;
    TEST_ASSERT_NOT_NULL(pDataFromSender0);
    decan_writeDataBlockCount++;
    return STD_OK;
}

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    /* Each test installs its own stubs and starts with clean interaction counts. */
    decan_expectedQueueCount  = 0u;
    decan_receivedQueueCount  = 0u;
    decan_readDataBlockCount  = 0u;
    decan_writeDataBlockCount = 0u;
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
    /* These constants document the topology selected by test.json: four
     * modules per string, two strings, and 18 cell blocks per module. */
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
    /* Database indexes are zero-based, so the first flat index maps to 0/0/0. */
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
    /* The first flat temperature index likewise maps to the first 0/0/0 entry. */
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
    decan_expectedQueues[0]  = ftsk_canToAfeCellVoltagesQueue;
    decan_expectedQueueCount = 1u;
    OS_ReceiveFromQueue_StubWithCallback(testReceiveFromQueue);

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    /* The callbacks accept production-local database blocks and count the
     * reads and writes performed by the receive operation. */
    DATA_Write1DataBlock_StubWithCallback(testWriteDataBlock);
    DATA_Read1DataBlock_StubWithCallback(testReadDataBlock);
    TEST_ASSERT_EQUAL(STD_OK, TEST_DECAN_ReceiveCanCellVoltages());
    TEST_ASSERT_EQUAL(0u, decan_readDataBlockCount);
    TEST_ASSERT_EQUAL(1u, decan_writeDataBlockCount);
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
    decan_expectedQueues[0]  = ftsk_canToAfeCellTemperaturesQueue;
    decan_expectedQueueCount = 1u;
    OS_ReceiveFromQueue_StubWithCallback(testReceiveFromQueue);

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    /* The production-local database blocks are not addressable from this test,
     * so callbacks keep the interaction counts explicit instead. */
    DATA_Write1DataBlock_StubWithCallback(testWriteDataBlock);
    DATA_Read1DataBlock_StubWithCallback(testReadDataBlock);
    TEST_ASSERT_EQUAL(STD_OK, TEST_DECAN_ReceiveCanCellTemperatures());
    TEST_ASSERT_EQUAL(0u, decan_readDataBlockCount);
    TEST_ASSERT_EQUAL(1u, decan_writeDataBlockCount);
}

/**
 * @brief   Testing extern function DECAN_Initialize
 * @details The following cases need to be tested:
 *          - Routine tests:
 *            - RT1/1: if function can be successfully run or not;
 */
void testDECAN_Initialize(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    OS_GetTickCount_ExpectAndReturn(0u);
    uint32_t currentTime = 0u;
    OS_DelayTaskUntil_Expect(&currentTime, 10u);

    STD_RETURN_TYPE_e returnValue = STD_NOT_OK;
    returnValue                   = DECAN_Initialize();
    TEST_ASSERT_EQUAL(STD_OK, returnValue);
}

/**
 * @brief   Testing extern function DECAN_TriggerAfe
 * @details The following cases need to be tested:
 *          - Routine tests:
 *            - RT1/1: if function can be successfully run or not;
 */
void testDECAN_TriggerAfe(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    decan_expectedQueues[0]  = ftsk_canToAfeCellVoltagesQueue;
    decan_expectedQueues[1]  = ftsk_canToAfeCellTemperaturesQueue;
    decan_expectedQueueCount = 2u;
    /* One queue stub validates both queue calls in order; database stubs
     * accept each production-local block and record the expected interactions.
     */
    OS_ReceiveFromQueue_StubWithCallback(testReceiveFromQueue);
    DATA_Write1DataBlock_StubWithCallback(testWriteDataBlock);
    DATA_Read1DataBlock_StubWithCallback(testReadDataBlock);

    OS_GetTickCount_ExpectAndReturn(0u);
    uint32_t currentTime = 0u;
    OS_DelayTaskUntil_Expect(&currentTime, 10u);

    STD_RETURN_TYPE_e returnValue = STD_NOT_OK;
    returnValue                   = DECAN_TriggerAfe();
    TEST_ASSERT_EQUAL(STD_OK, returnValue);
    /* TriggerAfe writes each received database block once. */
    TEST_ASSERT_EQUAL(0u, decan_readDataBlockCount);
    TEST_ASSERT_EQUAL(2u, decan_writeDataBlockCount);
}
