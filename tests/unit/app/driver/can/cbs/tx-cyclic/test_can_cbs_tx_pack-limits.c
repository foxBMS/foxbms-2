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
 * @file    test_can_cbs_tx_pack-limits.c
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

#include "battery_cell_cfg.h"
#include "database_cfg.h"

#include "can_cbs_tx_cyclic.h"
#include "can_cfg_tx-cyclic-message-definitions.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cbs_tx_pack-limits.c")

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
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/
uint64_t testMessageData[7u] = {0u, 1u, 2u, 3u, 4u, 5u, 6u};

float_t testSignalData[7u] = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};

static const CAN_SIGNAL_TYPE_s cantx_testSignalMaximumDischargeCurrent = {7u, 12u, 250.0f, 0.0f, 0.0f, 1023750.0f};

static const CAN_SIGNAL_TYPE_s cantx_testSignalMaximumChargeCurrent = {11u, 12u, 250.0f, 0.0f, 0.0f, 1023750.0f};

static const CAN_SIGNAL_TYPE_s cantx_testSignalMaximumDischargePower = {31u, 12u, 100.0f, 0.0f, 0.0f, 409500.0f};

static const CAN_SIGNAL_TYPE_s cantx_testSignalMaximumChargePower = {35u, 12u, 100.0f, 0.0f, 0.0f, 409500.0f};

static const CAN_SIGNAL_TYPE_s cantx_testSignalMaximumBatteryVoltage = {55u, 8u, 4000.0f, 0.0f, 0.0f, 1020000.0f};

static const CAN_SIGNAL_TYPE_s cantx_testSignalMinimumBatteryVoltage = {63u, 8u, 4000.0f, 0.0f, 0.0f, 1020000.0f};

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

OS_QUEUE ftsk_imdCanDataQueue = NULL_PTR;

const CAN_SHIM_s can_kShim = {
    .pQueueImd             = &ftsk_imdCanDataQueue,
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
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/**
 * @brief   Testing CANTX_CalculateMaximumDischargeCurrent
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/1: Function prepares signal data as expected
 */
void testCANTX_CalculateMaximumDischargeCurrent(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateMaximumDischargeCurrent(NULL_PTR));

    /* ======= Routine tests =============================================== */
    float_t maximumDischargeCurrent = (float_t)can_kShim.pTableSof->recommendedContinuousPackDischargeCurrent_mA;
    /* ======= RT1/1: Test implementation */
    CAN_TxPrepareSignalData_Expect(&maximumDischargeCurrent, cantx_testSignalMaximumDischargeCurrent);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT1/1: Call function under test */
    uint64_t testResult = TEST_CANTX_CalculateMaximumDischargeCurrent(&can_kShim);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);
}

/**
 * @brief   Testing CANTX_CalculateMaximumChargeCurrent
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/1: Function prepares signal data as expected
 */
void testCANTX_CalculateMaximumChargeCurrent(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateMaximumChargeCurrent(NULL_PTR));

    /* ======= Routine tests =============================================== */
    float_t maximumChargeCurrent = (float_t)can_kShim.pTableSof->recommendedContinuousPackChargeCurrent_mA;
    /* ======= RT1/1: Test implementation */
    CAN_TxPrepareSignalData_Expect(&maximumChargeCurrent, cantx_testSignalMaximumChargeCurrent);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT1/1: Call function under test */
    uint64_t testResult = TEST_CANTX_CalculateMaximumChargeCurrent(&can_kShim);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);
}

/**
 * @brief   Testing CANTX_CalculateMaximumDischargePower
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/1: Function prepares signal data as expected
 */
void testCANTX_CalculateMaximumDischargePower(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateMaximumDischargePower(NULL_PTR));

    /* ======= Routine tests =============================================== */
    float_t maximumDischargePower =
        ((float_t)can_kShim.pTableSof->recommendedContinuousPackDischargeCurrent_mA / 1000.0f) *
        ((float_t)can_kShim.pTablePackValues->batteryVoltage_mV / 1000.0f);
    /* ======= RT1/1: Test implementation */
    CAN_TxPrepareSignalData_Expect(&maximumDischargePower, cantx_testSignalMaximumDischargePower);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT1/1: Call function under test */
    uint64_t testResult = TEST_CANTX_CalculateMaximumDischargePower(&can_kShim);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);
}

/**
 * @brief   Testing CANTX_CalculateMaximumChargePower
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/1: Function prepares signal data as expected
 */
void testCANTX_CalculateMaximumChargePower(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateMaximumChargePower(NULL_PTR));

    /* ======= Routine tests =============================================== */
    float_t maximumChargePower = ((float_t)can_kShim.pTableSof->recommendedContinuousPackChargeCurrent_mA / 1000.0f) *
                                 ((float_t)can_kShim.pTablePackValues->batteryVoltage_mV / 1000.0f);
    /* ======= RT1/1: Test implementation */
    CAN_TxPrepareSignalData_Expect(&maximumChargePower, cantx_testSignalMaximumChargePower);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT1/1: Call function under test */
    uint64_t testResult = TEST_CANTX_CalculateMaximumChargePower(&can_kShim);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);
}

/**
 * @brief   Testing CANTX_CalculateMaximumBatteryVoltage
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/1: Function prepares signal data as expected
 */
void testCANTX_CalculateMaximumBatteryVoltage(void) {
    /* ======= Routine tests =============================================== */
    float_t maximumBatteryVoltage = (float_t)(BS_NR_OF_CELL_BLOCKS_PER_STRING * BC_VOLTAGE_MAX_MSL_mV);
    /* ======= RT1/1: Test implementation */
    CAN_TxPrepareSignalData_Expect(&maximumBatteryVoltage, cantx_testSignalMaximumBatteryVoltage);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT1/1: Call function under test */
    uint64_t testResult = TEST_CANTX_CalculateMaximumBatteryVoltage();
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);
}

/**
 * @brief   Testing CANTX_CalculateMinimumBatteryVoltage
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/1: Function prepares signal data as expected
 */
void testCANTX_CalculateMinimumBatteryVoltage(void) {
    /* ======= Routine tests =============================================== */
    float_t minimumBatteryVoltage = (float_t)(BS_NR_OF_CELL_BLOCKS_PER_STRING * BC_VOLTAGE_MIN_MSL_mV);
    /* ======= RT1/1: Test implementation */
    CAN_TxPrepareSignalData_Expect(&minimumBatteryVoltage, cantx_testSignalMinimumBatteryVoltage);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT1/1: Call function under test */
    uint64_t testResult = TEST_CANTX_CalculateMinimumBatteryVoltage();
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);
}

/**
 * @brief   Testing CANTX_BuildPackLimitsMessage
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: NULL_PTR for pMessageData -> assert
 *            - AT2/2: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/1: Function prepares signal data as expected
 */
void testCANTX_BuildPackLimitsMessage(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_BuildPackLimitsMessage(NULL_PTR, &can_kShim));

    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_BuildPackLimitsMessage(&testMessageData[0u], NULL_PTR));

    /* ======= Routine tests =============================================== */
    float_t maximumDischargeCurrent = (float_t)can_kShim.pTableSof->recommendedContinuousPackDischargeCurrent_mA;
    float_t maximumChargeCurrent    = (float_t)can_kShim.pTableSof->recommendedContinuousPackChargeCurrent_mA;
    float_t maximumDischargePower =
        ((float_t)can_kShim.pTableSof->recommendedContinuousPackDischargeCurrent_mA / 1000.0f) *
        ((float_t)can_kShim.pTablePackValues->batteryVoltage_mV / 1000.0f);
    float_t maximumChargePower = ((float_t)can_kShim.pTableSof->recommendedContinuousPackChargeCurrent_mA / 1000.0f) *
                                 ((float_t)can_kShim.pTablePackValues->batteryVoltage_mV / 1000.0f);
    float_t minimumBatteryVoltage = (float_t)(BS_NR_OF_CELL_BLOCKS_PER_STRING * BC_VOLTAGE_MIN_MSL_mV);
    float_t maximumBatteryVoltage = (float_t)(BS_NR_OF_CELL_BLOCKS_PER_STRING * BC_VOLTAGE_MAX_MSL_mV);
    /* ======= RT1/1: Test implementation */
    CAN_TxPrepareSignalData_Expect(&maximumDischargeCurrent, cantx_testSignalMaximumDischargeCurrent);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 7u, 12u, testSignalData[1u], CANTX_PACK_LIMITS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxPrepareSignalData_Expect(&maximumChargeCurrent, cantx_testSignalMaximumChargeCurrent);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 11u, 12u, testSignalData[2u], CANTX_PACK_LIMITS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxPrepareSignalData_Expect(&maximumDischargePower, cantx_testSignalMaximumDischargePower);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 31u, 12u, testSignalData[3u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxPrepareSignalData_Expect(&maximumChargePower, cantx_testSignalMaximumChargePower);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[4u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 35u, 12u, testSignalData[4u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    CAN_TxPrepareSignalData_Expect(&minimumBatteryVoltage, cantx_testSignalMinimumBatteryVoltage);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[5u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[4u], 63u, 8u, testSignalData[5u], CANTX_PACK_LIMITS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);
    CAN_TxPrepareSignalData_Expect(&maximumBatteryVoltage, cantx_testSignalMaximumBatteryVoltage);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[6u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[5u], 55u, 8u, testSignalData[6u], CANTX_PACK_LIMITS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[6u]);

    /* ======= RT1/1: Call function under test */
    TEST_CANTX_BuildPackLimitsMessage(&testMessageData[0u], &can_kShim);
    TEST_ASSERT_EQUAL(testMessageData[6u], testMessageData[0u]);
    testMessageData[0u] = 0u;
}

/**
 * @brief   Testing CANTX_PackLimits
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/7: invalid message id -> assert
 *            - AT2/7: invalid message id type -> assert
 *            - AT3/7: invalid message dlc size -> assert
 *            - AT4/7: invalid message endianness -> assert
 *            - AT5/7: NULL_PTR for pCanData -> assert
 *            - AT6/7: NO NULL_PTR for pMuxId -> assert
 *            - AT7/7: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/1: Function prepares signal data as expected
 */
void testCANTX_PackLimits(void) {
    /* ======= Assertion tests ============================================= */
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = CANTX_PACK_LIMITS_ID,
        .idType     = CANTX_PACK_LIMITS_ID_TYPE,
        .dlc        = CAN_MAX_DLC,
        .endianness = CANTX_PACK_LIMITS_ENDIANNESS,
    };

    uint8_t testCanData[CAN_MAX_DLC] = {0u};
    uint8_t testMuxId                = 0u;
    /* ======= AT1/7 ======= */
    testMessage.id = CAN_MAX_11BIT_ID;
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackLimits(testMessage, testCanData, NULL_PTR, &can_kShim));
    testMessage.id = CANTX_PACK_LIMITS_ID;
    /* ======= AT2/7 ======= */
    testMessage.idType = CAN_EXTENDED_IDENTIFIER_29_BIT;
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackLimits(testMessage, testCanData, NULL_PTR, &can_kShim));
    testMessage.idType = CANTX_PACK_LIMITS_ID_TYPE;
    /* ======= AT3/7 ======= */
    testMessage.dlc = 9u;
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackLimits(testMessage, testCanData, NULL_PTR, &can_kShim));
    testMessage.dlc = CAN_MAX_DLC;
    /* ======= AT4/7 ======= */
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackLimits(testMessage, testCanData, NULL_PTR, &can_kShim));
    testMessage.endianness = CAN_BIG_ENDIAN;
    /* ======= AT5/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackLimits(testMessage, NULL_PTR, NULL_PTR, &can_kShim));
    /* ======= AT6/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackLimits(testMessage, testCanData, &testMuxId, &can_kShim));
    /* ======= AT7/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackLimits(testMessage, testCanData, NULL_PTR, NULL_PTR));

    /* ======= Routine tests =============================================== */
    float_t maximumDischargeCurrent = (float_t)can_kShim.pTableSof->recommendedContinuousPackDischargeCurrent_mA;
    float_t maximumChargeCurrent    = (float_t)can_kShim.pTableSof->recommendedContinuousPackChargeCurrent_mA;
    float_t maximumDischargePower =
        ((float_t)can_kShim.pTableSof->recommendedContinuousPackDischargeCurrent_mA / 1000.0f) *
        ((float_t)can_kShim.pTablePackValues->batteryVoltage_mV / 1000.0f);
    float_t maximumChargePower = ((float_t)can_kShim.pTableSof->recommendedContinuousPackChargeCurrent_mA / 1000.0f) *
                                 ((float_t)can_kShim.pTablePackValues->batteryVoltage_mV / 1000.0f);
    float_t minimumBatteryVoltage = (float_t)(BS_NR_OF_CELL_BLOCKS_PER_STRING * BC_VOLTAGE_MIN_MSL_mV);
    float_t maximumBatteryVoltage = (float_t)(BS_NR_OF_CELL_BLOCKS_PER_STRING * BC_VOLTAGE_MAX_MSL_mV);
    /* ======= RT1/1: Test implementation */
    CAN_TxPrepareSignalData_Ignore();
    DATA_Read2DataBlocks_ExpectAndReturn(can_kShim.pTableSof, can_kShim.pTablePackValues, STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 7u, 12u, (uint64_t)maximumDischargeCurrent, CANTX_PACK_LIMITS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 11u, 12u, (uint64_t)maximumChargeCurrent, CANTX_PACK_LIMITS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 12u, (uint64_t)maximumDischargePower, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 35u, 12u, (uint64_t)maximumChargePower, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 63u, 8u, (uint64_t)minimumBatteryVoltage, CANTX_PACK_LIMITS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 55u, 8u, (uint64_t)maximumBatteryVoltage, CANTX_PACK_LIMITS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanData, CAN_BIG_ENDIAN);
    /* ======= RT1/1: Call function under test */
    uint32_t testResult = CANTX_PackLimits(testMessage, testCanData, NULL_PTR, &can_kShim);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(0u, testResult);
}
