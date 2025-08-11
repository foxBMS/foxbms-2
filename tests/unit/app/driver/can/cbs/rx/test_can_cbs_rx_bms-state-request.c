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
 * @file    test_can_cbs_rx_bms-state-request.c
 * @author  foxBMS Team
 * @date    2021-07-28 (date of creation)
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
#include "Mockbal.h"
#include "Mockbal_cfg.h"
#include "Mockbms_cfg.h"
#include "Mockcan.h"
#include "Mockcan_helper.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockfoxmath.h"
#include "Mockimd.h"
#include "Mockos.h"
#include "Mocksys_mon.h"

#include "database_cfg.h"

#include "can_cbs_rx.h"
#include "can_cfg_rx-message-definitions.h"
#include "test_assert_helper.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cbs_rx_bms-state-request.c")

TEST_INCLUDE_PATH("../../src/app/application/bal")
TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/rx")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/fram")
TEST_INCLUDE_PATH("../../src/app/driver/imd")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/engine/sys_mon")
TEST_INCLUDE_PATH("../../src/app/task/config")

/*========== Definitions and Implementations for Unit Test ==================*/

#define CANRX_CAN_REQUEST_UPDATE_TIME_ms (3000u)

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

uint64_t testSignalData[5u] = {0u, 1u, 2u, 3u, 0x10000u};

uint64_t testMessageData = UINT64_MAX;

uint64_t testMessageDataZero = 0u;

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

uint8_t testCanData[CAN_MAX_DLC] = {0x01u, 0x23u, 0x45u, 0x67u, 0x89u, 0xABu, 0xCDu, 0xEFu};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    can_tableStateRequest.previousStateRequestViaCan = 0u;
    can_tableStateRequest.stateRequestViaCan         = 0u;
    can_tableStateRequest.stateRequestViaCanPending  = 0u;
    can_tableStateRequest.stateCounter               = 0u;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/**
 * @brief   Testing CANTX_BmsStateRequest
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/6: message id invalid -> assert
 *            - AT2/6: message id type invalid -> assert
 *            - AT3/6: message dlc invalid -> assert
 *            - AT4/6: message endianness invalid -> assert
 *            - AT5/6: NULL_PTR for kpkCanData -> assert
 *            - AT6/6: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/1: All functions are called successfully
 */
void testCANRX_BmsStateRequest(void) {
    /* ======= Assertion tests ============================================= */
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = CAN_MAX_11BIT_ID,
        .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
        .dlc        = 8u,
        .endianness = CAN_BIG_ENDIAN,
    };
    /* ======= AT1/6 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANRX_BmsStateRequest(testMessage, testCanData, &can_kShim));
    testMessage.id = CANRX_BMS_STATE_REQUEST_ID;
    /* ======= AT2/6 ======= */
    testMessage.idType = CAN_EXTENDED_IDENTIFIER_29_BIT;
    TEST_ASSERT_FAIL_ASSERT(CANRX_BmsStateRequest(testMessage, testCanData, &can_kShim));
    testMessage.idType = CAN_STANDARD_IDENTIFIER_11_BIT;
    /* ======= AT3/6 ======= */
    testMessage.dlc = 0u;
    TEST_ASSERT_FAIL_ASSERT(CANRX_BmsStateRequest(testMessage, testCanData, &can_kShim));
    testMessage.dlc = CAN_MAX_DLC;
    /* ======= AT4/6 ======= */
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CANRX_BmsStateRequest(testMessage, testCanData, &can_kShim));
    testMessage.endianness = CAN_BIG_ENDIAN;
    /* ======= AT5/6 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANRX_BmsStateRequest(testMessage, NULL_PTR, &can_kShim));
    /* ======= AT6/6 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANRX_BmsStateRequest(testMessage, testCanData, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData, 1u, 2u, &testSignalData[0u], CANRX_BMS_STATE_REQUEST_ENDIANNESS);
    DATA_Read1DataBlock_ExpectAndReturn(can_kShim.pTableStateRequest, STD_OK);
    CAN_RxGetMessageDataFromCanData_Expect(&testMessageDataZero, testCanData, CANRX_BMS_STATE_REQUEST_ENDIANNESS);
    CAN_RxGetMessageDataFromCanData_ReturnThruPtr_pMessage(&testMessageData);
    CAN_RxGetSignalDataFromMessageData(
        testMessageData, 1u, 2u, &testSignalData[0u], CANRX_BMS_STATE_REQUEST_ENDIANNESS);
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData, 1u, 2u, &testSignalData[0u], CANRX_BMS_STATE_REQUEST_ENDIANNESS);
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData, 2u, 1u, &testSignalData[0u], CANRX_BMS_STATE_REQUEST_ENDIANNESS);
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData, 8u, 1u, &testSignalData[0u], CANRX_BMS_STATE_REQUEST_ENDIANNESS);

    BAL_GetInitializationState_ExpectAndReturn(STD_OK);
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_GLOBAL_DISABLE_REQUEST, BAL_OK);
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData, 23u, 8u, &testSignalData[0u], CANRX_BMS_STATE_REQUEST_ENDIANNESS);
    BAL_SetBalancingThreshold_Expect((uint32_t)testMessageDataZero);
    DATA_Write1DataBlock_ExpectAndReturn(can_kShim.pTableStateRequest, STD_OK);

    /* ======= RT1/1: Call function under test */
    uint32_t testResult = CANRX_BmsStateRequest(testMessage, testCanData, &can_kShim);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(0u, testResult);
}

/**
 * @brief   Testing CANRX_ClearAllPersistentFlags
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/2: No clearing of flags requested
 *            - RT2/2: Clearing of flags requested 1 String
 */
void testCANRX_ClearAllPersistentFlags(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData, 2u, 1u, &testSignalData[0u], CANRX_BMS_STATE_REQUEST_ENDIANNESS);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&testSignalData[0u]);
    /* ======= RT1/2: Call function under test */
    TEST_CANRX_ClearAllPersistentFlags(testMessageData);

    /* ======= RT2/2: Test implementation */
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData, 2u, 1u, &testSignalData[0u], CANRX_BMS_STATE_REQUEST_ENDIANNESS);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&testSignalData[1u]);
    DIAG_Handler_ExpectAndReturn(
        DIAG_ID_DEEP_DISCHARGE_DETECTED, DIAG_EVENT_OK, DIAG_STRING, 0u, DIAG_HANDLER_RETURN_OK);
    SYSM_ClearAllTimingViolations_Expect();
    /* ======= RT2/2: Call function under test */
    TEST_CANRX_ClearAllPersistentFlags(testMessageData);
}

/**
 * @brief   Testing CANRX_HandleModeRequest
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/7: standby requested
 *            - RT2/7: discharge requested
 *            - RT3/7: charge requested
 *            - RT4/7: invalid request
 *            - RT5/7: state counter overflow
 *            - RT6/7: same state as previous; OS_CheckTimeHasPassed returns true
 *            - RT7/7: same state as previous; OS_CheckTimeHasPassed returns false
 */
void testCANRX_HandleModeRequest(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_HandleModeRequest(testMessageData, NULL_PTR));
    /* ======= Routine tests =============================================== */
    can_tableStateRequest.previousStateRequestViaCan = BMS_REQ_ID_NOREQ;
    can_tableStateRequest.stateCounter               = 0u;
    /* ======= RT1/7: Test implementation */
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData, 1u, 2u, &testSignalData[0u], CANRX_BMS_STATE_REQUEST_ENDIANNESS);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&testSignalData[0u]);
    /* ======= RT1/7: Call function under test */
    TEST_CANRX_HandleModeRequest(testMessageData, &can_kShim);
    /* ======= RT1/7: Test output verification */
    TEST_ASSERT_EQUAL(BMS_REQ_ID_NOREQ, can_tableStateRequest.previousStateRequestViaCan);
    TEST_ASSERT_EQUAL(BMS_REQ_ID_STANDBY, can_tableStateRequest.stateRequestViaCan);
    TEST_ASSERT_EQUAL(BMS_REQ_ID_STANDBY, can_tableStateRequest.stateRequestViaCanPending);
    TEST_ASSERT_EQUAL(1u, can_tableStateRequest.stateCounter);

    /* ======= RT2/7: Test implementation */
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData, 1u, 2u, &testSignalData[0u], CANRX_BMS_STATE_REQUEST_ENDIANNESS);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&testSignalData[1u]);
    /* ======= RT2/7: Call function under test */
    TEST_CANRX_HandleModeRequest(testMessageData, &can_kShim);
    /* ======= RT2/7: Test output verification */
    TEST_ASSERT_EQUAL(BMS_REQ_ID_STANDBY, can_tableStateRequest.previousStateRequestViaCan);
    TEST_ASSERT_EQUAL(BMS_REQ_ID_NORMAL, can_tableStateRequest.stateRequestViaCan);
    TEST_ASSERT_EQUAL(BMS_REQ_ID_NORMAL, can_tableStateRequest.stateRequestViaCanPending);
    TEST_ASSERT_EQUAL(2u, can_tableStateRequest.stateCounter);

    /* ======= RT3/7: Test implementation */
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData, 1u, 2u, &testSignalData[0u], CANRX_BMS_STATE_REQUEST_ENDIANNESS);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&testSignalData[2u]);
    /* ======= RT3/7: Call function under test */
    TEST_CANRX_HandleModeRequest(testMessageData, &can_kShim);
    /* ======= RT3/7: Test output verification */
    TEST_ASSERT_EQUAL(BMS_REQ_ID_NORMAL, can_tableStateRequest.previousStateRequestViaCan);
    TEST_ASSERT_EQUAL(BMS_REQ_ID_CHARGE, can_tableStateRequest.stateRequestViaCan);
    TEST_ASSERT_EQUAL(BMS_REQ_ID_CHARGE, can_tableStateRequest.stateRequestViaCanPending);
    TEST_ASSERT_EQUAL(3u, can_tableStateRequest.stateCounter);

    /* ======= RT4/7: Test implementation */
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData, 1u, 2u, &testSignalData[0u], CANRX_BMS_STATE_REQUEST_ENDIANNESS);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&testSignalData[3u]);
    /* ======= RT4/7: Call function under test */
    TEST_CANRX_HandleModeRequest(testMessageData, &can_kShim);
    /* ======= RT4/7: Test output verification */
    TEST_ASSERT_EQUAL(BMS_REQ_ID_CHARGE, can_tableStateRequest.previousStateRequestViaCan);
    TEST_ASSERT_EQUAL(BMS_REQ_ID_NOREQ, can_tableStateRequest.stateRequestViaCan);
    TEST_ASSERT_EQUAL(BMS_REQ_ID_NOREQ, can_tableStateRequest.stateRequestViaCanPending);
    TEST_ASSERT_EQUAL(4u, can_tableStateRequest.stateCounter);

    /* ======= RT5/7: Test implementation */
    can_tableStateRequest.stateCounter = UINT8_MAX;
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData, 1u, 2u, &testSignalData[0u], CANRX_BMS_STATE_REQUEST_ENDIANNESS);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&testSignalData[3u]);
    OS_CheckTimeHasPassed_ExpectAndReturn(can_tableStateRequest.header.timestamp, 3000u, false);
    /* ======= RT5/7: Call function under test */
    TEST_CANRX_HandleModeRequest(testMessageData, &can_kShim);
    /* ======= RT5/7: Test output verification */
    TEST_ASSERT_EQUAL(BMS_REQ_ID_NOREQ, can_tableStateRequest.previousStateRequestViaCan);
    TEST_ASSERT_EQUAL(BMS_REQ_ID_NOREQ, can_tableStateRequest.stateRequestViaCan);
    TEST_ASSERT_EQUAL(BMS_REQ_ID_NOREQ, can_tableStateRequest.stateRequestViaCanPending);
    TEST_ASSERT_EQUAL(0u, can_tableStateRequest.stateCounter);

    /* ======= RT6/7: Test implementation */
    can_tableStateRequest.stateRequestViaCan = BMS_REQ_ID_STANDBY;
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData, 1u, 2u, &testSignalData[0u], CANRX_BMS_STATE_REQUEST_ENDIANNESS);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&testSignalData[0u]);
    OS_CheckTimeHasPassed_ExpectAndReturn(can_tableStateRequest.header.timestamp, 3000u, true);
    /* ======= RT6/7: Call function under test */
    TEST_CANRX_HandleModeRequest(testMessageData, &can_kShim);
    /* ======= RT6/7: Test output verification */
    TEST_ASSERT_EQUAL(BMS_REQ_ID_STANDBY, can_tableStateRequest.previousStateRequestViaCan);
    TEST_ASSERT_EQUAL(BMS_REQ_ID_STANDBY, can_tableStateRequest.stateRequestViaCan);
    TEST_ASSERT_EQUAL(BMS_REQ_ID_STANDBY, can_tableStateRequest.stateRequestViaCanPending);
    TEST_ASSERT_EQUAL(1u, can_tableStateRequest.stateCounter);

    /* ======= RT7/7: Test implementation */
    can_tableStateRequest.stateRequestViaCan = BMS_REQ_ID_NORMAL;
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData, 1u, 2u, &testSignalData[0u], CANRX_BMS_STATE_REQUEST_ENDIANNESS);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&testSignalData[1u]);
    OS_CheckTimeHasPassed_ExpectAndReturn(can_tableStateRequest.header.timestamp, 3000u, false);
    /* ======= RT7/7: Call function under test */
    TEST_CANRX_HandleModeRequest(testMessageData, &can_kShim);
    /* ======= RT7/7: Test output verification */
    TEST_ASSERT_EQUAL(BMS_REQ_ID_NORMAL, can_tableStateRequest.previousStateRequestViaCan);
    TEST_ASSERT_EQUAL(BMS_REQ_ID_NORMAL, can_tableStateRequest.stateRequestViaCan);
    TEST_ASSERT_EQUAL(BMS_REQ_ID_STANDBY, can_tableStateRequest.stateRequestViaCanPending);
    TEST_ASSERT_EQUAL(2u, can_tableStateRequest.stateCounter);
}

/**
 * @brief   Testing CANRX_HandleBalancingRequest
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/3: Disable balancing
 *            - RT2/3: Enable balancing
 *            - RT3/3: Balancing not initialized
 */
void testCANRX_HandleBalancingRequest(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    /* ======= Routine tests =============================================== */
    /* ======= RT1/3: Test implementation */
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData, 8u, 1u, &testSignalData[0u], CANRX_BMS_STATE_REQUEST_ENDIANNESS);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&testSignalData[0u]);
    BAL_GetInitializationState_ExpectAndReturn(STD_OK);
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_GLOBAL_DISABLE_REQUEST, BAL_OK);
    /* ======= RT1/3: Call function under test */
    TEST_CANRX_HandleBalancingRequest(testMessageData);

    /* ======= RT2/3: Test implementation */
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData, 8u, 1u, &testSignalData[0u], CANRX_BMS_STATE_REQUEST_ENDIANNESS);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&testSignalData[1u]);
    BAL_GetInitializationState_ExpectAndReturn(STD_OK);
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_GLOBAL_ENABLE_REQUEST, BAL_OK);
    /* ======= RT2/3: Call function under test */
    TEST_CANRX_HandleBalancingRequest(testMessageData);

    /* ======= RT3/3: Test implementation */
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData, 8u, 1u, &testSignalData[0u], CANRX_BMS_STATE_REQUEST_ENDIANNESS);
    BAL_GetInitializationState_ExpectAndReturn(STD_NOT_OK);
    /* ======= RT3/3: Call function under test */
    TEST_CANRX_HandleBalancingRequest(testMessageData);
}

/**
 * @brief   Testing
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/2: Function reads and handles data correctly
 *            - RT2/2: Requested threshold higher than maximum value
 */
void testCANRX_SetBalancingThreshold(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData, 23u, 8u, &testSignalData[0u], CANRX_BMS_STATE_REQUEST_ENDIANNESS);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&testSignalData[1u]);
    BAL_SetBalancingThreshold_Expect(testSignalData[1u]);
    /* ======= RT1/2: Call function under test */
    TEST_CANRX_SetBalancingThreshold(testMessageData);

    /* ======= RT2/2: Test implementation */
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData, 23u, 8u, &testSignalData[0u], CANRX_BMS_STATE_REQUEST_ENDIANNESS);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&testSignalData[4u]);
    BAL_SetBalancingThreshold_Expect(UINT16_MAX);
    /* ======= RT2/2: Call function under test */
    TEST_CANRX_SetBalancingThreshold(testMessageData);
}
