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
 * @file    test_can_cbs_tx_cell-temperatures.c
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
#include "Mockcan.h"
#include "Mockcan_helper.h"
#include "Mockdatabase.h"
#include "Mockdatabase_helper.h"
#include "Mockdiag.h"
#include "Mockfoxmath.h"
#include "Mockimd.h"
#include "Mockos.h"

#include "database_cfg.h"

#include "can_cbs_tx_cyclic.h"
#include "can_cfg_tx-cyclic-message-definitions.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cbs_tx_cell-temperatures.c")

TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/tx-cyclic")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/fram")
TEST_INCLUDE_PATH("../../src/app/driver/imd")
TEST_INCLUDE_PATH("../../src/app/engine/database")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/engine/sys_mon")
TEST_INCLUDE_PATH("../../src/app/task/config")

/*========== Definitions and Implementations for Unit Test ==================*/
/** the number of temperatures per message-frame */
#define CANTX_NUMBER_OF_MUX_TEMPERATURES_PER_MESSAGE (6u)

uint64_t testMessageData[14u] = {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 11u, 12u, 13u};

float_t testSignalData[3u] = {0u, 1u, 2u};

float_t testCellTemperature0 = 850.0f;
float_t testCellTemperature1 = 570.0f;

static const CAN_SIGNAL_TYPE_s cantx_testCell0TemperatureInvalidFlag = {8u, 1u, 1.0f, 0.0f, 0.0f, 1.0f};
static const CAN_SIGNAL_TYPE_s cantx_testCell1TemperatureInvalidFlag = {9u, 1u, 1.0f, 0.0f, 0.0f, 1.0f};
static const CAN_SIGNAL_TYPE_s cantx_testCell0Temperature_degC       = {23u, 8u, 10.0f, 0.0f, -1280.0f, 1270.0f};
static const CAN_SIGNAL_TYPE_s cantx_testCell1Temperature_degC       = {31u, 8u, 10.0f, 0.0f, -1280.0f, 1270.0f};
static const CAN_SIGNAL_TYPE_s cantx_testCell2Temperature_degC       = {39u, 8u, 10.0f, 0.0f, -1280.0f, 1270.0f};
static const CAN_SIGNAL_TYPE_s cantx_testCell3Temperature_degC       = {47u, 8u, 10.0f, 0.0f, -1280.0f, 1270.0f};
static const CAN_SIGNAL_TYPE_s cantx_testCell4Temperature_degC       = {55u, 8u, 10.0f, 0.0f, -1280.0f, 1270.0f};
static const CAN_SIGNAL_TYPE_s cantx_testCell5Temperature_degC       = {63u, 8u, 10.0f, 0.0f, -1280.0f, 1270.0f};

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
    can_kShim.pTableCellTemperature->invalidCellTemperature[0u][0u][0u] = 0u;
    can_kShim.pTableCellTemperature->invalidCellTemperature[0u][0u][1u] = 1u;

    can_kShim.pTableCellTemperature->cellTemperature_ddegC[0u][0u][0u] = (int16_t)testCellTemperature0;
    can_kShim.pTableCellTemperature->cellTemperature_ddegC[0u][0u][1u] = (int16_t)testCellTemperature1;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/**
 * @brief   Testing CANTX_TemperatureSetData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/4: BS_NR_OF_TEMP_SENSORS for testSensorId -> assert
 *            - AT2/4: NULL_PTR for pMessage -> assert
 *            - AT3/4: CAN_LITTLE_ENDIAN for endianness -> assert
 *            - AT4/4: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/2: Signal data is valid
 *            - RT2/2: Signal data is invalid
 */
void testCANTX_TemperatureSetData(void) {
    /* ======= Assertion tests ============================================= */
    uint16_t testSensorId = 0u;
    /* ======= AT1/4 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_TemperatureSetData(
        BS_NR_OF_TEMP_SENSORS,
        &testMessageData[0u],
        cantx_testCell0Temperature_degC,
        cantx_testCell0TemperatureInvalidFlag,
        CAN_BIG_ENDIAN,
        &can_kShim));
    /* ======= AT2/4 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_TemperatureSetData(
        testSensorId,
        NULL_PTR,
        cantx_testCell0Temperature_degC,
        cantx_testCell0TemperatureInvalidFlag,
        CAN_BIG_ENDIAN,
        &can_kShim));
    /* ======= AT3/4 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_TemperatureSetData(
        testSensorId,
        &testMessageData[0u],
        cantx_testCell0Temperature_degC,
        cantx_testCell0TemperatureInvalidFlag,
        CAN_LITTLE_ENDIAN,
        &can_kShim));
    /* ======= AT4/4 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_TemperatureSetData(
        testSensorId,
        &testMessageData[0u],
        cantx_testCell0Temperature_degC,
        cantx_testCell0TemperatureInvalidFlag,
        CAN_BIG_ENDIAN,
        NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint64_t messageData = 0u;
    /* ======= RT1/2: Test implementation */
    DATA_GetStringNumberFromTemperatureIndex_ExpectAndReturn(testSensorId, 0u);
    DATA_GetModuleNumberFromTemperatureIndex_ExpectAndReturn(testSensorId, 0u);
    DATA_GetSensorNumberFromTemperatureIndex_ExpectAndReturn(testSensorId, 0u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 8u, 1u, 1u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxPrepareSignalData_Expect(&testCellTemperature0, cantx_testCell0Temperature_degC);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 23u, 8u, (uint64_t)testSignalData[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    /* ======= RT1/2: Call function under test */
    TEST_CANTX_TemperatureSetData(
        testSensorId,
        &messageData,
        cantx_testCell0Temperature_degC,
        cantx_testCell0TemperatureInvalidFlag,
        CAN_BIG_ENDIAN,
        &can_kShim);
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[2u], messageData);

    messageData = 0u;
    /* ======= RT2/2: Test implementation */
    DATA_GetStringNumberFromTemperatureIndex_ExpectAndReturn(testSensorId, 0u);
    DATA_GetModuleNumberFromTemperatureIndex_ExpectAndReturn(testSensorId, 0u);
    DATA_GetSensorNumberFromTemperatureIndex_ExpectAndReturn(testSensorId, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 9u, 1u, 0u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxPrepareSignalData_Expect(&testCellTemperature1, cantx_testCell1Temperature_degC);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 31u, 8u, (uint64_t)testSignalData[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    /* ======= RT2/2: Call function under test */
    TEST_CANTX_TemperatureSetData(
        testSensorId,
        &messageData,
        cantx_testCell1Temperature_degC,
        cantx_testCell1TemperatureInvalidFlag,
        CAN_BIG_ENDIAN,
        &can_kShim);
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[2u], messageData);
}

/**
 * @brief   Testing CANTX_TemperatureSetData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/7: invalid message.id -> assert
 *            - AT2/7: invalid message.idType -> assert
 *            - AT3/7: invalid message.dlc -> assert
 *            - AT4/7: invalid message.endianness -> assert
 *            - AT5/7: NULL_PTR for pCanData -> assert
 *            - AT6/7: NULL_PTR for pMuxId -> assert
 *            - AT7/7: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/2: Setting two temperatures
 *            - RT2/2: Setting six temperatures
 *
 */
void testCANTX_CellTemperatures(void) {
    /* ======= Assertion tests ============================================= */
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = 0x260,
        .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
        .dlc        = 8u,
        .endianness = CAN_BIG_ENDIAN,
    };
    uint8_t testCanData[CAN_MAX_DLC] = {0u};
    uint8_t testMuxId                = 0u;
    /* ======= AT1/7 ======= */
    testMessage.id = CAN_MAX_11BIT_ID;
    TEST_ASSERT_FAIL_ASSERT(CANTX_CellTemperatures(testMessage, testCanData, &testMuxId, &can_kShim));
    testMessage.id = 0x260;
    /* ======= AT2/7 ======= */
    testMessage.idType = CAN_EXTENDED_IDENTIFIER_29_BIT;
    TEST_ASSERT_FAIL_ASSERT(CANTX_CellTemperatures(testMessage, testCanData, &testMuxId, &can_kShim));
    testMessage.idType = CAN_STANDARD_IDENTIFIER_11_BIT;
    /* ======= AT3/7 ======= */
    testMessage.dlc = 9u;
    TEST_ASSERT_FAIL_ASSERT(CANTX_CellTemperatures(testMessage, testCanData, &testMuxId, &can_kShim));
    testMessage.dlc = 8u;
    /* ======= AT4/7 ======= */
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CANTX_CellTemperatures(testMessage, testCanData, &testMuxId, &can_kShim));
    testMessage.endianness = CAN_BIG_ENDIAN;
    /* ======= AT5/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_CellTemperatures(testMessage, NULL_PTR, &testMuxId, &can_kShim));
    /* ======= AT6/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_CellTemperatures(testMessage, testCanData, NULL_PTR, &can_kShim));
    /* ======= AT7/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_CellTemperatures(testMessage, testCanData, &testMuxId, NULL_PTR));

    /* ======= Routine tests =============================================== */
    testMuxId = 1u;
    /* Calculate the global cell ID based on the multiplexer value for the first cell */
    uint16_t temperatureSensorId = (testMuxId * CANTX_NUMBER_OF_MUX_TEMPERATURES_PER_MESSAGE);
    /* ======= RT1/2: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 1u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    DATA_GetStringNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    DATA_GetModuleNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    DATA_GetSensorNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 8u, 1u, 1u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxPrepareSignalData_Expect(&testCellTemperature0, cantx_testCell0Temperature_degC);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 23u, 8u, testSignalData[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    temperatureSensorId++;

    DATA_GetStringNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    DATA_GetModuleNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    DATA_GetSensorNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 9u, 1u, 1u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    CAN_TxPrepareSignalData_Expect(&testCellTemperature0, cantx_testCell1Temperature_degC);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[4u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[4u], 31u, 8u, testSignalData[4u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);

    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[5u], testCanData, CAN_BIG_ENDIAN);

    /* ======= RT1/2: Call function under test */
    uint32_t testResult = CANTX_CellTemperatures(testMessage, testCanData, &testMuxId, &can_kShim);
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(0u, testResult);
    TEST_ASSERT_EQUAL(2u, testMuxId);

    /* ======= RT2/2: Test implementation */
    testMuxId           = 3u;
    temperatureSensorId = 0;

    DATA_Read1DataBlock_ExpectAndReturn(can_kShim.pTableCellTemperature, STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);

    DATA_GetStringNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    DATA_GetModuleNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    DATA_GetSensorNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 8u, 1u, 1u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxPrepareSignalData_Expect(&testCellTemperature0, cantx_testCell0Temperature_degC);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 23u, 8u, testSignalData[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    temperatureSensorId++;

    DATA_GetStringNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    DATA_GetModuleNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    DATA_GetSensorNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 9u, 1u, 1u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    CAN_TxPrepareSignalData_Expect(&testCellTemperature0, cantx_testCell1Temperature_degC);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[4u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[4u], 31u, 8u, testSignalData[4u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);
    temperatureSensorId++;

    DATA_GetStringNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    DATA_GetModuleNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    DATA_GetSensorNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[5u], 10u, 1u, 1u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[6u]);
    CAN_TxPrepareSignalData_Expect(&testCellTemperature0, cantx_testCell2Temperature_degC);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[6u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[6u], 39u, 8u, testSignalData[6u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[7u]);
    temperatureSensorId++;

    DATA_GetStringNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    DATA_GetModuleNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    DATA_GetSensorNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[7u], 11u, 1u, 1u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[8u]);
    CAN_TxPrepareSignalData_Expect(&testCellTemperature0, cantx_testCell3Temperature_degC);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[8u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[8u], 47u, 8u, testSignalData[8u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[9u]);
    temperatureSensorId++;

    DATA_GetStringNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    DATA_GetModuleNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    DATA_GetSensorNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[9u], 12u, 1u, 1u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[10u]);
    CAN_TxPrepareSignalData_Expect(&testCellTemperature0, cantx_testCell4Temperature_degC);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[10u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[10u], 55u, 8u, testSignalData[10u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[11u]);
    temperatureSensorId++;

    DATA_GetStringNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    DATA_GetModuleNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    DATA_GetSensorNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[11u], 13u, 1u, 1u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[12u]);
    CAN_TxPrepareSignalData_Expect(&testCellTemperature0, cantx_testCell5Temperature_degC);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[12u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[12u], 63u, 8u, testSignalData[12u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[13u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[13u], testCanData, CAN_BIG_ENDIAN);
    /* ======= RT2/2: Call function under test */
    testResult = CANTX_CellTemperatures(testMessage, testCanData, &testMuxId, &can_kShim);
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(0u, testResult);
    TEST_ASSERT_EQUAL(1u, testMuxId);
}
