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
 * @file    test_can_cbs_tx_bms-state-details.c
 * @author  foxBMS Team
 * @date    2021-07-27 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the CAN driver callbacks
 * @details Tests BMS state details
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
#include "Mocksys_mon.h"

#include "database_cfg.h"

#include "can_cbs_tx_cyclic.h"
#include "can_cfg_tx-cyclic-message-definitions.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cbs_tx_bms-state-details.c")

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
const CAN_NODE_s can_node1 = {
    .canNodeRegister = canREG1,
};

const CAN_NODE_s can_node2Isolated = {
    .canNodeRegister = canREG2,
};

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
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void test_BmsStateDetails(void) {
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = CANTX_BMS_STATE_DETAILS_ID,
        .idType     = CANTX_BMS_STATE_DETAILS_ID_TYPE,
        .dlc        = CANTX_BMS_STATE_DETAILS_DLC,
        .endianness = CANTX_BMS_STATE_DETAILS_ENDIANNESS,
    };
    uint8_t testCanDataZeroArray[CANTX_BMS_STATE_DETAILS_DLC] = {0u};

    /* set local data table as needed */
    can_tableErrorState.taskEngineTimingViolationError    = true;
    can_tableErrorState.task1msTimingViolationError       = true;
    can_tableErrorState.task10msTimingViolationError      = true;
    can_tableErrorState.task100msTimingViolationError     = true;
    can_tableErrorState.task100msAlgoTimingViolationError = true;

    /* create timing violation response struct */
    SYSM_TIMING_VIOLATION_RESPONSE_s testRecordedTimingViolations = {
        .recordedViolationEngine    = true,
        .recordedViolation1ms       = true,
        .recordedViolation10ms      = true,
        .recordedViolation100ms     = true,
        .recordedViolation100msAlgo = true,
    };

    /* values of message data after each time information was added*/
    uint64_t testMessageData[11u] = {
        0x0000u,
        0x0001u,
        0x0003u,
        0x0007u,
        0x000Fu,
        0x001Fu,
        0x011Fu,
        0x031Fu,
        0x071Fu,
        0x0F1Fu,
        0x1F1Fu,
    };

    SYSM_TIMING_VIOLATION_RESPONSE_s testRecordedTimingViolationsZero = {0u};

    DATA_Read1DataBlock_ExpectAndReturn(can_kShim.pTableErrorState, STD_OK);
    SYSM_GetRecordedTimingViolations_Expect(&testRecordedTimingViolationsZero);
    SYSM_GetRecordedTimingViolations_ReturnThruPtr_pAnswer(&testRecordedTimingViolations);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 0u, 1u, 1u, CANTX_BMS_STATE_DETAILS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 1u, 1u, 1u, CANTX_BMS_STATE_DETAILS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 2u, 1u, 1u, CANTX_BMS_STATE_DETAILS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 3u, 1u, 1u, CANTX_BMS_STATE_DETAILS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[4u], 4u, 1u, 1u, CANTX_BMS_STATE_DETAILS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[5u], 8u, 1u, 1u, CANTX_BMS_STATE_DETAILS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[6u]);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[6u], 9u, 1u, 1u, CANTX_BMS_STATE_DETAILS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[7u]);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[7u], 10u, 1u, 1u, CANTX_BMS_STATE_DETAILS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[8u]);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[8u], 11u, 1u, 1u, CANTX_BMS_STATE_DETAILS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[9u]);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[9u], 12u, 1u, 1u, CANTX_BMS_STATE_DETAILS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[10u]);
    CAN_TxSetCanDataWithMessageData_Expect(
        testMessageData[10u], testCanDataZeroArray, CANTX_BMS_STATE_DETAILS_ENDIANNESS);

    CANTX_BmsStateDetails(testMessage, testCanDataZeroArray, NULL_PTR, &can_kShim);
}

void testSetTimingViolation(void) {
    uint64_t testMessageData[10u] = {0u};
    /* create timing violation response struct */
    SYSM_TIMING_VIOLATION_RESPONSE_s testRecordedTimingViolations = {
        .recordedViolationEngine    = true,
        .recordedViolation1ms       = true,
        .recordedViolation10ms      = true,
        .recordedViolation100ms     = true,
        .recordedViolation100msAlgo = true,
    };

    /* Test SetTimingViolationEngine */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 0u, 1u, 1u, CANTX_BMS_STATE_DETAILS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    TEST_CANTX_SetTimingViolationEngine(&testMessageData[0u], &can_kShim);

    /* Test SetTimingViolation1ms */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 1u, 1u, 1u, CANTX_BMS_STATE_DETAILS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    TEST_CANTX_SetTimingViolation1ms(&testMessageData[1u], &can_kShim);

    /* Test SetTimingViolation10Ms */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 2u, 1u, 1u, CANTX_BMS_STATE_DETAILS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    TEST_CANTX_SetTimingViolation10Ms(&testMessageData[2u], &can_kShim);

    /* Test SetTimingViolation100Ms */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 3u, 1u, 1u, CANTX_BMS_STATE_DETAILS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    TEST_CANTX_SetTimingViolation100Ms(&testMessageData[3u], &can_kShim);

    /* Test SetTimingViolation100MsAlgo */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[4u], 4u, 1u, 1u, CANTX_BMS_STATE_DETAILS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);
    TEST_CANTX_SetTimingViolation100MsAlgo(&testMessageData[4u], &can_kShim);

    /* Test SetTimingViolationEngineRec */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[5u], 8u, 1u, 1u, CANTX_BMS_STATE_DETAILS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[6u]);
    TEST_CANTX_SetTimingViolationEngineRec(&testMessageData[5u], &testRecordedTimingViolations);

    /* Test SetTimingViolation1MsRec */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[6u], 9u, 1u, 1u, CANTX_BMS_STATE_DETAILS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[7u]);
    TEST_CANTX_SetTimingViolation1MsRec(&testMessageData[6u], &testRecordedTimingViolations);

    /* Test SetTimingViolation10MsRec */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[7u], 10u, 1u, 1u, CANTX_BMS_STATE_DETAILS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[8u]);
    TEST_CANTX_SetTimingViolation10MsRec(&testMessageData[7u], &testRecordedTimingViolations);

    /* Test SetTimingViolation100MsRec */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[8u], 11u, 1u, 1u, CANTX_BMS_STATE_DETAILS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[9u]);
    TEST_CANTX_SetTimingViolation100MsRec(&testMessageData[8u], &testRecordedTimingViolations);

    /* Test SetTimingViolation100MsAlgoRec */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[9u], 12u, 1u, 1u, CANTX_BMS_STATE_DETAILS_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[10u]);
    TEST_CANTX_SetTimingViolation100MsAlgoRec(&testMessageData[9u], &testRecordedTimingViolations);
}
