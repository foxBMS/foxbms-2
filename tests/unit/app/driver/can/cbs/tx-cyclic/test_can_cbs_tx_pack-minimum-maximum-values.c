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
 * @file    test_can_cbs_tx_pack-minimum-maximum-values.c
 * @author  foxBMS Team
 * @date    2021-04-22 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
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

#include <stdbool.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cbs_tx_pack-minimum-maximum-values.c")

TEST_INCLUDE_PATH("../../src/app/application/bms")
TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/tx-cyclic")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/contactor")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/fram")
TEST_INCLUDE_PATH("../../src/app/driver/imd")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/driver/sps")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/engine/sys_mon")
TEST_INCLUDE_PATH("../../src/app/task/config")

/*========== Definitions and Implementations for Unit Test ==================*/
uint64_t testMessageData[5u] = {0u, 1u, 2u, 3u, 4u};

float_t testSignalData[4u] = {0.0f, 1.0f, 2.0f, 3.0f};

float_t testMinimumCellVoltage0 = 1750;
float_t testMaximumCellVoltage0 = 3000;
float_t testMinimumTemperature0 = -150;
float_t testMaximumTemperature0 = 350;

float_t testMinimumCellVoltage1 = 1800;
float_t testMaximumCellVoltage1 = 1900;
float_t testMinimumTemperature1 = -100;
float_t testMaximumTemperature1 = 300;

static const CAN_SIGNAL_TYPE_s cantx_testSignalMaximumTemperature = {55u, 8u, 10.0f, 0.0f, -1280.0f, 1270.0f};
static const CAN_SIGNAL_TYPE_s cantx_testSignalMinimumTemperature = {63u, 8u, 10.0f, 0.0f, -1280.0f, 1270.0f};

static DATA_BLOCK_CELL_VOLTAGE_s can_tableCellVoltages        = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
static DATA_BLOCK_CELL_TEMPERATURE_s can_tableTemperatures    = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
static DATA_BLOCK_MIN_MAX_s can_tableMinimumMaximumValues     = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
static DATA_BLOCK_CURRENT_SENSOR_s can_tableCurrentSensor     = {.header.uniqueId = DATA_BLOCK_ID_CURRENT_SENSOR};
static DATA_BLOCK_OPEN_WIRE_s can_tableOpenWire               = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};
static DATA_BLOCK_STATE_REQUEST_s can_tableStateRequest       = {.header.uniqueId = DATA_BLOCK_ID_STATE_REQUEST};
static DATA_BLOCK_PACK_VALUES_s can_tablePackValues           = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
static DATA_BLOCK_SOF_s can_tableSof                          = {.header.uniqueId = DATA_BLOCK_ID_SOF};
static DATA_BLOCK_SOC_s can_tableSoc                          = {.header.uniqueId = DATA_BLOCK_ID_SOC};
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
    .pTableErrorState      = &can_tableErrorState,
    .pTableInsulation      = &can_tableInsulation,
    .pTableMsl             = &can_tableMslFlags,
    .pTableRsl             = &can_tableRslFlags,
    .pTableMol             = &can_tableMolFlags,
    .pTableAerosolSensor   = &can_tableAerosolSensor,
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    can_kShim.pTableMinMax->minimumCellVoltage_mV[0u]    = (int16_t)testMinimumCellVoltage0;
    can_kShim.pTableMinMax->maximumCellVoltage_mV[0u]    = (int16_t)testMaximumCellVoltage0;
    can_kShim.pTableMinMax->minimumTemperature_ddegC[0u] = (int16_t)testMinimumTemperature0;
    can_kShim.pTableMinMax->maximumTemperature_ddegC[0u] = (int16_t)testMaximumTemperature0;

    can_kShim.pTableMinMax->minimumCellVoltage_mV[1u]    = (int16_t)testMinimumCellVoltage1;
    can_kShim.pTableMinMax->maximumCellVoltage_mV[1u]    = (int16_t)testMaximumCellVoltage1;
    can_kShim.pTableMinMax->minimumTemperature_ddegC[1u] = (int16_t)testMinimumTemperature1;
    can_kShim.pTableMinMax->maximumTemperature_ddegC[1u] = (int16_t)testMaximumTemperature1;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/**
 * @brief   Testing CANTX_GetPackMaximumVoltage
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/4: Only first string is closed
 *            - RT2/4: Only second string is closed
 *            - RT3/4: Both Strings are closed
 *            - RT4/4: No String is closed
 */
void testCANTX_GetPackMaximumVoltage(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_GetPackMaximumVoltage(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/4: Test implementation */
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(1u);
    BMS_IsStringClosed_ExpectAndReturn(0u, true);
    BMS_IsStringClosed_ExpectAndReturn(1u, false);
    /* ======= RT1/4: Call function under test */
    int16_t testResult = TEST_CANTX_GetPackMaximumVoltage(&can_kShim);
    /* ======= RT1/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableMinimumMaximumValues.maximumCellVoltage_mV[0u], testResult);

    /* ======= RT2/4: Test implementation */
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(1u);
    BMS_IsStringClosed_ExpectAndReturn(0u, false);
    BMS_IsStringClosed_ExpectAndReturn(1u, true);
    /* ======= RT2/4: Call function under test */
    testResult = TEST_CANTX_GetPackMaximumVoltage(&can_kShim);
    /* ======= RT2/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableMinimumMaximumValues.maximumCellVoltage_mV[1u], testResult);

    /* ======= RT3/4: Test implementation */
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(2u);
    BMS_IsStringClosed_ExpectAndReturn(0u, true);
    BMS_IsStringClosed_ExpectAndReturn(1u, true);
    /* ======= RT3/4: Call function under test */
    testResult = TEST_CANTX_GetPackMaximumVoltage(&can_kShim);
    /* ======= RT3/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableMinimumMaximumValues.maximumCellVoltage_mV[0u], testResult);

    /* ======= RT4/4: Test implementation */
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(0u);
    /* ======= RT4/4: Call function under test */
    testResult = TEST_CANTX_GetPackMaximumVoltage(&can_kShim);
    /* ======= RT4/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableMinimumMaximumValues.maximumCellVoltage_mV[0u], testResult);
}

/**
 * @brief   Testing CANTX_GetPackMinimumVoltage
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/4: Only first string is closed
 *            - RT2/4: Only second string is closed
 *            - RT3/4: Both Strings are closed
 *            - RT4/4: No String is closed
 */
void testCANTX_GetPackMinimumVoltage(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_GetPackMinimumVoltage(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/4: Test implementation */
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(1u);
    BMS_IsStringClosed_ExpectAndReturn(0u, true);
    BMS_IsStringClosed_ExpectAndReturn(1u, false);
    /* ======= RT1/4: Call function under test */
    int16_t testResult = TEST_CANTX_GetPackMinimumVoltage(&can_kShim);
    /* ======= RT1/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableMinimumMaximumValues.minimumCellVoltage_mV[0u], testResult);

    /* ======= RT2/4: Test implementation */
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(1u);
    BMS_IsStringClosed_ExpectAndReturn(0u, false);
    BMS_IsStringClosed_ExpectAndReturn(1u, true);
    /* ======= RT2/4: Call function under test */
    testResult = TEST_CANTX_GetPackMinimumVoltage(&can_kShim);
    /* ======= RT2/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableMinimumMaximumValues.minimumCellVoltage_mV[1u], testResult);

    /* ======= RT3/4: Test implementation */
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(2u);
    BMS_IsStringClosed_ExpectAndReturn(0u, true);
    BMS_IsStringClosed_ExpectAndReturn(1u, true);
    /* ======= RT3/4: Call function under test */
    testResult = TEST_CANTX_GetPackMinimumVoltage(&can_kShim);
    /* ======= RT3/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableMinimumMaximumValues.minimumCellVoltage_mV[0u], testResult);

    /* ======= RT4/4: Test implementation */
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(0u);
    /* ======= RT4/4: Call function under test */
    testResult = TEST_CANTX_GetPackMinimumVoltage(&can_kShim);
    /* ======= RT4/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableMinimumMaximumValues.minimumCellVoltage_mV[0u], testResult);
}

/**
 * @brief   Testing CANTX_GetPackMaximumTemperature
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/4: Only first string is closed
 *            - RT2/4: Only second string is closed
 *            - RT3/4: Both Strings are closed
 *            - RT4/4: No String is closed
 */
void testCANTX_GetPackMaximumTemperature(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_GetPackMaximumTemperature(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/4: Test implementation */
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(1u);
    BMS_IsStringClosed_ExpectAndReturn(0u, true);
    BMS_IsStringClosed_ExpectAndReturn(1u, false);
    /* ======= RT1/4: Call function under test */
    int16_t testResult = TEST_CANTX_GetPackMaximumTemperature(&can_kShim);
    /* ======= RT1/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableMinimumMaximumValues.maximumTemperature_ddegC[0u], testResult);

    /* ======= RT2/4: Test implementation */
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(1u);
    BMS_IsStringClosed_ExpectAndReturn(0u, false);
    BMS_IsStringClosed_ExpectAndReturn(1u, true);
    /* ======= RT2/4: Call function under test */
    testResult = TEST_CANTX_GetPackMaximumTemperature(&can_kShim);
    /* ======= RT2/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableMinimumMaximumValues.maximumTemperature_ddegC[1u], testResult);

    /* ======= RT3/4: Test implementation */
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(2u);
    BMS_IsStringClosed_ExpectAndReturn(0u, true);
    BMS_IsStringClosed_ExpectAndReturn(1u, true);
    /* ======= RT3/4: Call function under test */
    testResult = TEST_CANTX_GetPackMaximumTemperature(&can_kShim);
    /* ======= RT3/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableMinimumMaximumValues.maximumTemperature_ddegC[0u], testResult);

    /* ======= RT4/4: Test implementation */
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(0u);
    /* ======= RT4/4: Call function under test */
    testResult = TEST_CANTX_GetPackMaximumTemperature(&can_kShim);
    /* ======= RT4/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableMinimumMaximumValues.maximumTemperature_ddegC[0u], testResult);
}

/**
 * @brief   Testing CANTX_GetPackMinimumTemperature
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/4: Only first string is closed
 *            - RT2/4: Only second string is closed
 *            - RT3/4: Both Strings are closed
 *            - RT4/4: No String is closed
 */
void testCANTX_GetPackMinimumTemperature(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_GetPackMinimumTemperature(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/4: Test implementation */
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(1u);
    BMS_IsStringClosed_ExpectAndReturn(0u, true);
    BMS_IsStringClosed_ExpectAndReturn(1u, false);
    /* ======= RT1/4: Call function under test */
    int16_t testResult = TEST_CANTX_GetPackMinimumTemperature(&can_kShim);
    /* ======= RT1/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableMinimumMaximumValues.minimumTemperature_ddegC[0u], testResult);

    /* ======= RT2/4: Test implementation */
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(1u);
    BMS_IsStringClosed_ExpectAndReturn(0u, false);
    BMS_IsStringClosed_ExpectAndReturn(1u, true);
    /* ======= RT2/4: Call function under test */
    testResult = TEST_CANTX_GetPackMinimumTemperature(&can_kShim);
    /* ======= RT2/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableMinimumMaximumValues.minimumTemperature_ddegC[1u], testResult);

    /* ======= RT3/4: Test implementation */
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(2u);
    BMS_IsStringClosed_ExpectAndReturn(0u, true);
    BMS_IsStringClosed_ExpectAndReturn(1u, true);
    /* ======= RT3/4: Call function under test */
    testResult = TEST_CANTX_GetPackMinimumTemperature(&can_kShim);
    /* ======= RT3/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableMinimumMaximumValues.minimumTemperature_ddegC[0u], testResult);

    /* ======= RT4/4: Test implementation */
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(0u);
    /* ======= RT4/4: Call function under test */
    testResult = TEST_CANTX_GetPackMinimumTemperature(&can_kShim);
    /* ======= RT4/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableMinimumMaximumValues.minimumTemperature_ddegC[0u], testResult);
}

/**
 * @brief   Testing CANTX_CalculatePackMaximumTemperature
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/1: Function prepares signal data as expected
 */
void testCANTX_CalculatePackMaximumTemperature(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculatePackMaximumTemperature(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(0u);
    CAN_TxPrepareSignalData_Expect(&testMaximumTemperature0, cantx_testSignalMaximumTemperature);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT1/1: Call function under test */
    uint64_t testResult = TEST_CANTX_CalculatePackMaximumTemperature(&can_kShim);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);
}

/**
 * @brief   Testing CANTX_CalculatePackMinimumTemperature
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/1: Function prepares signal data as expected
 */
void testCANTX_CalculatePackMinimumTemperature(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculatePackMinimumTemperature(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(0u);
    CAN_TxPrepareSignalData_Expect(&testMinimumTemperature0, cantx_testSignalMinimumTemperature);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT1/1: Call function under test */
    uint64_t testResult = TEST_CANTX_CalculatePackMinimumTemperature(&can_kShim);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);
}

/**
 * @brief   Testing CANTX_BuildPackMinimumMaximumValuesMessage
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: NULL_PTR for kpkCanShim -> assert
 *            - AT2/2: NULL_PTR for pMessageData -> assert
 *          - Routine validation:
 *            - RT1/1: Message data is set as expected
 */
void testCANTX_BuildPackMinimumMaximumValuesMessage(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_BuildPackMinimumMaximumValuesMessage(NULL_PTR, &testMessageData[0u]));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_BuildPackMinimumMaximumValuesMessage(&can_kShim, NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint64_t testResult = 0u;
    /* ======= RT1/1: Test implementation */
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(0u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 14u, testMaximumCellVoltage0, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(0u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 9u, 14u, testMinimumCellVoltage0, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);

    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(0u);
    CAN_TxPrepareSignalData_Expect(&testMaximumTemperature0, cantx_testSignalMaximumTemperature);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 55u, 8u, testMaximumTemperature0, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(0u);
    CAN_TxPrepareSignalData_Expect(&testMinimumTemperature0, cantx_testSignalMinimumTemperature);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 63u, 8u, testMinimumTemperature0, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_BuildPackMinimumMaximumValuesMessage(&can_kShim, &testResult);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(4u, testResult);
}

/**
 * @brief   Testing CANTX_PackMinimumMaximumValues
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/7: Invalid message id -> assert
 *            - AT2/7: Invalid message id type -> assert
 *            - AT3/7: Invalid message dlc size -> assert
 *            - AT4/7: Invalid message endianness -> assert
 *            - AT5/7: NULL_PTR for pCanData -> assert
 *            - AT6/7: NO NULL_PTR for pMuxId -> assert
 *            - AT7/7: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/1: Message data is set as expected
 */
void testCANTX_PackMinimumMaximumValues(void) {
    /* ======= Assertion tests ============================================= */
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = 0x231,
        .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
        .dlc        = 8u,
        .endianness = CAN_BIG_ENDIAN,
    };
    uint8_t muxId                = 0u;
    uint8_t canData[CAN_MAX_DLC] = {0u};
    /* ======= AT1/7 ======= */
    testMessage.id = CAN_MAX_11BIT_ID;
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackMinimumMaximumValues(testMessage, canData, NULL_PTR, &can_kShim));
    testMessage.id = 0x231;
    /* ======= AT2/7 ======= */
    testMessage.idType = CAN_EXTENDED_IDENTIFIER_29_BIT;
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackMinimumMaximumValues(testMessage, canData, NULL_PTR, &can_kShim));
    testMessage.idType = CAN_STANDARD_IDENTIFIER_11_BIT;
    /* ======= AT3/7 ======= */
    testMessage.dlc = 9u;
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackMinimumMaximumValues(testMessage, canData, NULL_PTR, &can_kShim));
    testMessage.dlc = 8u;
    /* ======= AT4/7 ======= */
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackMinimumMaximumValues(testMessage, canData, NULL_PTR, &can_kShim));
    testMessage.endianness = CAN_BIG_ENDIAN;
    /* ======= AT5/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackMinimumMaximumValues(testMessage, NULL_PTR, NULL_PTR, &can_kShim));
    /* ======= AT6/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackMinimumMaximumValues(testMessage, canData, &muxId, &can_kShim));
    /* ======= AT7/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackMinimumMaximumValues(testMessage, canData, NULL_PTR, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    DATA_Read1DataBlock_ExpectAndReturn(can_kShim.pTableMinMax, STD_OK);
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(0u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 14u, testMaximumCellVoltage0, CAN_BIG_ENDIAN);
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(0u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 9u, 14u, testMinimumCellVoltage0, CAN_BIG_ENDIAN);
    float_t signalDataMaxTemp = testMaximumTemperature0;
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(0u);
    CAN_TxPrepareSignalData_Expect(&signalDataMaxTemp, cantx_testSignalMaximumTemperature);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 55u, 8u, testMaximumTemperature0, CAN_BIG_ENDIAN);
    float_t signalDataMinTemp = testMinimumTemperature0;
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(0u);
    CAN_TxPrepareSignalData_Expect(&signalDataMinTemp, cantx_testSignalMinimumTemperature);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 63u, 8u, testMinimumTemperature0, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], canData, testMessage.endianness);
    /* ======= RT1/1: Call function under test */

    uint32_t testResult = CANTX_PackMinimumMaximumValues(testMessage, canData, NULL_PTR, &can_kShim);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(0u, testResult);
}
