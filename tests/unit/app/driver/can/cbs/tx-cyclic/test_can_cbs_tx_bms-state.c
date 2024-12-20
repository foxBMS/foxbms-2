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
 * @file    test_can_cbs_tx_bms-state.c
 * @author  foxBMS Team
 * @date    2021-07-27 (date of creation)
 * @updated 2024-12-20 (date of last update)
 * @version v1.8.0
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
#include "Mocksys_mon.h"

#include "database_cfg.h"

#include "can_cbs_tx_cyclic.h"
#include "can_cfg_tx-cyclic-message-definitions.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cbs_tx_bms-state.c")

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
uint64_t testMessageData[22u] = {0u};

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
static DATA_BLOCK_MSL_FLAG_s can_tableMslFlags                  = {.header.uniqueId = DATA_BLOCK_ID_MSL_FLAG};
static DATA_BLOCK_RSL_FLAG_s can_tableRslFlags                  = {.header.uniqueId = DATA_BLOCK_ID_RSL_FLAG};
static DATA_BLOCK_MOL_FLAG_s can_tableMolFlags                  = {.header.uniqueId = DATA_BLOCK_ID_MOL_FLAG};
static DATA_BLOCK_AEROSOL_SENSOR_s can_tableAerosolSensor       = {.header.uniqueId = DATA_BLOCK_ID_AEROSOL_SENSOR};
static DATA_BLOCK_BALANCING_CONTROL_s can_tableBalancingControl = {.header.uniqueId = DATA_BLOCK_ID_BALANCING_CONTROL};

OS_QUEUE imd_canDataQueue = NULL_PTR;

const CAN_SHIM_s can_kShim = {
    .pQueueImd              = &imd_canDataQueue,
    .pTableCellVoltage      = &can_tableCellVoltages,
    .pTableCellTemperature  = &can_tableTemperatures,
    .pTableMinMax           = &can_tableMinimumMaximumValues,
    .pTableCurrentSensor    = &can_tableCurrentSensor,
    .pTableOpenWire         = &can_tableOpenWire,
    .pTableStateRequest     = &can_tableStateRequest,
    .pTablePackValues       = &can_tablePackValues,
    .pTableSof              = &can_tableSof,
    .pTableSoc              = &can_tableSoc,
    .pTableSoe              = &can_tableSoe,
    .pTableErrorState       = &can_tableErrorState,
    .pTableInsulation       = &can_tableInsulation,
    .pTableMsl              = &can_tableMslFlags,
    .pTableRsl              = &can_tableRslFlags,
    .pTableMol              = &can_tableMolFlags,
    .pTableAerosolSensor    = &can_tableAerosolSensor,
    .pTableBalancingControl = &can_tableBalancingControl,
};

/* bms state with standard config only relevant entries differ */
static BMS_STATE_s bms_state = {
    .timer                             = 0,
    .stateRequest                      = BMS_STATE_NO_REQUEST,
    .state                             = BMS_STATEMACH_NORMAL,            /* changed */
    .substate                          = BMS_PRECHARGE_CLOSE_NEXT_STRING, /* changed */
    .lastState                         = BMS_STATEMACH_UNINITIALIZED,
    .lastSubstate                      = BMS_ENTRY,
    .triggerentry                      = 0u,
    .ErrRequestCounter                 = 0u,
    .initFinished                      = STD_NOT_OK,
    .counter                           = 0u,
    .OscillationTimeout                = 0u,
    .prechargeTryCounter               = 0u,
    .powerPath                         = BMS_POWER_PATH_OPEN,
    .closedStrings                     = {0u},
    .closedPrechargeContactors         = {0u},
    .numberOfClosedStrings             = 5u, /* changed */
    .deactivatedStrings                = {0},
    .firstClosedString                 = 0u,
    .stringOpenTimeout                 = 0u,
    .nextStringClosedTimer             = 0u,
    .stringCloseTimeout                = 0u,
    .nextState                         = BMS_STATEMACH_STANDBY,
    .restTimer_10ms                    = BS_RELAXATION_PERIOD_10ms,
    .currentFlowState                  = BMS_RELAXATION,
    .remainingDelay_ms                 = UINT32_MAX,
    .minimumActiveDelay_ms             = UINT32_MAX,
    .transitionToErrorState            = true, /* changed */
    .timeAboveContactorBreakCurrent_ms = 0u,
    .stringToBeOpened                  = 0u,
    .contactorToBeOpened               = CONT_UNDEFINED,
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    for (uint8_t i = 0; i < 22; i++) {
        testMessageData[i] = i;
    }
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/**
 * @brief   Testing CANTX_AnySysMonTimingIssueDetected
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/7: No flags set
 *            - RT2/7: recordedViolationAny flag set
 *            - RT3/7: taskEngineTimingViolationError set
 *            - RT4/7: task1msTimingViolationError set
 *            - RT5/7: task10msTimingViolationError set
 *            - RT6/7: task100msTimingViolationError set
 *            - RT7/7: task100msAlgoTimingViolationError set
 */
void testCANTX_AnySysMonTimingIssueDetected(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_AnySysMonTimingIssueDetected(NULL_PTR));

    /* ======= Routine tests =============================================== */
    SYSM_TIMING_VIOLATION_RESPONSE_s testRecordedTimingViolationsZero = {0u};
    SYSM_TIMING_VIOLATION_RESPONSE_s testRecordedTimingViolations     = {
            .recordedViolationAny       = false,
            .recordedViolationEngine    = false,
            .recordedViolation1ms       = false,
            .recordedViolation10ms      = false,
            .recordedViolation100ms     = false,
            .recordedViolation100msAlgo = false,
    };
    /* ======= RT1/7: Test implementation */
    SYSM_GetRecordedTimingViolations_Expect(&testRecordedTimingViolationsZero);
    SYSM_GetRecordedTimingViolations_ReturnThruPtr_pAnswer(&testRecordedTimingViolations);
    /* ======= RT1/7: Call function under test */
    bool testResult = TEST_CANTX_AnySysMonTimingIssueDetected(&can_kShim);
    /* ======= RT1/7: Test output verification */
    TEST_ASSERT_EQUAL(false, testResult);

    testRecordedTimingViolations.recordedViolationAny = true;
    /* ======= RT2/7: Test implementation */
    SYSM_GetRecordedTimingViolations_Expect(&testRecordedTimingViolationsZero);
    SYSM_GetRecordedTimingViolations_ReturnThruPtr_pAnswer(&testRecordedTimingViolations);
    /* ======= RT2/7: Call function under test */
    testResult = TEST_CANTX_AnySysMonTimingIssueDetected(&can_kShim);
    /* ======= RT2/7: Test output verification */
    TEST_ASSERT_EQUAL(true, testResult);
    testRecordedTimingViolations.recordedViolationAny = false;

    can_tableErrorState.taskEngineTimingViolationError = true;
    /* ======= RT3/7: Test implementation */
    SYSM_GetRecordedTimingViolations_Expect(&testRecordedTimingViolationsZero);
    SYSM_GetRecordedTimingViolations_ReturnThruPtr_pAnswer(&testRecordedTimingViolations);
    /* ======= RT3/7: Call function under test */
    testResult = TEST_CANTX_AnySysMonTimingIssueDetected(&can_kShim);
    /* ======= RT3/7: Test output verification */
    TEST_ASSERT_EQUAL(true, testResult);
    can_tableErrorState.taskEngineTimingViolationError = false;

    can_tableErrorState.task1msTimingViolationError = true;
    /* ======= RT4/7: Test implementation */
    SYSM_GetRecordedTimingViolations_Expect(&testRecordedTimingViolationsZero);
    SYSM_GetRecordedTimingViolations_ReturnThruPtr_pAnswer(&testRecordedTimingViolations);
    /* ======= RT4/7: Call function under test */
    testResult = TEST_CANTX_AnySysMonTimingIssueDetected(&can_kShim);
    /* ======= RT4/7: Test output verification */
    TEST_ASSERT_EQUAL(true, testResult);
    can_tableErrorState.task1msTimingViolationError = false;

    can_tableErrorState.task10msTimingViolationError = true;
    /* ======= RT5/7: Test implementation */
    SYSM_GetRecordedTimingViolations_Expect(&testRecordedTimingViolationsZero);
    SYSM_GetRecordedTimingViolations_ReturnThruPtr_pAnswer(&testRecordedTimingViolations);
    /* ======= RT5/7: Call function under test */
    testResult = TEST_CANTX_AnySysMonTimingIssueDetected(&can_kShim);
    /* ======= RT5/7: Test output verification */
    TEST_ASSERT_EQUAL(true, testResult);
    can_tableErrorState.task10msTimingViolationError = false;

    can_tableErrorState.task100msTimingViolationError = true;
    /* ======= RT6/7: Test implementation */
    SYSM_GetRecordedTimingViolations_Expect(&testRecordedTimingViolationsZero);
    SYSM_GetRecordedTimingViolations_ReturnThruPtr_pAnswer(&testRecordedTimingViolations);
    /* ======= RT6/7: Call function under test */
    testResult = TEST_CANTX_AnySysMonTimingIssueDetected(&can_kShim);
    /* ======= RT6/7: Test output verification */
    TEST_ASSERT_EQUAL(true, testResult);
    can_tableErrorState.task100msTimingViolationError = false;

    can_tableErrorState.task100msAlgoTimingViolationError = true;
    /* ======= RT7/7: Test implementation */
    SYSM_GetRecordedTimingViolations_Expect(&testRecordedTimingViolationsZero);
    SYSM_GetRecordedTimingViolations_ReturnThruPtr_pAnswer(&testRecordedTimingViolations);
    /* ======= RT7/7: Call function under test */
    testResult = TEST_CANTX_AnySysMonTimingIssueDetected(&can_kShim);
    /* ======= RT7/7: Test output verification */
    TEST_ASSERT_EQUAL(true, testResult);
    can_tableErrorState.task100msAlgoTimingViolationError = false;
}

/**
 * @brief   Testing CANTX_AnySysMonTimingIssueDetected
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: NULL_PTR for pMessageData -> assert
 *            - AT2/2: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/2: Function prepares signal data as expected, no precharge error
 *            - RT2/2: Function prepares signal data as expected, precharge errors
 */
void testCANTX_BuildBmsStateMessage(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_BuildBmsStateMessage(NULL_PTR, &can_kShim));

    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_BuildBmsStateMessage(&testMessageData[0u], NULL_PTR));

    /* ======= Routine tests =============================================== */
    can_tableInsulation.isImdRunning                         = true;
    can_tableErrorState.prechargeAbortedDueToVoltage[0u]     = false;
    can_tableErrorState.prechargeAbortedDueToCurrent[0u]     = false;
    can_tableErrorState.prechargeAbortedDueToVoltage[0u]     = false;
    can_tableErrorState.prechargeAbortedDueToCurrent[0u]     = false;
    can_tableErrorState.mcuDieTemperatureViolationError      = true;
    can_tableErrorState.mainFuseError                        = true;
    can_tableErrorState.interlockOpenedError                 = true;
    can_tableErrorState.criticalLowInsulationResistanceError = true;
    can_tableErrorState.stateRequestTimingViolationError     = true;
    can_tableErrorState.supplyVoltageClamp30cError           = true;
    can_tableMslFlags.packChargeOvercurrent                  = true;
    can_tableMslFlags.packDischargeOvercurrent               = true;
    can_tableErrorState.alertFlagSetError                    = true;
    can_tableErrorState.framReadCrcError                     = true;
    can_tableInsulation.insulationResistance_kOhm            = 1250u;
    can_tableBalancingControl.enableBalancing                = true;

    SYSM_TIMING_VIOLATION_RESPONSE_s testRecordedTimingViolationsZero = {0u};
    SYSM_TIMING_VIOLATION_RESPONSE_s testRecordedTimingViolations     = {
            .recordedViolationAny       = true,
            .recordedViolationEngine    = true,
            .recordedViolation1ms       = true,
            .recordedViolation10ms      = true,
            .recordedViolation100ms     = true,
            .recordedViolation100msAlgo = true,
    };

    uint64_t testResult = 0u;
    /* ======= RT1/2: Test implementation - everything okay */
    CAN_TxPrepareSignalData_Ignore();

    /* BMS State */
    BMS_GetState_ExpectAndReturn(bms_state.state);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 3u, 4u, bms_state.state, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);

    /* BMS Substate */
    BMS_GetSubstate_ExpectAndReturn(bms_state.substate);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 37u, 6u, bms_state.substate, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);

    /* Connected strings */
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(bms_state.numberOfClosedStrings);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 7u, 4u, 5u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);

    /* General/Fatal error  */
    DIAG_IsAnyFatalErrorSet_ExpectAndReturn(true);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 10u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);

    /* System Monitoring Error */
    BMS_IsTransitionToErrorStateActive_ExpectAndReturn(bms_state.transitionToErrorState);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[4u], 11u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);
    SYSM_GetRecordedTimingViolations_Expect(&testRecordedTimingViolationsZero);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    SYSM_GetRecordedTimingViolations_ReturnThruPtr_pAnswer(&testRecordedTimingViolations);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[5u], 12u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[6u]);

    /* Insulation monitoring active */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[6u], 13u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[7u]);

    /* Error: insulation */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[7u], 23u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[8u]);

    /* Insulation resistance */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[8u], 63u, 8u, 1250u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[9u]);

    /* Charging complete: TODO */
    /* Heater state: TODO */
    /* Cooling state: TODO */

    /* Error: Precharge voltage */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[9u], 16u, 1u, 0u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[10u]);

    /* Error: Precharge current */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[10u], 17u, 1u, 0u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[11u]);

    /* Error: MCU die temperature */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[11u], 18u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[12u]);

    /* Error: master overtemperature: TODO */
    /* Error: master undertemperature: TODO */

    /* Main fuse state */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[12u], 21u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[13u]);

    /* Error: interlock */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[13u], 22u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[14u]);

    /* Error: Can timing */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[14u], 24u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[15u]);

    /* Error: Overcurrent pack charge */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[15u], 25u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[16u]);

    /* Error: Overcurrent pack discharge */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[16u], 26u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[17u]);

    /* Error: Alert flag */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[17u], 27u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[18u]);

    /* Error: NVRAM CRC */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[18u], 28u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[19u]);

    /* Error: Clamp 30C */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[19u], 30u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[20u]);

    /* Balancing allowed flag */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[20u], 31u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[21u]);

    /* ======= RT1/2: Call function under test */
    TEST_CANTX_BuildBmsStateMessage(&testResult, &can_kShim);
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(21u, testResult);

    /* Reset local variables */
    testResult = 0u;

    /* Set precharge error to true */
    can_tableErrorState.prechargeAbortedDueToVoltage[0u] = true;
    can_tableErrorState.prechargeAbortedDueToCurrent[0u] = true;
    /* ======= RT2/2: Test implementation - Precharge errors set */
    CAN_TxPrepareSignalData_Ignore();

    /* BMS State */
    BMS_GetState_ExpectAndReturn(bms_state.state);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 3u, 4u, bms_state.state, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);

    /* BMS Substate */
    BMS_GetSubstate_ExpectAndReturn(bms_state.substate);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 37u, 6u, bms_state.substate, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);

    /* Connected strings */
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(bms_state.numberOfClosedStrings);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 7u, 4u, 5u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);

    /* General/Fatal error  */
    DIAG_IsAnyFatalErrorSet_ExpectAndReturn(true);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 10u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);

    /* System Monitoring Error */
    BMS_IsTransitionToErrorStateActive_ExpectAndReturn(bms_state.transitionToErrorState);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[4u], 11u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);
    SYSM_GetRecordedTimingViolations_Expect(&testRecordedTimingViolationsZero);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    SYSM_GetRecordedTimingViolations_ReturnThruPtr_pAnswer(&testRecordedTimingViolations);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[5u], 12u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[6u]);

    /* Insulation monitoring active */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[6u], 13u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[7u]);

    /* Error: insulation */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[7u], 23u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[8u]);

    /* Insulation resistance */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[8u], 63u, 8u, 1250u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[9u]);

    /* Charging complete: TODO */
    /* Heater state: TODO */
    /* Cooling state: TODO */

    /* Error: Precharge voltage */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[9u], 16u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[10u]);

    /* Error: Precharge current */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[10u], 17u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[11u]);

    /* Error: MCU die temperature */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[11u], 18u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[12u]);

    /* Error: master overtemperature: TODO */
    /* Error: master undertemperature: TODO */

    /* Main fuse state */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[12u], 21u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[13u]);

    /* Error: interlock */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[13u], 22u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[14u]);

    /* Error: Can timing */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[14u], 24u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[15u]);

    /* Error: Overcurrent pack charge */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[15u], 25u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[16u]);

    /* Error: Overcurrent pack discharge */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[16u], 26u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[17u]);

    /* Error: Alert flag */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[17u], 27u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[18u]);

    /* Error: NVRAM CRC */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[18u], 28u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[19u]);

    /* Error: Clamp 30C */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[19u], 30u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[20u]);

    /* Balancing allowed flag */
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[20u], 31u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[21u]);

    /* ======= RT1/2: Call function under test */
    TEST_CANTX_BuildBmsStateMessage(&testResult, &can_kShim);
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(21u, testResult);
}

/**
 * @brief   Testing CANTX_BmsState
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
 *            - RT1/1: can data is set as expected
 */
void testCANTX_BmsState(void) {
    /* ======= Assertion tests ============================================= */
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = 0x220,
        .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
        .dlc        = 8u,
        .endianness = CAN_BIG_ENDIAN,
    };

    uint8_t testCanData[CAN_MAX_DLC] = {0u};
    uint8_t testMuxId                = 0u;
    /* ======= AT1/7 ======= */
    testMessage.id = CAN_MAX_11BIT_ID;
    TEST_ASSERT_FAIL_ASSERT(CANTX_BmsState(testMessage, testCanData, NULL_PTR, &can_kShim));
    testMessage.id = 0x220;
    /* ======= AT2/7 ======= */
    testMessage.idType = CAN_EXTENDED_IDENTIFIER_29_BIT;
    TEST_ASSERT_FAIL_ASSERT(CANTX_BmsState(testMessage, testCanData, NULL_PTR, &can_kShim));
    testMessage.idType = CAN_STANDARD_IDENTIFIER_11_BIT;
    /* ======= AT3/7 ======= */
    testMessage.dlc = 9u;
    TEST_ASSERT_FAIL_ASSERT(CANTX_BmsState(testMessage, testCanData, NULL_PTR, &can_kShim));
    testMessage.dlc = 8u;
    /* ======= AT4/7 ======= */
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CANTX_BmsState(testMessage, testCanData, NULL_PTR, &can_kShim));
    testMessage.endianness = CAN_BIG_ENDIAN;
    /* ======= AT5/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_BmsState(testMessage, NULL_PTR, NULL_PTR, &can_kShim));
    /* ======= AT6/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_BmsState(testMessage, testCanData, &testMuxId, &can_kShim));
    /* ======= AT7/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_BmsState(testMessage, testCanData, NULL_PTR, NULL_PTR));

    /* ======= Routine tests =============================================== */
    can_tableInsulation.isImdRunning                         = true;
    can_tableErrorState.prechargeAbortedDueToVoltage[0u]     = false;
    can_tableErrorState.prechargeAbortedDueToCurrent[0u]     = false;
    can_tableErrorState.mcuDieTemperatureViolationError      = true;
    can_tableErrorState.mainFuseError                        = true;
    can_tableErrorState.interlockOpenedError                 = true;
    can_tableErrorState.criticalLowInsulationResistanceError = true;
    can_tableErrorState.stateRequestTimingViolationError     = true;
    can_tableMslFlags.packChargeOvercurrent                  = true;
    can_tableMslFlags.packDischargeOvercurrent               = true;
    can_tableErrorState.alertFlagSetError                    = true;
    can_tableErrorState.framReadCrcError                     = true;
    can_tableInsulation.insulationResistance_kOhm            = 1250u;

    SYSM_TIMING_VIOLATION_RESPONSE_s testRecordedTimingViolationsZero = {0u};
    SYSM_TIMING_VIOLATION_RESPONSE_s testRecordedTimingViolations     = {
            .recordedViolationAny       = true,
            .recordedViolationEngine    = true,
            .recordedViolation1ms       = true,
            .recordedViolation10ms      = true,
            .recordedViolation100ms     = true,
            .recordedViolation100msAlgo = true,
    };
    /* ======= RT1/1: Test implementation */
    CAN_TxPrepareSignalData_Ignore();
    DATA_Read4DataBlocks_ExpectAndReturn(
        can_kShim.pTableErrorState,
        can_kShim.pTableInsulation,
        can_kShim.pTableMsl,
        can_kShim.pTableBalancingControl,
        STD_OK);
    BMS_GetState_ExpectAndReturn(bms_state.state);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 3u, 4u, bms_state.state, CANTX_BMS_STATE_ENDIANNESS);
    BMS_GetSubstate_ExpectAndReturn(bms_state.substate);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 37u, 6u, bms_state.substate, CANTX_BMS_STATE_ENDIANNESS);
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(bms_state.numberOfClosedStrings);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 4u, 5u, CANTX_BMS_STATE_ENDIANNESS);
    DIAG_IsAnyFatalErrorSet_ExpectAndReturn(true);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 10u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    BMS_IsTransitionToErrorStateActive_ExpectAndReturn(bms_state.transitionToErrorState);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 11u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    SYSM_GetRecordedTimingViolations_Expect(&testRecordedTimingViolationsZero);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    SYSM_GetRecordedTimingViolations_ReturnThruPtr_pAnswer(&testRecordedTimingViolations);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 12u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 13u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 23u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 63u, 8u, 1250u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 16u, 1u, 0u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 17u, 1u, 0u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 18u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 21u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 22u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 24u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 25u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 26u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 27u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 28u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 30u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 31u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanData, CANTX_BMS_STATE_ENDIANNESS);
    /* ======= RT1/1: Call function under test */
    uint32_t testResult = CANTX_BmsState(testMessage, testCanData, NULL_PTR, &can_kShim);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(0u, testResult);
}

/**
 * @brief   Testing CANTX_TransmitBmsState
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - NONE
 *          - Routine validation:
 *            - RT1/2: message has been sent successfully
 *            - RT2/2: message has NOT been sent successfully
 */
void testCANTX_TransmitBmsState(void) {
    /* ======= Routine tests =============================================== */
    uint8_t testCanData[CAN_MAX_DLC] = {0u};

    can_tableInsulation.isImdRunning                         = true;
    can_tableErrorState.prechargeAbortedDueToVoltage[0u]     = false;
    can_tableErrorState.prechargeAbortedDueToCurrent[0u]     = false;
    can_tableErrorState.mcuDieTemperatureViolationError      = true;
    can_tableErrorState.mainFuseError                        = true;
    can_tableErrorState.interlockOpenedError                 = true;
    can_tableErrorState.criticalLowInsulationResistanceError = true;
    can_tableErrorState.stateRequestTimingViolationError     = true;
    can_tableMslFlags.packChargeOvercurrent                  = true;
    can_tableMslFlags.packDischargeOvercurrent               = true;
    can_tableErrorState.alertFlagSetError                    = true;
    can_tableErrorState.framReadCrcError                     = true;
    can_tableInsulation.insulationResistance_kOhm            = 1250u;

    SYSM_TIMING_VIOLATION_RESPONSE_s testRecordedTimingViolationsZero = {0u};
    SYSM_TIMING_VIOLATION_RESPONSE_s testRecordedTimingViolations     = {
            .recordedViolationAny       = true,
            .recordedViolationEngine    = true,
            .recordedViolation1ms       = true,
            .recordedViolation10ms      = true,
            .recordedViolation100ms     = true,
            .recordedViolation100msAlgo = true,
    };
    /* ======= RT1/2: Test implementation */
    CAN_TxPrepareSignalData_Ignore();
    DATA_Read4DataBlocks_ExpectAndReturn(
        can_kShim.pTableErrorState,
        can_kShim.pTableInsulation,
        can_kShim.pTableMsl,
        can_kShim.pTableBalancingControl,
        STD_OK);
    BMS_GetState_ExpectAndReturn(bms_state.state);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 3u, 4u, bms_state.state, CANTX_BMS_STATE_ENDIANNESS);
    BMS_GetSubstate_ExpectAndReturn(bms_state.substate);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 37u, 6u, bms_state.substate, CANTX_BMS_STATE_ENDIANNESS);
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(bms_state.numberOfClosedStrings);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 4u, 5u, CANTX_BMS_STATE_ENDIANNESS);
    DIAG_IsAnyFatalErrorSet_ExpectAndReturn(true);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 10u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    BMS_IsTransitionToErrorStateActive_ExpectAndReturn(bms_state.transitionToErrorState);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 11u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    SYSM_GetRecordedTimingViolations_Expect(&testRecordedTimingViolationsZero);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    SYSM_GetRecordedTimingViolations_ReturnThruPtr_pAnswer(&testRecordedTimingViolations);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 12u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 13u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 23u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 63u, 8u, 1250u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 16u, 1u, 0u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 17u, 1u, 0u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 18u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 21u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 22u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 24u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 25u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 26u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 27u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 28u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 30u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 31u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanData, CANTX_BMS_STATE_ENDIANNESS);
    CAN_DataSend_ExpectAndReturn(CAN_NODE_1, 0x220, CAN_STANDARD_IDENTIFIER_11_BIT, testCanData, STD_OK);
    /* ======= RT1/2: Call function under test */
    STD_RETURN_TYPE_e testResult = CANTX_TransmitBmsState();
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);

    /* ======= RT2/2: Test implementation */
    CAN_TxPrepareSignalData_Ignore();
    DATA_Read4DataBlocks_ExpectAndReturn(
        can_kShim.pTableErrorState,
        can_kShim.pTableInsulation,
        can_kShim.pTableMsl,
        can_kShim.pTableBalancingControl,
        STD_OK);
    BMS_GetState_ExpectAndReturn(bms_state.state);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 3u, 4u, bms_state.state, CANTX_BMS_STATE_ENDIANNESS);
    BMS_GetSubstate_ExpectAndReturn(bms_state.substate);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 37u, 6u, bms_state.substate, CANTX_BMS_STATE_ENDIANNESS);
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(bms_state.numberOfClosedStrings);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 4u, 5u, CANTX_BMS_STATE_ENDIANNESS);
    DIAG_IsAnyFatalErrorSet_ExpectAndReturn(true);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 10u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    BMS_IsTransitionToErrorStateActive_ExpectAndReturn(bms_state.transitionToErrorState);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 11u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    SYSM_GetRecordedTimingViolations_Expect(&testRecordedTimingViolationsZero);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    SYSM_GetRecordedTimingViolations_ReturnThruPtr_pAnswer(&testRecordedTimingViolations);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 12u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 13u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 23u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 63u, 8u, 1250u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 16u, 1u, 0u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 17u, 1u, 0u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 18u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 21u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 22u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 24u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 25u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 26u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 27u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 28u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 30u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(true, 1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 31u, 1u, 1u, CANTX_BMS_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanData, CANTX_BMS_STATE_ENDIANNESS);
    CAN_DataSend_ExpectAndReturn(CAN_NODE_1, 0x220, CAN_STANDARD_IDENTIFIER_11_BIT, testCanData, STD_NOT_OK);
    /* ======= RT2/2: Call function under test */
    testResult = CANTX_TransmitBmsState();
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);
}
