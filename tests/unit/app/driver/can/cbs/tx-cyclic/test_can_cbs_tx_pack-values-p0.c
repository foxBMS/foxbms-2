/**
 *
 * @copyright &copy; 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    test_can_cbs_tx_pack-values-p0.c
 * @author  foxBMS Team
 * @date    2021-07-27 (date of creation)
 * @updated 2024-08-08 (date of last update)
 * @version v1.7.0
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
#include "Mockdiag.h"
#include "Mockfoxmath.h"
#include "Mockimd.h"
#include "Mockos.h"

#include "database_cfg.h"

#include "can_cbs_tx_cyclic.h"
#include "can_cfg_tx-cyclic-message-definitions.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cbs_tx_pack-values-p0.c")

TEST_INCLUDE_PATH("../../src/app/application/bms")
TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/tx-cyclic")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/fram")
TEST_INCLUDE_PATH("../../src/app/driver/imd")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/engine/sys_mon")
TEST_INCLUDE_PATH("../../src/app/task/config")

/*========== Definitions and Implementations for Unit Test ==================*/
uint64_t testMessageData[5u] = {0u, 1u, 2u, 3u, 4u};

float_t testSignalData[5u] = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f};

float_t testBatteryVoltage = 324800;
float_t testBusVoltage     = 325100;
float_t testCurrent        = -200500;
float_t testPower          = -65100;

static const CAN_SIGNAL_TYPE_s cantx_testSignalBatteryVoltage = {7u, 15u, 100.0f, 0.0f, -1638400.0f, 1638300.0f};
static const CAN_SIGNAL_TYPE_s cantx_testSignalBusVoltage     = {8u, 15u, 100.0f, 0.0f, -1638400.0f, 1638300.0f};
static const CAN_SIGNAL_TYPE_s cantx_testSignalPower          = {25u, 17u, 10.0f, 0.0f, -655360.0f, 655350.0f};
static const CAN_SIGNAL_TYPE_s cantx_testSignalCurrent        = {40u, 17u, 10.0f, 0.0f, -655360.0f, 655350.0f};

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
    can_kShim.pTablePackValues->packCurrent_mA           = (int32_t)testCurrent;
    can_kShim.pTablePackValues->batteryVoltage_mV        = (int32_t)testBatteryVoltage;
    can_kShim.pTablePackValues->highVoltageBusVoltage_mV = (int32_t)testBusVoltage;
    can_kShim.pTablePackValues->packPower_W              = (int32_t)testPower;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/**
 * @brief   Testing CANTX_CalculateBatteryVoltage
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/1: Signal data is prepared as expected
 */
void testCANTX_CalculateBatteryVoltage(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateBatteryVoltage(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    CAN_TxPrepareSignalData_Expect(&testBatteryVoltage, cantx_testSignalBatteryVoltage);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT1/1: Call function under test */
    uint64_t testResult = TEST_CANTX_CalculateBatteryVoltage(&can_kShim);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(1.0, testResult);
}

/**
 * @brief   Testing CANTX_CalculateBusVoltage
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/1: Signal data is prepared as expected
 */
void testCANTX_CalculateBusVoltage(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateBusVoltage(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    CAN_TxPrepareSignalData_Expect(&testBusVoltage, cantx_testSignalBusVoltage);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT1/1: Call function under test */
    uint64_t testResult = TEST_CANTX_CalculateBusVoltage(&can_kShim);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);
}

/**
 * @brief   Testing CANTX_CalculatePower
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/1: Signal data is prepared as expected
 */
void testCANTX_CalculatePower(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculatePower(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    CAN_TxPrepareSignalData_Expect(&testPower, cantx_testSignalPower);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT1/1: Call function under test */
    uint64_t testResult = TEST_CANTX_CalculatePower(&can_kShim);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);
}

/**
 * @brief   Testing CANTX_CalculateCurrent
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/1: Signal data is prepared as expected
 */
void testCANTX_CalculateCurrent(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateCurrent(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    CAN_TxPrepareSignalData_Expect(&testCurrent, cantx_testSignalCurrent);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT1/1: Call function under test */
    uint64_t testResult = TEST_CANTX_CalculateCurrent(&can_kShim);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);
}

/**
 * @brief   Testing CANTX_BuildP0Message
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: NULL_PTR for kpkCanShim -> assert
 *            - AT2/2: NULL_PTR for pMessageData -> assert
 *          - Routine validation:
 *            - RT1/1: Message data is set as expected
 */
void testCANTX_BuildP0Message(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_BuildP0Message(NULL_PTR, &testMessageData[0]));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_BuildP0Message(&can_kShim, NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint64_t testResult = 0u;
    /* ======= RT1/1: Test implementation */
    CAN_TxPrepareSignalData_Expect(&testBatteryVoltage, cantx_testSignalBatteryVoltage);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 7u, 15u, testSignalData[1u], CANTX_PACK_VALUES_P0_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxPrepareSignalData_Expect(&testBusVoltage, cantx_testSignalBusVoltage);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 8u, 15u, testSignalData[2u], CANTX_PACK_VALUES_P0_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxPrepareSignalData_Expect(&testPower, cantx_testSignalPower);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[2u], 25u, 17u, testSignalData[3u], CANTX_PACK_VALUES_P0_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxPrepareSignalData_Expect(&testCurrent, cantx_testSignalCurrent);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[4u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[3u], 40u, 17u, testSignalData[4u], CANTX_PACK_VALUES_P0_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_BuildP0Message(&can_kShim, &testResult);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(4u, testResult);
}

/**
 * @brief   Testing CANTX_PackValuesP0
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
void testCANTX_PackValuesP0(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t testCanData[CAN_MAX_DLC] = {0u};
    uint8_t testMuxId                = 0u;

    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = 0x233,
        .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
        .dlc        = CAN_FOXBMS_MESSAGES_DEFAULT_DLC,
        .endianness = CAN_BIG_ENDIAN,
    };
    /* ======= AT1/7 ======= */
    testMessage.id = CAN_MAX_11BIT_ID;
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackValuesP0(testMessage, testCanData, NULL_PTR, &can_kShim));
    testMessage.id = 0x233;
    /* ======= AT2/7 ======= */
    testMessage.idType = CAN_EXTENDED_IDENTIFIER_29_BIT;
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackValuesP0(testMessage, testCanData, NULL_PTR, &can_kShim));
    testMessage.idType = CAN_STANDARD_IDENTIFIER_11_BIT;
    /* ======= AT3/7 ======= */
    testMessage.dlc = 9u;
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackValuesP0(testMessage, testCanData, NULL_PTR, &can_kShim));
    testMessage.dlc = 8u;
    /* ======= AT4/7 ======= */
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackValuesP0(testMessage, testCanData, NULL_PTR, &can_kShim));
    testMessage.endianness = CAN_BIG_ENDIAN;
    /* ======= AT5/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackValuesP0(testMessage, NULL_PTR, NULL_PTR, &can_kShim));
    /* ======= AT6/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackValuesP0(testMessage, testCanData, &testMuxId, &can_kShim));
    /* ======= AT7/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackValuesP0(testMessage, testCanData, NULL_PTR, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    CAN_TxPrepareSignalData_Ignore();
    DATA_Read1DataBlock_ExpectAndReturn(can_kShim.pTablePackValues, STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 7u, 15u, can_tablePackValues.batteryVoltage_mV, CANTX_PACK_VALUES_P0_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 8u, 15u, can_tablePackValues.highVoltageBusVoltage_mV, CANTX_PACK_VALUES_P0_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 25u, 17u, can_tablePackValues.packPower_W, CANTX_PACK_VALUES_P0_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 40u, 17u, can_tablePackValues.packCurrent_mA, CANTX_PACK_VALUES_P0_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanData, CAN_BIG_ENDIAN);
    /* ======= RT1/1: Call function under test */
    uint64_t testResult = CANTX_PackValuesP0(testMessage, testCanData, NULL_PTR, &can_kShim);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(0u, testResult);
}
