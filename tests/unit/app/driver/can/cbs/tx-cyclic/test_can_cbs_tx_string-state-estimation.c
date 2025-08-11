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
 * @file    test_can_cbs_tx_string-state-estimation.c
 * @author  foxBMS Team
 * @date    2021-07-27 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the CAN driver callbacks
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockbms.h"
#include "Mockcan.h"
#include "Mockcan_helper.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockfoxmath.h"
#include "Mockimd.h"
#include "Mockos.h"

#include "database_cfg.h"

#include "can_cbs_tx_cyclic.h"
#include "can_cfg_tx-cyclic-message-definitions.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cbs_tx_string-state-estimation.c")

TEST_INCLUDE_PATH("../../src/app/application/bms")
TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/tx-cyclic")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/contactor")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/fram")
TEST_INCLUDE_PATH("../../src/app/driver/imd")
TEST_INCLUDE_PATH("../../src/app/driver/sps")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/engine/sys_mon")
TEST_INCLUDE_PATH("../../src/app/task/config")

/*========== Definitions and Implementations for Unit Test ==================*/
uint64_t testMessageData[8u] = {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u};

float_t testSignalData[5u] = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f};

float_t testSoh = 100.0f;

float_t testMinimumSoc0       = 15;
float_t testMaximumSoc0       = 95;
float_t testMinimumSoe0       = 10;
float_t testMaximumSoe0       = 90;
float_t testMinimumSoeEnergy0 = 12300.0f;

float_t testMinimumSoc1       = 10;
float_t testMaximumSoc1       = 90;
float_t testMinimumSoe1       = 15;
float_t testMaximumSoe1       = 95;
float_t testMinimumSoeEnergy1 = 45600.0f;

static const CAN_SIGNAL_TYPE_s cantx_testSignalMinimumSoc       = {3u, 9u, 0.2f, 0.0f, 0.0f, 100.0f};
static const CAN_SIGNAL_TYPE_s cantx_testSignalMaximumSoc       = {10u, 9u, 0.2f, 0.0f, 0.0f, 100.0f};
static const CAN_SIGNAL_TYPE_s cantx_testSignalMinimumSoe       = {17u, 9u, 0.2f, 0.0f, 0.0f, 100.0f};
static const CAN_SIGNAL_TYPE_s cantx_testSignalMaximumSoe       = {24u, 9u, 0.2f, 0.0f, 0.0f, 100.0f};
static const CAN_SIGNAL_TYPE_s cantx_testSignalSoh              = {47u, 9u, 0.2f, 0.0f, 0.0f, 100.0f};
static const CAN_SIGNAL_TYPE_s cantx_testSignalMinimumSoeEnergy = {54u, 15u, 10.0f, 0.0f, 0.0f, 327670.0f};

static DATA_BLOCK_CELL_VOLTAGE_s can_tableCellVoltages        = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
static DATA_BLOCK_CELL_TEMPERATURE_s can_tableTemperatures    = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
static DATA_BLOCK_MIN_MAX_s can_tableMinimumMaximumValues     = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
static DATA_BLOCK_CURRENT_SENSOR_s can_tableCurrentSensor     = {.header.uniqueId = DATA_BLOCK_ID_CURRENT_SENSOR};
static DATA_BLOCK_OPEN_WIRE_s can_tableOpenWire               = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};
static DATA_BLOCK_STATE_REQUEST_s can_tableStateRequest       = {.header.uniqueId = DATA_BLOCK_ID_STATE_REQUEST};
static DATA_BLOCK_PACK_VALUES_s can_tablePackValues           = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
static DATA_BLOCK_SOF_s can_tableSof                          = {.header.uniqueId = DATA_BLOCK_ID_SOF};
static DATA_BLOCK_SOC_s can_tableSoc                          = {.header.uniqueId = DATA_BLOCK_ID_SOC};
static DATA_BLOCK_SOE_s can_tableSoe                          = {.header.uniqueId = DATA_BLOCK_ID_SOE};
static DATA_BLOCK_ERROR_STATE_s can_tableErrorState           = {.header.uniqueId = DATA_BLOCK_ID_ERROR_STATE};
static DATA_BLOCK_INSULATION_MONITORING_s can_tableInsulation = {
    .header.uniqueId = DATA_BLOCK_ID_INSULATION_MONITORING};
static DATA_BLOCK_MSL_FLAG_s can_tableMslFlags            = {.header.uniqueId = DATA_BLOCK_ID_MSL_FLAG};
static DATA_BLOCK_RSL_FLAG_s can_tableRslFlags            = {.header.uniqueId = DATA_BLOCK_ID_RSL_FLAG};
static DATA_BLOCK_MOL_FLAG_s can_tableMolFlags            = {.header.uniqueId = DATA_BLOCK_ID_MOL_FLAG};
static DATA_BLOCK_AEROSOL_SENSOR_s can_tableAerosolSensor = {.header.uniqueId = DATA_BLOCK_ID_AEROSOL_SENSOR};

OS_QUEUE imd_canDataQueue = NULL_PTR;

const CAN_SHIM_s can_kShim = {
    .pQueueImd             = &imd_canDataQueue,
    .pTableCellVoltage     = &can_tableCellVoltages,
    .pTableCellTemperature = &can_tableTemperatures,
    .pTableMinMax          = &can_tableMinimumMaximumValues,
    .pTableCurrentSensor   = &can_tableCurrentSensor,
    .pTableOpenWire        = &can_tableOpenWire,
    .pTableStateRequest    = &can_tableStateRequest,
    .pTablePackValues      = &can_tablePackValues,
    .pTableSof             = &can_tableSof,
    .pTableSoc             = &can_tableSoc,
    .pTableSoe             = &can_tableSoe,
    .pTableErrorState      = &can_tableErrorState,
    .pTableInsulation      = &can_tableInsulation,
    .pTableMsl             = &can_tableMslFlags,
    .pTableRsl             = &can_tableRslFlags,
    .pTableMol             = &can_tableMolFlags,
    .pTableAerosolSensor   = &can_tableAerosolSensor,
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    can_tableSoc.minimumSoc_perc[0u] = testMinimumSoc0;
    can_tableSoc.maximumSoc_perc[0u] = testMaximumSoc0;
    can_tableSoe.minimumSoe_perc[0u] = testMinimumSoe0;
    can_tableSoe.maximumSoe_perc[0u] = testMaximumSoe0;
    can_tableSoe.minimumSoe_Wh[0u]   = (uint32_t)testMinimumSoeEnergy0;

    can_tableSoc.minimumSoc_perc[1u] = testMinimumSoc1;
    can_tableSoc.maximumSoc_perc[1u] = testMaximumSoc1;
    can_tableSoe.minimumSoe_perc[1u] = testMinimumSoe1;
    can_tableSoe.maximumSoe_perc[1u] = testMaximumSoe1;
    can_tableSoe.minimumSoe_Wh[1u]   = (uint32_t)testMinimumSoeEnergy1;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/**
 * @brief   Testing CANTX_CalculateMinimumStringSoc
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: BS_NR_OF_STRINGS for stringNumber -> assert
 *            - AT2/2: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/2: Signal data is prepared as expected for string 0
 *            - RT2/2: Signal data is prepared as expected for string 1
 */
void testCANTX_CalculateMinimumStringSoc(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t testStringNumber = 0u;
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateMinimumStringSoc(BS_NR_OF_STRINGS, &can_kShim));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateMinimumStringSoc(testStringNumber, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    CAN_TxPrepareSignalData_Expect(&testMinimumSoc0, cantx_testSignalMinimumSoc);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT1/2: Call function under test */
    uint64_t testResult = TEST_CANTX_CalculateMinimumStringSoc(testStringNumber, &can_kShim);
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);

    /* ======= RT2/2: Test implementation */
    testStringNumber = 1u;
    CAN_TxPrepareSignalData_Expect(&testMinimumSoc1, cantx_testSignalMinimumSoc);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT2/2: Call function under test */
    testResult = TEST_CANTX_CalculateMinimumStringSoc(testStringNumber, &can_kShim);
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);
}

/**
 * @brief   Testing CANTX_CalculateMaximumStringSoc
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: BS_NR_OF_STRINGS for stringNumber -> assert
 *            - AT2/2: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/2: Signal data is prepared as expected for string 0
 *            - RT2/2: Signal data is prepared as expected for string 1
 */
void testCANTX_CalculateMaximumStringSoc(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t testStringNumber = 0u;
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateMaximumStringSoc(BS_NR_OF_STRINGS, &can_kShim));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateMaximumStringSoc(testStringNumber, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    CAN_TxPrepareSignalData_Expect(&testMaximumSoc0, cantx_testSignalMaximumSoc);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT1/2: Call function under test */
    uint64_t testResult = TEST_CANTX_CalculateMaximumStringSoc(testStringNumber, &can_kShim);
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);

    /* ======= RT2/2: Test implementation */
    testStringNumber = 1u;
    CAN_TxPrepareSignalData_Expect(&testMaximumSoc1, cantx_testSignalMaximumSoc);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT2/2: Call function under test */
    testResult = TEST_CANTX_CalculateMaximumStringSoc(testStringNumber, &can_kShim);
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);
}

/**
 * @brief   Testing CANTX_CalculateMinimumStringSoe
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: BS_NR_OF_STRINGS for stringNumber -> assert
 *            - AT2/2: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/2: Signal data is prepared as expected for string 0
 *            - RT2/2: Signal data is prepared as expected for string 1
 */
void testCANTX_CalculateMinimumStringSoe(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t testStringNumber = 0u;
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateMinimumStringSoe(BS_NR_OF_STRINGS, &can_kShim));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateMinimumStringSoe(testStringNumber, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    CAN_TxPrepareSignalData_Expect(&testMinimumSoe0, cantx_testSignalMinimumSoe);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT1/2: Call function under test */
    uint64_t testResult = TEST_CANTX_CalculateMinimumStringSoe(testStringNumber, &can_kShim);
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);

    /* ======= RT2/2: Test implementation */
    testStringNumber = 1u;
    CAN_TxPrepareSignalData_Expect(&testMinimumSoe1, cantx_testSignalMinimumSoe);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT2/2: Call function under test */
    testResult = TEST_CANTX_CalculateMinimumStringSoe(testStringNumber, &can_kShim);
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);
}

/**
 * @brief   Testing CANTX_CalculateMaximumStringSoe
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: BS_NR_OF_STRINGS for stringNumber -> assert
 *            - AT2/2: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/2: Signal data is prepared as expected for string 0
 *            - RT2/2: Signal data is prepared as expected for string 1
 */
void testCANTX_CalculateMaximumStringSoe(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t testStringNumber = 0u;
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateMaximumStringSoe(BS_NR_OF_STRINGS, &can_kShim));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateMaximumStringSoe(testStringNumber, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    CAN_TxPrepareSignalData_Expect(&testMaximumSoe0, cantx_testSignalMaximumSoe);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT1/2: Call function under test */
    uint64_t testResult = TEST_CANTX_CalculateMaximumStringSoe(testStringNumber, &can_kShim);
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);

    /* ======= RT2/2: Test implementation */
    testStringNumber = 1u;
    CAN_TxPrepareSignalData_Expect(&testMaximumSoe1, cantx_testSignalMaximumSoe);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT2/2: Call function under test */
    testResult = TEST_CANTX_CalculateMaximumStringSoe(testStringNumber, &can_kShim);
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);
}

/**
 * @brief   Testing CANTX_CalculateStringSoh
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: BS_NR_OF_STRINGS for stringNumber -> assert
 *            - AT2/2: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/2: Signal data is prepared as expected for string 0
 *            - RT2/2: Signal data is prepared as expected for string 1
 */
void testCANTX_CalculateStringSoh(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t testStringNumber = 0u;
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateStringSoh(BS_NR_OF_STRINGS, &can_kShim));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateStringSoh(testStringNumber, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    CAN_TxPrepareSignalData_Expect(&testSoh, cantx_testSignalSoh);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT1/2: Call function under test */
    uint64_t testResult = TEST_CANTX_CalculateStringSoh(testStringNumber, &can_kShim);
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);

    /* ======= RT2/2: Test implementation */
    testStringNumber = 1u;
    CAN_TxPrepareSignalData_Expect(&testSoh, cantx_testSignalSoh);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT2/2: Call function under test */
    testResult = TEST_CANTX_CalculateStringSoh(testStringNumber, &can_kShim);
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);
}

/**
 * @brief   Testing CANTX_CalculateStringEnergy
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: BS_NR_OF_STRINGS for stringNumber -> assert
 *            - AT2/2: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/2: Signal data is prepared as expected for string 0
 *            - RT2/2: Signal data is prepared as expected for string 1
 */
void testCANTX_CalculateStringEnergy(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t testStringNumber = 0u;
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateStringEnergy(BS_NR_OF_STRINGS, &can_kShim));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateStringEnergy(testStringNumber, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    CAN_TxPrepareSignalData_Expect(&testMinimumSoeEnergy0, cantx_testSignalMinimumSoeEnergy);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT1/2: Call function under test */
    uint64_t testResult = TEST_CANTX_CalculateStringEnergy(testStringNumber, &can_kShim);
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);

    /* ======= RT2/2: Test implementation */
    testStringNumber = 1u;
    CAN_TxPrepareSignalData_Expect(&testMinimumSoeEnergy1, cantx_testSignalMinimumSoeEnergy);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT2/2: Call function under test */
    testResult = TEST_CANTX_CalculateStringEnergy(testStringNumber, &can_kShim);
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);
}

/**
 * @brief   Testing CANTX_BuildStringStateEstimationMessage
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/3: NULL_PTR for pMessageData -> assert
 *            - AT2/3: BS_NR_OF_STRINGS for stringNumber -> assert
 *            - AT3/3: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/1: Signal data is prepared as expected for string 0
 *            - RT1/1: Signal data is prepared as expected for string 1
 */
void testCANTX_BuildStringStateEstimationMessage(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t testStringNumber = 0u;
    /* ======= AT1/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_BuildStringStateEstimationMessage(NULL_PTR, testStringNumber, &can_kShim));
    /* ======= AT2/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(
        TEST_CANTX_BuildStringStateEstimationMessage(&testMessageData[0u], BS_NR_OF_STRINGS, &can_kShim));
    /* ======= AT3/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(
        TEST_CANTX_BuildStringStateEstimationMessage(&testMessageData[0u], testStringNumber, NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint64_t testResult = 0u;
    /* ======= RT1/2: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 4u, 0u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxPrepareSignalData_Expect(&testMinimumSoc0, cantx_testSignalMinimumSoc);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 3u, 9u, testMinimumSoc0, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxPrepareSignalData_Expect(&testMaximumSoc0, cantx_testSignalMaximumSoc);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 10u, 9u, testMaximumSoc0, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxPrepareSignalData_Expect(&testMinimumSoe0, cantx_testSignalMinimumSoe);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 17u, 9u, testMinimumSoe0, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    CAN_TxPrepareSignalData_Expect(&testMaximumSoe0, cantx_testSignalMaximumSoe);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[4u], 24u, 9u, testMaximumSoe0, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);
    CAN_TxPrepareSignalData_Expect(&testSoh, cantx_testSignalSoh);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[5u], 47, 9u, testSoh, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[6u]);
    CAN_TxPrepareSignalData_Expect(&testMinimumSoeEnergy0, cantx_testSignalMinimumSoeEnergy);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[6u], 54, 15u, testMinimumSoeEnergy0, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[7u]);
    /* ======= RT1/2: Call function under test */
    TEST_CANTX_BuildStringStateEstimationMessage(&testResult, testStringNumber, &can_kShim);
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(7u, testResult);

    testResult = 0u;
    /* ======= RT1/2: Test implementation */
    testStringNumber = 1u;
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 4u, 1u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxPrepareSignalData_Expect(&testMinimumSoc1, cantx_testSignalMinimumSoc);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 3u, 9u, testMinimumSoc1, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxPrepareSignalData_Expect(&testMaximumSoc1, cantx_testSignalMaximumSoc);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 10u, 9u, testMaximumSoc1, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxPrepareSignalData_Expect(&testMinimumSoe1, cantx_testSignalMinimumSoe);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 17u, 9u, testMinimumSoe1, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    CAN_TxPrepareSignalData_Expect(&testMaximumSoe1, cantx_testSignalMaximumSoe);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[4u], 24u, 9u, testMaximumSoe1, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);
    CAN_TxPrepareSignalData_Expect(&testSoh, cantx_testSignalSoh);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[5u], 47, 9u, testSoh, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[6u]);
    CAN_TxPrepareSignalData_Expect(&testMinimumSoeEnergy1, cantx_testSignalMinimumSoeEnergy);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[6u], 54, 15u, testMinimumSoeEnergy1, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[7u]);
    /* ======= RT1/2: Call function under test */
    TEST_CANTX_BuildStringStateEstimationMessage(&testResult, testStringNumber, &can_kShim);
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(7u, testResult);
}

/**
 * @brief   Testing CANTX_StringStateEstimation
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/8: Invalid message id -> assert
 *            - AT2/8: Invalid message id type -> assert
 *            - AT3/8: Invalid message dlc size -> assert
 *            - AT4/8: Invalid message endianness -> assert
 *            - AT5/8: NULL_PTR for pCanData -> assert
 *            - AT6/8: NULL_PTR for pMuxId -> assert
 *            - AT6/8: NULL_PTR for pMuxId -> assert
 *            - AT8/8: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/2: Message data is set as expected for string 0
 *            - RT2/2: Message data is set as expected for string 1
 */
void testCANTX_StringStateEstimation(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t testCanData[CAN_MAX_DLC] = {0u};
    uint8_t testMuxId                = 0u;

    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = 0x245,
        .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
        .dlc        = CAN_FOXBMS_MESSAGES_DEFAULT_DLC,
        .endianness = CAN_BIG_ENDIAN,
    };
    /* ======= AT1/8 ======= */
    testMessage.id = CAN_MAX_11BIT_ID;
    TEST_ASSERT_FAIL_ASSERT(CANTX_StringStateEstimation(testMessage, testCanData, &testMuxId, &can_kShim));
    testMessage.id = 0x245;
    /* ======= AT2/8 ======= */
    testMessage.idType = CAN_EXTENDED_IDENTIFIER_29_BIT;
    TEST_ASSERT_FAIL_ASSERT(CANTX_StringStateEstimation(testMessage, testCanData, &testMuxId, &can_kShim));
    testMessage.idType = CAN_STANDARD_IDENTIFIER_11_BIT;
    /* ======= AT3/8 ======= */
    testMessage.dlc = 9u;
    TEST_ASSERT_FAIL_ASSERT(CANTX_StringStateEstimation(testMessage, testCanData, &testMuxId, &can_kShim));
    testMessage.dlc = 8u;
    /* ======= AT4/8 ======= */
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CANTX_StringStateEstimation(testMessage, testCanData, &testMuxId, &can_kShim));
    testMessage.endianness = CAN_BIG_ENDIAN;
    /* ======= AT5/8 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_StringStateEstimation(testMessage, NULL_PTR, &testMuxId, &can_kShim));
    /* ======= AT6/8 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_StringStateEstimation(testMessage, testCanData, NULL_PTR, &can_kShim));
    /* ======= AT6/8 ======= */
    testMuxId = BS_NR_OF_STRINGS;
    TEST_ASSERT_FAIL_ASSERT(CANTX_StringStateEstimation(testMessage, testCanData, &testMuxId, &can_kShim));
    testMuxId = 0u;
    /* ======= AT8/8 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_StringStateEstimation(testMessage, testCanData, &testMuxId, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    /*float_t testMinimumSoc0       = 15;
    float_t testMaximumSoc0       = 95;
    float_t testMinimumSoe0       = 10;
    float_t testMaximumSoe0       = 90;
    float_t testMinimumSoeEnergy0 = 12300.0f;

    float_t testMinimumSoc1       = 10;
    float_t testMaximumSoc1       = 90;
    float_t testMinimumSoe1       = 15;
    float_t testMaximumSoe1       = 95;
    float_t testMinimumSoeEnergy1 = 45600.0f;*/
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 4u, 0u, CAN_BIG_ENDIAN);
    CAN_TxPrepareSignalData_Expect(&testMinimumSoc0, cantx_testSignalMinimumSoc);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 3u, 9u, testMinimumSoc0, CAN_BIG_ENDIAN);
    CAN_TxPrepareSignalData_Expect(&testMaximumSoc0, cantx_testSignalMaximumSoc);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 10u, 9u, testMaximumSoc0, CAN_BIG_ENDIAN);
    CAN_TxPrepareSignalData_Expect(&testMinimumSoe0, cantx_testSignalMinimumSoe);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 17u, 9u, testMinimumSoe0, CAN_BIG_ENDIAN);
    CAN_TxPrepareSignalData_Expect(&testMaximumSoe0, cantx_testSignalMaximumSoe);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 24u, 9u, testMaximumSoe0, CAN_BIG_ENDIAN);
    CAN_TxPrepareSignalData_Expect(&testSoh, cantx_testSignalSoh);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 47, 9u, testSoh, CAN_BIG_ENDIAN);
    CAN_TxPrepareSignalData_Expect(&testMinimumSoeEnergy0, cantx_testSignalMinimumSoeEnergy);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 54, 15u, testMinimumSoeEnergy0, CAN_BIG_ENDIAN);

    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanData, CAN_BIG_ENDIAN);
    /* ======= RT1/2: Call function under test */
    uint64_t testResult = CANTX_StringStateEstimation(testMessage, testCanData, &testMuxId, &can_kShim);
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(0u, testResult);

    /* ======= RT2/2: Test implementation */
    testMuxId = 1u;
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 4u, 1u, CAN_BIG_ENDIAN);
    CAN_TxPrepareSignalData_Expect(&testMinimumSoc1, cantx_testSignalMinimumSoc);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 3u, 9u, testMinimumSoc1, CAN_BIG_ENDIAN);
    CAN_TxPrepareSignalData_Expect(&testMaximumSoc1, cantx_testSignalMaximumSoc);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 10u, 9u, testMaximumSoc1, CAN_BIG_ENDIAN);
    CAN_TxPrepareSignalData_Expect(&testMinimumSoe1, cantx_testSignalMinimumSoe);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 17u, 9u, testMinimumSoe1, CAN_BIG_ENDIAN);
    CAN_TxPrepareSignalData_Expect(&testMaximumSoe1, cantx_testSignalMaximumSoe);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 24u, 9u, testMaximumSoe1, CAN_BIG_ENDIAN);
    CAN_TxPrepareSignalData_Expect(&testSoh, cantx_testSignalSoh);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 47, 9u, testSoh, CAN_BIG_ENDIAN);
    CAN_TxPrepareSignalData_Expect(&testMinimumSoeEnergy1, cantx_testSignalMinimumSoeEnergy);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 54, 15u, testMinimumSoeEnergy1, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanData, CAN_BIG_ENDIAN);
    /* ======= RT2/2: Call function under test */
    testResult = CANTX_StringStateEstimation(testMessage, testCanData, &testMuxId, &can_kShim);
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(0u, testResult);
    TEST_ASSERT_EQUAL(0u, testMuxId);
}
