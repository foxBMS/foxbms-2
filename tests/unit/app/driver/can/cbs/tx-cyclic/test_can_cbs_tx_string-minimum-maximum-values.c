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
 * @file    test_can_cbs_tx_string-minimum-maximum-values.c
 * @author  foxBMS Team
 * @date    2021-04-22 (date of creation)
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
TEST_SOURCE_FILE("can_cbs_tx_string-minimum-maximum-values.c")

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
uint64_t testMessageData[6u] = {0u, 1u, 2u, 3u, 4u, 5u};

float_t testSignalData[5u] = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f};

float_t testMinimumCellVoltage0 = 5775u;
float_t testMaximumCellVoltage0 = 12345u;
float_t testMinimumTemperature0 = 5u;
float_t testMaximumTemperature0 = 500u;

float_t testMinimumCellVoltage1 = 4554u;
float_t testMaximumCellVoltage1 = 13531u;
float_t testMinimumTemperature1 = 50u;
float_t testMaximumTemperature1 = 550u;

static const CAN_SIGNAL_TYPE_s cantx_testSignalMaximumTemperature = {35u, 9u, 5.0f, 0.0f, -1280.0f, 1275.0f};

static const CAN_SIGNAL_TYPE_s cantx_testSignalMinimumTemperature = {42u, 9u, 5.0f, 0.0f, -1280.0f, 1275.0f};

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
static DATA_BLOCK_MSL_FLAG_s can_tableMslFlags = {.header.uniqueId = DATA_BLOCK_ID_MSL_FLAG};
static DATA_BLOCK_RSL_FLAG_s can_tableRslFlags = {.header.uniqueId = DATA_BLOCK_ID_RSL_FLAG};
static DATA_BLOCK_MOL_FLAG_s can_tableMolFlags = {.header.uniqueId = DATA_BLOCK_ID_MOL_FLAG};

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
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    can_tableMinimumMaximumValues.minimumCellVoltage_mV[0u]    = (int16_t)testMinimumCellVoltage0;
    can_tableMinimumMaximumValues.maximumCellVoltage_mV[0u]    = (int16_t)testMaximumCellVoltage0;
    can_tableMinimumMaximumValues.minimumTemperature_ddegC[0u] = (int16_t)testMinimumTemperature0;
    can_tableMinimumMaximumValues.maximumTemperature_ddegC[0u] = (int16_t)testMaximumTemperature0;

    can_tableMinimumMaximumValues.minimumCellVoltage_mV[1u]    = (int16_t)testMinimumCellVoltage1;
    can_tableMinimumMaximumValues.maximumCellVoltage_mV[1u]    = (int16_t)testMaximumCellVoltage1;
    can_tableMinimumMaximumValues.minimumTemperature_ddegC[1u] = (int16_t)testMinimumTemperature1;
    can_tableMinimumMaximumValues.maximumTemperature_ddegC[1u] = (int16_t)testMaximumTemperature1;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/**
 * @brief   Testing CANTX_CalculateStringMaximumTemperature
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: BS_NR_OF_STRINGS for stringNumber -> assert
 *            - AT2/2: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/2: Signal data is prepared as expected for string 0
 *            - RT2/2: Signal data is prepared as expected for string 1
 */
void testCANTX_CalculateStringMaximumTemperature(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t testStringNumber = 0u;
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateStringMaximumTemperature(BS_NR_OF_STRINGS, &can_kShim));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateStringMaximumTemperature(testStringNumber, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    CAN_TxPrepareSignalData_Expect(&testMaximumTemperature0, cantx_testSignalMaximumTemperature);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT1/2: Call function under test */
    uint64_t testResult = TEST_CANTX_CalculateStringMaximumTemperature(testStringNumber, &can_kShim);
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);

    /* ======= RT2/2: Test implementation */
    testStringNumber = 1u;
    CAN_TxPrepareSignalData_Expect(&testMaximumTemperature1, cantx_testSignalMaximumTemperature);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT2/2: Call function under test */
    testResult = TEST_CANTX_CalculateStringMaximumTemperature(testStringNumber, &can_kShim);
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);
}

/**
 * @brief   Testing CANTX_CalculateStringMinimumTemperature
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: BS_NR_OF_STRINGS for stringNumber -> assert
 *            - AT2/2: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/2: Signal data is prepared as expected for string 0
 *            - RT2/2: Signal data is prepared as expected for string 1
 */
void testCANTX_CalculateStringMinimumTemperature(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t testStringNumber = 0u;
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateStringMinimumTemperature(BS_NR_OF_STRINGS, &can_kShim));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateStringMinimumTemperature(testStringNumber, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    CAN_TxPrepareSignalData_Expect(&testMinimumTemperature0, cantx_testSignalMinimumTemperature);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT1/2: Call function under test */
    uint64_t testResult = TEST_CANTX_CalculateStringMinimumTemperature(testStringNumber, &can_kShim);
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);

    /* ======= RT2/2: Test implementation */
    testStringNumber = 1u;
    CAN_TxPrepareSignalData_Expect(&testMinimumTemperature1, cantx_testSignalMinimumTemperature);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT2/2: Call function under test */
    testResult = TEST_CANTX_CalculateStringMinimumTemperature(testStringNumber, &can_kShim);
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);
}

/**
 * @brief   Testing CANTX_BuildStringMessage
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/3: NULL_PTR for pMessageData -> assert
 *            - AT2/3: BS_NR_OF_STRINGS for stringNumber -> assert
 *            - AT3/3: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/1: Signal data is prepared as expected for string 0
 *            - RT1/1: Signal data is prepared as expected for string 1
 */
void testCANTX_BuildStringMessage(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t testStringNumber = 0u;
    /* ======= AT1/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_BuildStringMessage(NULL_PTR, testStringNumber, &can_kShim));
    /* ======= AT2/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_BuildStringMessage(&testMessageData[0u], BS_NR_OF_STRINGS, &can_kShim));
    /* ======= AT3/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_BuildStringMessage(&testMessageData[0u], testStringNumber, NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint64_t testResult = 0u;
    /* ======= RT1/2: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 3u, 4u, 0u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 15u, 14u, testMaximumCellVoltage0, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 17u, 14u, testMinimumCellVoltage0, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxPrepareSignalData_Expect(&testMaximumTemperature0, cantx_testSignalMaximumTemperature);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 35u, 9u, testMaximumTemperature0, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    CAN_TxPrepareSignalData_Expect(&testMinimumTemperature0, cantx_testSignalMinimumTemperature);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[4u], 42u, 9u, testMinimumTemperature0, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);
    /* ======= RT1/2: Call function under test */
    TEST_CANTX_BuildStringMessage(&testResult, testStringNumber, &can_kShim);
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(5u, testResult);

    testResult = 0u;
    /* ======= RT2/2: Test implementation */
    testStringNumber = 1u;
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 3u, 4u, 1u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 15u, 14u, testMaximumCellVoltage1, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 17u, 14u, testMinimumCellVoltage1, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxPrepareSignalData_Expect(&testMaximumTemperature1, cantx_testSignalMaximumTemperature);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 35u, 9u, testMaximumTemperature1, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    CAN_TxPrepareSignalData_Expect(&testMinimumTemperature1, cantx_testSignalMinimumTemperature);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[4u], 42u, 9u, testMinimumTemperature1, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);
    /* ======= RT2/2: Call function under test */
    TEST_CANTX_BuildStringMessage(&testResult, testStringNumber, &can_kShim);
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(5u, testResult);
}

/**
 * @brief   Testing CANTX_StringMinimumMaximumValues
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/8: Invalid message id -> assert
 *            - AT2/8: Invalid message id type -> assert
 *            - AT3/8: Invalid message dlc size -> assert
 *            - AT4/8: Invalid message endianness -> assert
 *            - AT5/8: NULL_PTR for pCanData -> assert
 *            - AT6/8: NULL_PTR for pMuxId -> assert
 *            - AT7/8: &BS_NR_OF_STRINGS for pMuxId -> assert
 *            - AT8/8: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/2: Message data is set as expected for string 0
 *            - RT2/2: Message data is set as expected for string 1
 */
void testCANTX_StringMinimumMaximumValues(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t testCanData[CAN_MAX_DLC] = {0u};
    uint8_t testMuxId                = 0u;

    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = 0x241,
        .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
        .dlc        = CAN_FOXBMS_MESSAGES_DEFAULT_DLC,
        .endianness = CAN_BIG_ENDIAN,
    };
    /* ======= AT1/8 ======= */
    testMessage.id = CAN_MAX_11BIT_ID;
    TEST_ASSERT_FAIL_ASSERT(CANTX_StringMinimumMaximumValues(testMessage, testCanData, &testMuxId, &can_kShim));
    testMessage.id = 0x241;
    /* ======= AT2/8 ======= */
    testMessage.idType = CAN_EXTENDED_IDENTIFIER_29_BIT;
    TEST_ASSERT_FAIL_ASSERT(CANTX_StringMinimumMaximumValues(testMessage, testCanData, &testMuxId, &can_kShim));
    testMessage.idType = CAN_STANDARD_IDENTIFIER_11_BIT;
    /* ======= AT3/8 ======= */
    testMessage.dlc = 9u;
    TEST_ASSERT_FAIL_ASSERT(CANTX_StringMinimumMaximumValues(testMessage, testCanData, &testMuxId, &can_kShim));
    testMessage.dlc = 8u;
    /* ======= AT4/8 ======= */
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CANTX_StringMinimumMaximumValues(testMessage, testCanData, &testMuxId, &can_kShim));
    testMessage.endianness = CAN_BIG_ENDIAN;
    /* ======= AT5/8 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_StringMinimumMaximumValues(testMessage, NULL_PTR, &testMuxId, &can_kShim));
    /* ======= AT6/8 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_StringMinimumMaximumValues(testMessage, testCanData, NULL_PTR, &can_kShim));
    /* ======= AT7/8 ======= */
    testMuxId = BS_NR_OF_STRINGS;
    TEST_ASSERT_FAIL_ASSERT(CANTX_StringMinimumMaximumValues(testMessage, testCanData, &testMuxId, &can_kShim));
    testMuxId = 0u;
    /* ======= AT8/8 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_StringMinimumMaximumValues(testMessage, testCanData, &testMuxId, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 3u, 4u, 0u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 14u, testMaximumCellVoltage0, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 17u, 14u, testMinimumCellVoltage0, CAN_BIG_ENDIAN);
    CAN_TxPrepareSignalData_Expect(&testMaximumTemperature0, cantx_testSignalMaximumTemperature);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 35u, 9u, testMaximumTemperature0, CAN_BIG_ENDIAN);
    CAN_TxPrepareSignalData_Expect(&testMinimumTemperature0, cantx_testSignalMinimumTemperature);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 42u, 9u, testMinimumTemperature0, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanData, CAN_BIG_ENDIAN);
    /* ======= RT1/2: Call function under test */
    uint64_t testResult = CANTX_StringMinimumMaximumValues(testMessage, testCanData, &testMuxId, &can_kShim);
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(0u, testResult);

    /* ======= RT2/2: Test implementation */
    testMuxId = 1u;
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 3u, 4u, 1u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 14u, testMaximumCellVoltage1, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 17u, 14u, testMinimumCellVoltage1, CAN_BIG_ENDIAN);
    CAN_TxPrepareSignalData_Expect(&testMaximumTemperature1, cantx_testSignalMaximumTemperature);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 35u, 9u, testMaximumTemperature1, CAN_BIG_ENDIAN);
    CAN_TxPrepareSignalData_Expect(&testMinimumTemperature1, cantx_testSignalMinimumTemperature);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 42u, 9u, testMinimumTemperature1, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanData, CAN_BIG_ENDIAN);
    /* ======= RT2/2: Call function under test */
    testResult = CANTX_StringMinimumMaximumValues(testMessage, testCanData, &testMuxId, &can_kShim);
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(0u, testResult);
    TEST_ASSERT_EQUAL(0u, testMuxId);
}
