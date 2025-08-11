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
 * @file    test_sys.c
 * @author  foxBMS Team
 * @date    2020-04-02 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the sys module
 * @details Test functions:
 *          - testSYS_GeneralMacroBist
 *          - testSYS_Trigger
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockafe.h"
#include "Mockalgorithm.h"
#include "Mockbal.h"
#include "Mockbms.h"
#include "Mockcan.h"
#include "Mockcontactor.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockfassert.h"
#include "Mockfram.h"
#include "Mockimd.h"
#include "Mockinterlock.h"
#include "Mockmeas.h"
#include "Mockos.h"
#include "Mockrtc.h"
#include "Mocksbc.h"
#include "Mocksof_trapezoid.h"
#include "Mockstate_estimation.h"
#include "Mocksys_cfg.h"

#include "fram_cfg.h"

#include "sys.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("sys.c")

TEST_INCLUDE_PATH("../../src/app/application/algorithm")
TEST_INCLUDE_PATH("../../src/app/application/algorithm/config")
TEST_INCLUDE_PATH("../../src/app/application/algorithm/state_estimation")
TEST_INCLUDE_PATH("../../src/app/application/algorithm/state_estimation/sof/trapezoid")
TEST_INCLUDE_PATH("../../src/app/application/bal")
TEST_INCLUDE_PATH("../../src/app/application/bms")
TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/contactor")
TEST_INCLUDE_PATH("../../src/app/driver/fram")
TEST_INCLUDE_PATH("../../src/app/driver/imd")
TEST_INCLUDE_PATH("../../src/app/driver/interlock")
TEST_INCLUDE_PATH("../../src/app/driver/meas")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/driver/sbc")
TEST_INCLUDE_PATH("../../src/app/driver/sbc/fs8x_driver")
TEST_INCLUDE_PATH("../../src/app/driver/spi")
TEST_INCLUDE_PATH("../../src/app/driver/sps")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/engine/sys")
TEST_INCLUDE_PATH("../../src/app/task/config")

/*========== Definitions and Implementations for Unit Test ==================*/
SBC_STATE_s sbc_stateMcuSupervisor;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testSYS_RunStateMachine(void) {
    SYS_STATE_s sys_state  = {0};
    sys_state.currentState = SYS_FSM_STATE_UNINITIALIZED;

    /* Test State Machine naturally, without special input */
    /* state SYS_STATEMACH_UNINITIALIZED */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SYS_SetStateRequest(SYS_STATE_INITIALIZATION_REQUEST);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_SYS_RunStateMachine(&sys_state);

    /* state SYS_FSM_STATE_INITIALIZATION, substate SYS_FSM_SUBSTATE_ENTRY */
    TEST_SYS_RunStateMachine(&sys_state);

    /* substate SYS_FSM_CHECK_DEEP_DISCHARGE */
    FRAM_ReadData_ExpectAndReturn(FRAM_BLOCK_ID_DEEP_DISCHARGE_FLAG, STD_OK);
    TEST_SYS_RunStateMachine(&sys_state);

    /* state SYS_FSM_STATE_INITIALIZATION, substate SYS_FSM_SUBSTATE_START_INITIALIZATION_SBC */
    SBC_SetStateRequest_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_state);
    /* substate SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_SBC */
    SBC_GetState_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATEMACHINE_RUNNING);
    TEST_SYS_RunStateMachine(&sys_state);

    /* substate SYS_FSM_SUBSTATE_INITIALIZATION_CAN */
    CAN_Initialize_Expect();
    TEST_SYS_RunStateMachine(&sys_state);

    /* substate SYS_FSM_SUBSTATE_INITIALIZE_RTC */
    RTC_IsRtcModuleInitialized_ExpectAndReturn(false);
    TEST_SYS_RunStateMachine(&sys_state);
    RTC_IsRtcModuleInitialized_ExpectAndReturn(true);
    TEST_SYS_RunStateMachine(&sys_state);

    /* substate SYS_FSM_SUBSTATE_START_UP_BIST */
    DATA_ExecuteDataBist_Expect();
    TEST_SYS_RunStateMachine(&sys_state);

    /* substate SYS_FSM_SUBSTATE_SEND_BOOT_MESSAGE */
    SYS_SendBootMessage_Expect();
    TEST_SYS_RunStateMachine(&sys_state);

    /* state SYS_FSM_STATE_PRE_RUNNING substate SYS_FSM_SUBSTATE_ENTRY */
    TEST_SYS_RunStateMachine(&sys_state);

    /* substate SYS_FSM_SUBSTATE_INITIALIZE_INTERLOCK*/
    ILCK_SetStateRequest_ExpectAndReturn(ILCK_STATE_INITIALIZATION_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_state);

    /* substate SYS_FSM_SUBSTATE_START_INITIALIZATION_BAL */
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_state);
    /* substate SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BAL */
    BAL_GetInitializationState_ExpectAndReturn(STD_OK);
    TEST_SYS_RunStateMachine(&sys_state);
    /* substate SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BAL_GLOBAL_ENABLE */
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_GLOBAL_DISABLE_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_state);

    /* substate SYS_FSM_SUBSTATE_START_FIRST_MEASUREMENT_CYCLE */
    MEAS_StartMeasurement_ExpectAndReturn(STD_OK);
    TEST_SYS_RunStateMachine(&sys_state);
    /* substate SYS_FSM_SUBSTATE_WAIT_FIRST_MEASUREMENT_CYCLE */
    MEAS_IsFirstMeasurementCycleFinished_ExpectAndReturn(true);
    ALGO_UnlockInitialization_Expect();
    TEST_SYS_RunStateMachine(&sys_state);

    /* substate SYS_FSM_SUBSTATE_INITIALIZATION_MISC */
    CAN_EnablePeriodic_Expect(true);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        SE_InitializeSoc_Expect(false, s);
        SE_InitializeSoe_Expect(false, s);
        SE_InitializeSoh_Expect(s);
    }
    TEST_SYS_RunStateMachine(&sys_state);

    /* substate SYS_FSM_SUBSTATE_INITIALIZATION_IMD */
    IMD_RequestInitialization_ExpectAndReturn(IMD_REQUEST_OK);
    TEST_SYS_RunStateMachine(&sys_state);

    /* substate SYS_FSM_SUBSTATE_START_INITIALIZATION_BMS */
    BMS_SetStateRequest_ExpectAndReturn(BMS_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_state);

    /* substate SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BMS */
    BMS_GetInitializationState_ExpectAndReturn(STD_OK);
    TEST_SYS_RunStateMachine(&sys_state);

    /* state SYS_FSM_STATE_RUNNING */
    TEST_SYS_RunStateMachine(&sys_state);
}

void testSYS_RunStateMachineErrorState(void) {
    /* ======= Routine tests =============================================== */
    SYS_STATE_s sys_stateSbcError  = {0};
    sys_stateSbcError.currentState = SYS_FSM_STATE_UNINITIALIZED;

    /* ======= RT1/7: Test implementation: Error in SBC Initialization */
    /* state SYS_FSM_STATE_UNINITIALIZED */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SYS_SetStateRequest(SYS_STATE_INITIALIZATION_REQUEST);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_SYS_RunStateMachine(&sys_stateSbcError);

    /* state SYS_FSM_STATE_INITIALIZATION, substate SYS_ENTRY */
    TEST_SYS_RunStateMachine(&sys_stateSbcError);

    /* substate SYS_FSM_CHECK_DEEP_DISCHARGE */
    FRAM_ReadData_ExpectAndReturn(FRAM_BLOCK_ID_DEEP_DISCHARGE_FLAG, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateSbcError);

    /* substate SYS_ENTRY */
    SBC_SetStateRequest_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateSbcError);
    /* substate SYS_FSM_SUBSTATE_START_INITIALIZATION_SBC */
    for (uint16_t i = 0; i < (SYS_STATE_MACHINE_SBC_INIT_TIMEOUT_MS / SYS_TASK_CYCLE_CONTEXT_MS) + 2; i++) {
        SBC_GetState_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATEMACHINE_ERROR);
        TEST_SYS_RunStateMachine(&sys_stateSbcError);
    }
    /* state SYS_STATEMACH_ERROR */
    TEST_SYS_RunStateMachine(&sys_stateSbcError);

    /* ======= RT2/7: Test implementation: Error in Balancing Initialization */
    SYS_STATE_s sys_stateBalancingError  = {0};
    sys_stateBalancingError.currentState = SYS_FSM_STATE_UNINITIALIZED;
    /* state SYS_FSM_STATE_UNINITIALIZED */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SYS_SetStateRequest(SYS_STATE_INITIALIZATION_REQUEST);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_SYS_RunStateMachine(&sys_stateBalancingError);

    /* state SYS_FSM_STATE_INITIALIZATION */
    TEST_SYS_RunStateMachine(&sys_stateBalancingError);

    /* substate SYS_FSM_CHECK_DEEP_DISCHARGE */
    FRAM_ReadData_ExpectAndReturn(FRAM_BLOCK_ID_DEEP_DISCHARGE_FLAG, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateBalancingError);

    /* substate SYS_FSM_SUBSTATE_START_INITIALIZATION_SBC */
    SBC_SetStateRequest_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateBalancingError);
    /* substate SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_SBC */
    SBC_GetState_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATEMACHINE_RUNNING);
    TEST_SYS_RunStateMachine(&sys_stateBalancingError);

    /* substate SYS_FSM_SUBSTATE_INITIALIZATION_CAN */
    CAN_Initialize_Expect();
    TEST_SYS_RunStateMachine(&sys_stateBalancingError);

    /* substate SYS_FSM_SUBSTATE_INITIALIZE_RTC */
    RTC_IsRtcModuleInitialized_ExpectAndReturn(true);
    TEST_SYS_RunStateMachine(&sys_stateBalancingError);

    /* substate SYS_FSM_SUBSTATE_SYSTEM_BIST */
    DATA_ExecuteDataBist_Expect();
    TEST_SYS_RunStateMachine(&sys_stateBalancingError);

    /* substate SYS_FSM_SUBSTATE_SEND_BOOT_MESSAGE */
    SYS_SendBootMessage_Expect();
    TEST_SYS_RunStateMachine(&sys_stateBalancingError);

    /* state SYS_FSM_STATE_PRE_RUNNING */
    ILCK_SetStateRequest_ExpectAndReturn(ILCK_STATE_INITIALIZATION_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateBalancingError);

    /* substate SYS_FSM_SUBSTATE_START_INITIALIZATION_BAL */
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateBalancingError);
    /* substate SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BAL */
    for (uint16_t i = 0; i < (SYS_STATE_MACHINE_BAL_INITIALIZATION_TIMEOUT_MS / SYS_TASK_CYCLE_CONTEXT_MS) + 2; i++) {
        BAL_GetInitializationState_ExpectAndReturn(STD_NOT_OK);
        TEST_SYS_RunStateMachine(&sys_stateBalancingError);
    }
    /* state SYS_FSM_STATE_ERROR */
    TEST_SYS_RunStateMachine(&sys_stateBalancingError);

    /* ======= RT3/7: Test implementation: Error in Global Balancing Initialization */
    SYS_STATE_s sys_stateGlobalBalancingError  = {0};
    sys_stateGlobalBalancingError.currentState = SYS_FSM_STATE_UNINITIALIZED;
    /* state SYS_FSM_STATE_UNINITIALIZED */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SYS_SetStateRequest(SYS_STATE_INITIALIZATION_REQUEST);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_SYS_RunStateMachine(&sys_stateGlobalBalancingError);

    /* state SYS_FSM_STATE_INITIALIZATION */
    TEST_SYS_RunStateMachine(&sys_stateGlobalBalancingError);

    /* substate SYS_FSM_CHECK_DEEP_DISCHARGE */
    FRAM_ReadData_ExpectAndReturn(FRAM_BLOCK_ID_DEEP_DISCHARGE_FLAG, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateGlobalBalancingError);

    /* substate SYS_FSM_SUBSTATE_START_INITIALIZATION_SBC */
    SBC_SetStateRequest_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateGlobalBalancingError);
    /* substate SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_SBC */
    SBC_GetState_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATEMACHINE_RUNNING);
    TEST_SYS_RunStateMachine(&sys_stateGlobalBalancingError);

    /* substate SYS_FSM_SUBSTATE_INITIALIZATION_CAN */
    CAN_Initialize_Expect();
    TEST_SYS_RunStateMachine(&sys_stateGlobalBalancingError);

    /* substate SYS_FSM_SUBSTATE_INITIALIZE_RTC */
    RTC_IsRtcModuleInitialized_ExpectAndReturn(true);
    TEST_SYS_RunStateMachine(&sys_stateGlobalBalancingError);

    /* substate SYS_FSM_SUBSTATE_SYSTEM_BIST */
    DATA_ExecuteDataBist_Expect();
    TEST_SYS_RunStateMachine(&sys_stateGlobalBalancingError);

    /* substate SYS_FSM_SUBSTATE_SEND_BOOT_MESSAGE */
    SYS_SendBootMessage_Expect();
    TEST_SYS_RunStateMachine(&sys_stateGlobalBalancingError);

    /* state SYS_FSM_STATE_PRE_RUNNING */
    ILCK_SetStateRequest_ExpectAndReturn(ILCK_STATE_INITIALIZATION_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateGlobalBalancingError);

    /* substate SYS_FSM_SUBSTATE_START_INITIALIZATION_BAL */
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateGlobalBalancingError);
    /* substate SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BAL */
    BAL_GetInitializationState_ExpectAndReturn(STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateGlobalBalancingError);
    /* substate SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BAL_GLOBAL_ENABLE */
    for (uint16_t i = 0; i < (SYS_STATE_MACHINE_INITIALIZATION_TIMEOUT_MS / SYS_TASK_CYCLE_CONTEXT_MS) + 2; i++) {
        BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_GLOBAL_DISABLE_REQUEST, STD_NOT_OK);
        TEST_SYS_RunStateMachine(&sys_stateGlobalBalancingError);
    }
    /* state SYS_FSM_STATE_ERROR */
    TEST_SYS_RunStateMachine(&sys_stateGlobalBalancingError);

    /* ======= RT4/7: Test implementation: Error in Balancing */
    SYS_STATE_s sys_stateFirstMeasurementError  = {0};
    sys_stateFirstMeasurementError.currentState = SYS_FSM_STATE_UNINITIALIZED;
    /* state SYS_FSM_STATE_UNINITIALIZED */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SYS_SetStateRequest(SYS_STATE_INITIALIZATION_REQUEST);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);

    /* state SYS_FSM_STATE_INITIALIZATION */
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);

    /* substate SYS_FSM_CHECK_DEEP_DISCHARGE */
    FRAM_ReadData_ExpectAndReturn(FRAM_BLOCK_ID_DEEP_DISCHARGE_FLAG, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);

    /* substate SYS_FSM_SUBSTATE_START_INITIALIZATION_SBC */
    SBC_SetStateRequest_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);
    /* substate SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_SBC */
    SBC_GetState_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATEMACHINE_RUNNING);
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);

    /* substate SYS_FSM_SUBSTATE_INITIALIZATION_CAN */
    CAN_Initialize_Expect();
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);

    /* substate SYS_FSM_SUBSTATE_INITIALIZE_RTC */
    RTC_IsRtcModuleInitialized_ExpectAndReturn(true);
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);

    /* substate SYS_FSM_SUBSTATE_SYSTEM_BIST */
    DATA_ExecuteDataBist_Expect();
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);

    /* substate SYS_FSM_SUBSTATE_SEND_BOOT_MESSAGE */
    SYS_SendBootMessage_Expect();
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);

    /* state SYS_FSM_STATE_PRE_RUNNING */
    ILCK_SetStateRequest_ExpectAndReturn(ILCK_STATE_INITIALIZATION_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);

    /* substate SYS_FSM_SUBSTATE_START_INITIALIZATION_BAL */
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);
    /* substate SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BAL */
    BAL_GetInitializationState_ExpectAndReturn(STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);
    /* substate SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BAL_GLOBAL_ENABLE */
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_GLOBAL_DISABLE_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);

    /* substate SYS_FSM_SUBSTATE_START_FIRST_MEASUREMENT_CYCLE */
    MEAS_StartMeasurement_ExpectAndReturn(STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);
    /* substate SYS_WAIT_FIRST_MEASUREMENT_CYCLE */
    for (uint16_t i = 0; i < (SYS_STATE_MACHINE_INITIALIZATION_TIMEOUT_MS / SYS_TASK_CYCLE_CONTEXT_MS) + 2; i++) {
        MEAS_IsFirstMeasurementCycleFinished_ExpectAndReturn(false);
        TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);
    }

    /* state SYS_FSM_STATE_ERROR */
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);

    /* ======= RT5/7: Test implementation: Error in IMD Entry */
    SYS_STATE_s sys_stateImdError  = {0};
    sys_stateImdError.currentState = SYS_FSM_STATE_UNINITIALIZED;
    /* state SYS_FSM_STATE_UNINITIALIZED */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SYS_SetStateRequest(SYS_STATE_INITIALIZATION_REQUEST);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_SYS_RunStateMachine(&sys_stateImdError);

    /* state SYS_FSM_STATE_INITIALIZATION */
    TEST_SYS_RunStateMachine(&sys_stateImdError);

    /* Substate SYS_FSM_CHECK_DEEP_DISCHARGE */
    FRAM_ReadData_ExpectAndReturn(FRAM_BLOCK_ID_DEEP_DISCHARGE_FLAG, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdError);

    /* substate SYS_FSM_SUBSTATE_START_INITIALIZATION_SBC */
    SBC_SetStateRequest_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdError);
    /* substate SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_SBC */
    SBC_GetState_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATEMACHINE_RUNNING);
    TEST_SYS_RunStateMachine(&sys_stateImdError);

    /* substate SYS_FSM_SUBSTATE_INITIALIZATION_CAN */
    CAN_Initialize_Expect();
    TEST_SYS_RunStateMachine(&sys_stateImdError);

    /* substate SYS_FSM_SUBSTATE_INITIALIZE_RTC */
    RTC_IsRtcModuleInitialized_ExpectAndReturn(true);
    TEST_SYS_RunStateMachine(&sys_stateImdError);

    /* substate SYS_FSM_SUBSTATE_SYSTEM_BIST */
    DATA_ExecuteDataBist_Expect();
    TEST_SYS_RunStateMachine(&sys_stateImdError);

    /* substate SYS_FSM_SUBSTATE_SEND_BOOT_MESSAGE */
    SYS_SendBootMessage_Expect();
    TEST_SYS_RunStateMachine(&sys_stateImdError);

    /* state SYS_FSM_STATE_PRE_RUNNING */
    ILCK_SetStateRequest_ExpectAndReturn(ILCK_STATE_INITIALIZATION_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdError);

    /* substate SYS_FSM_SUBSTATE_START_INITIALIZATION_BAL */
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdError);
    /* substate SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BAL */
    BAL_GetInitializationState_ExpectAndReturn(STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdError);
    /* substate SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BAL_GLOBAL_ENABLE */
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_GLOBAL_DISABLE_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdError);

    /* substate SYS_FSM_SUBSTATE_START_FIRST_MEASUREMENT_CYCLE */
    MEAS_StartMeasurement_ExpectAndReturn(STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdError);
    /* substate SYS_FSM_SUBSTATE_WAIT_FIRST_MEASUREMENT_CYCLE */
    MEAS_IsFirstMeasurementCycleFinished_ExpectAndReturn(true);
    ALGO_UnlockInitialization_Expect();
    TEST_SYS_RunStateMachine(&sys_stateImdError);

    /* substate SYS_FSM_SUBSTATE_INITIALIZATION_MISC */
    CAN_EnablePeriodic_Expect(true);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        SE_InitializeSoc_Expect(false, s);
        SE_InitializeSoe_Expect(false, s);
        SE_InitializeSoh_Expect(s);
    }
    TEST_SYS_RunStateMachine(&sys_stateImdError);

    /* substate SYS_FSM_SUBSTATE_INITIALIZATION_IMD */
    for (uint16_t i = 0; i < SYS_STATE_MACHINE_INITIALIZATION_REQUEST_RETRY_COUNTER; i++) {
        IMD_RequestInitialization_ExpectAndReturn(IMD_REQUEST_PENDING);
        TEST_SYS_RunStateMachine(&sys_stateImdError);
    }

    /* state SYS_FSM_STATE_ERROR */
    TEST_SYS_RunStateMachine(&sys_stateImdError);

    /* ======= RT6/7: Test implementation: Error in IMD Initialization */
    SYS_STATE_s sys_stateImdInitError  = {0};
    sys_stateImdInitError.currentState = SYS_FSM_STATE_UNINITIALIZED;
    /* state SYS_FSM_STATE_UNINITIALIZED */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SYS_SetStateRequest(SYS_STATE_INITIALIZATION_REQUEST);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);

    /* state SYS_FSM_STATE_INITIALIZATION */
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);

    /* substate SYS_FSM_CHECK_DEEP_DISCHARGE */
    FRAM_ReadData_ExpectAndReturn(FRAM_BLOCK_ID_DEEP_DISCHARGE_FLAG, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);

    /* substate SYS_FSM_SUBSTATE_START_INITIALIZATION_SBC */
    SBC_SetStateRequest_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);
    /* substate SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_SBC */
    SBC_GetState_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATEMACHINE_RUNNING);
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);

    /* substate SYS_FSM_SUBSTATE_INITIALIZATION_CAN */
    CAN_Initialize_Expect();
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);

    /* substate SYS_FSM_SUBSTATE_INITIALIZE_RTC */
    RTC_IsRtcModuleInitialized_ExpectAndReturn(true);
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);

    /* substate SYS_FSM_SUBSTATE_SYSTEM_BIST */
    DATA_ExecuteDataBist_Expect();
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);

    /* substate SYS_FSM_SUBSTATE_SEND_BOOT_MESSAGE */
    SYS_SendBootMessage_Expect();
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);

    /* state SYS_FSM_STATE_PRE_RUNNING */
    ILCK_SetStateRequest_ExpectAndReturn(ILCK_STATE_INITIALIZATION_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);

    /* substate SYS_FSM_SUBSTATE_START_INITIALIZATION_BAL */
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);
    /* substate SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BAL */
    BAL_GetInitializationState_ExpectAndReturn(STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);
    /* substate SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BAL_GLOBAL_ENABLE */
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_GLOBAL_DISABLE_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);

    /* substate SYS_FSM_SUBSTATE_START_FIRST_MEASUREMENT_CYCLE */
    MEAS_StartMeasurement_ExpectAndReturn(STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);
    /* substate SYS_FSM_SUBSTATE_WAIT_FIRST_MEASUREMENT_CYCLE */
    MEAS_IsFirstMeasurementCycleFinished_ExpectAndReturn(true);
    ALGO_UnlockInitialization_Expect();
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);

    /* substate SYS_FSM_SUBSTATE_INITIALIZATION_MISC */
    CAN_EnablePeriodic_Expect(true);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        SE_InitializeSoc_Expect(false, s);
        SE_InitializeSoe_Expect(false, s);
        SE_InitializeSoh_Expect(s);
    }
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);

    /* substate SYS_FSM_SUBSTATE_INITIALIZATION_IMD */
    for (uint16_t i = 0; i < SYS_STATE_MACHINE_INITIALIZATION_REQUEST_RETRY_COUNTER; i++) {
        IMD_RequestInitialization_ExpectAndReturn(IMD_REQUEST_PENDING);
        TEST_SYS_RunStateMachine(&sys_stateImdInitError);
    }

    /* state SYS_FSM_STATE_ERROR */
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);

    /* ======= RT7/7: Test implementation: Error in BMS Initialization */
    SYS_STATE_s sys_stateBmsInitError  = {0};
    sys_stateBmsInitError.currentState = SYS_FSM_STATE_UNINITIALIZED;
    /* state SYS_FSM_STATE_UNINITIALIZED */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SYS_SetStateRequest(SYS_STATE_INITIALIZATION_REQUEST);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);

    /* state SYS_FSM_STATE_INITIALIZATION */
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);

    /* Substate SYS_FSM_CHECK_DEEP_DISCHARGE */
    FRAM_ReadData_ExpectAndReturn(FRAM_BLOCK_ID_DEEP_DISCHARGE_FLAG, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);

    /* substate SYS_FSM_SUBSTATE_START_INITIALIZATION_SBC */
    SBC_SetStateRequest_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);
    /* substate SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_SBC */
    SBC_GetState_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATEMACHINE_RUNNING);
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);

    /* substate SYS_FSM_SUBSTATE_INITIALIZATION_CAN */
    CAN_Initialize_Expect();
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);

    /* substate SYS_FSM_SUBSTATE_INITIALIZE_RTC */
    RTC_IsRtcModuleInitialized_ExpectAndReturn(true);
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);

    /* substate SYS_FSM_SUBSTATE_SYSTEM_BIST */
    DATA_ExecuteDataBist_Expect();
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);

    /* substate SYS_FSM_SUBSTATE_SEND_BOOT_MESSAGE */
    SYS_SendBootMessage_Expect();
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);

    /* state SYS_FSM_STATE_PRE_RUNNING */
    ILCK_SetStateRequest_ExpectAndReturn(ILCK_STATE_INITIALIZATION_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);

    /* substate SYS_FSM_SUBSTATE_START_INITIALIZATION_BAL */
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);
    /* substate SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BAL */
    BAL_GetInitializationState_ExpectAndReturn(STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);
    /* substate SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BAL_GLOBAL_ENABLE */
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_GLOBAL_DISABLE_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);

    /* substate SYS_FSM_SUBSTATE_START_FIRST_MEASUREMENT_CYCLE */
    MEAS_StartMeasurement_ExpectAndReturn(STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);
    /* substate SYS_FSM_SUBSTATE_WAIT_FIRST_MEASUREMENT_CYCLE */
    MEAS_IsFirstMeasurementCycleFinished_ExpectAndReturn(true);
    ALGO_UnlockInitialization_Expect();
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);

    /* substate SYS_FSM_SUBSTATE_INITIALIZATION_MISC */
    CAN_EnablePeriodic_Expect(true);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        SE_InitializeSoc_Expect(false, s);
        SE_InitializeSoe_Expect(false, s);
        SE_InitializeSoh_Expect(s);
    }
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);

    /* substate SYS_FSM_SUBSTATE_INITIALIZATION_IMD */
    IMD_RequestInitialization_ExpectAndReturn(IMD_REQUEST_OK);
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);

    /* substate SYS_FSM_SUBSTATE_START_INITIALIZATION_BMS */
    BMS_SetStateRequest_ExpectAndReturn(BMS_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);
    /* substate SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BMS */
    for (uint16_t i = 0; i < (SYS_STATE_MACHINE_INITIALIZATION_TIMEOUT_MS / SYS_TASK_CYCLE_CONTEXT_MS) + 2; i++) {
        BMS_GetInitializationState_ExpectAndReturn(STD_NOT_OK);
        TEST_SYS_RunStateMachine(&sys_stateBmsInitError);
    }

    /* state SYS_FSM_STATE_ERROR */
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);
}

void testSYS_CheckStateRequest(void) {
    /* ======= Routine tests =============================================== */
    SYS_STATE_REQUEST_e stateRequest = SYS_STATE_INITIALIZATION_REQUEST;

    /* ======= RT1/3: Test implementation: initialization request */
    TEST_SYS_CheckStateRequest(stateRequest);

    /* ======= RT2/3: Test implementation: error request */
    stateRequest = SYS_STATE_ERROR_REQUEST;
    TEST_SYS_CheckStateRequest(stateRequest);

    /* ======= RT3/3: Test implementation: no request */
    stateRequest = SYS_STATE_NO_REQUEST;
    TEST_SYS_CheckStateRequest(stateRequest);
}

/** make sure that the general BIST is running */
void testSYS_GeneralMacroBist(void) {
    TEST_ASSERT_PASS_ASSERT(TEST_SYS_GeneralMacroBist());
}

void testSYS_Trigger(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(SYS_Trigger(NULL_PTR));

    /* ======= Routine tests =============================================== */
    SYS_STATE_s sys_state         = {0};
    STD_RETURN_TYPE_e returnValue = STD_NOT_OK;

    /* ======= RT1/x: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    returnValue = SYS_Trigger(&sys_state);
    TEST_ASSERT_TRUE(returnValue == STD_OK);

    /* ======= RT1/x: Test implementation */
    sys_state.triggerEntry = 1u;
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    returnValue = SYS_Trigger(&sys_state);
    TEST_ASSERT_TRUE(returnValue == STD_NOT_OK);

    /* ======= RT1/x: Test implementation */
    sys_state.triggerEntry = 0u;
    sys_state.timer        = 2u;
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    returnValue = SYS_Trigger(&sys_state);
    TEST_ASSERT_TRUE(returnValue == STD_OK);
}

void testSYS_UninitTests(void) {
    SYS_STATE_s sys_stateNoReq  = {0};
    sys_stateNoReq.currentState = SYS_FSM_STATE_UNINITIALIZED;
    /* state SYS_FSM_STATE_UNINITIALIZED */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SYS_SetStateRequest(SYS_STATE_NO_REQUEST);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_SYS_RunStateMachine(&sys_stateNoReq);

    TEST_ASSERT_EQUAL(SYS_FSM_STATE_UNINITIALIZED, sys_stateNoReq.currentState);
    TEST_ASSERT_EQUAL(0, sys_stateNoReq.illegalRequestsCounter);

    SYS_STATE_s sys_stateIllReq  = {0};
    sys_stateIllReq.currentState = SYS_FSM_STATE_UNINITIALIZED;
    /* state SYS_FSM_STATE_UNINITIALIZED */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SYS_SetStateRequest(SYS_STATE_ERROR_REQUEST);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_SYS_RunStateMachine(&sys_stateIllReq);

    /* state SYS_FSM_STATE_UNINITIALIZED */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SYS_SetStateRequest(SYS_STATE_ERROR_REQUEST);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_SYS_RunStateMachine(&sys_stateIllReq);

    TEST_ASSERT_EQUAL(SYS_FSM_STATE_UNINITIALIZED, sys_stateIllReq.currentState);
    TEST_ASSERT_EQUAL(2, sys_stateIllReq.illegalRequestsCounter);
}

void testSYS_CurrentSensorSubstate(void) {

    /* ======= RT1/3: Test everything works */
    SYS_STATE_s sys_stateCurSens1     = {0};
    sys_stateCurSens1.currentState    = SYS_FSM_STATE_PRE_RUNNING;
    sys_stateCurSens1.currentSubstate = SYS_FSM_SUBSTATE_START_CURRENT_SENSOR_PRESENCE_CHECK;

    /* state SYS_FSM_STATE_PRE_RUNNING, substate SYS_FSM_SUBSTATE_START_CURRENT_SENSOR_PRESENCE_CHECK*/
    CAN_EnablePeriodic_Expect(true);
    TEST_SYS_RunStateMachine(&sys_stateCurSens1);

    /* state SYS_FSM_STATE_PRE_RUNNING, substate SYS_FSM_SUBSTATE_WAIT_CURRENT_SENSOR_PRESENCE_CHECK*/
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        CAN_IsCurrentSensorPresent_ExpectAndReturn(s, true);
        CAN_IsCurrentSensorCcPresent_ExpectAndReturn(s, true);
        SE_InitializeSoc_Expect(true, s);
        CAN_IsCurrentSensorEcPresent_ExpectAndReturn(s, true);
        SE_InitializeSoe_Expect(true, s);
        SE_InitializeSoh_Expect(s);
    }
    SOF_Init_Expect();
    TEST_SYS_RunStateMachine(&sys_stateCurSens1);

    /* ======= RT2/3: Sensor present but not working  */
    SYS_STATE_s sys_stateCurSens2     = {0};
    sys_stateCurSens2.currentState    = SYS_FSM_STATE_PRE_RUNNING;
    sys_stateCurSens2.currentSubstate = SYS_FSM_SUBSTATE_START_CURRENT_SENSOR_PRESENCE_CHECK;

    /* state SYS_FSM_STATE_PRE_RUNNING, substate SYS_FSM_SUBSTATE_START_CURRENT_SENSOR_PRESENCE_CHECK*/
    CAN_EnablePeriodic_Expect(true);
    TEST_SYS_RunStateMachine(&sys_stateCurSens2);

    /* state SYS_FSM_STATE_PRE_RUNNING, substate SYS_FSM_SUBSTATE_WAIT_CURRENT_SENSOR_PRESENCE_CHECK*/
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        CAN_IsCurrentSensorPresent_ExpectAndReturn(s, true);
        CAN_IsCurrentSensorCcPresent_ExpectAndReturn(s, false);
        SE_InitializeSoc_Expect(false, s);
        CAN_IsCurrentSensorEcPresent_ExpectAndReturn(s, false);
        SE_InitializeSoe_Expect(false, s);
        SE_InitializeSoh_Expect(s);
    }
    SOF_Init_Expect();
    TEST_SYS_RunStateMachine(&sys_stateCurSens2);

    /* ======= RT3/3: Sensor not present */
    SYS_STATE_s sys_stateCurSens3     = {0};
    sys_stateCurSens3.currentState    = SYS_FSM_STATE_PRE_RUNNING;
    sys_stateCurSens3.currentSubstate = SYS_FSM_SUBSTATE_START_CURRENT_SENSOR_PRESENCE_CHECK;

    /* state SYS_FSM_STATE_PRE_RUNNING, substate SYS_FSM_SUBSTATE_START_CURRENT_SENSOR_PRESENCE_CHECK*/
    CAN_EnablePeriodic_Expect(true);
    TEST_SYS_RunStateMachine(&sys_stateCurSens3);
    TEST_ASSERT_EQUAL(0, sys_stateCurSens3.initializationTimeout);

    /* state SYS_FSM_STATE_PRE_RUNNING, substate SYS_FSM_SUBSTATE_WAIT_CURRENT_SENSOR_PRESENCE_CHECK*/
    for (uint8_t i = 0; i <= (SYS_STATE_MACHINE_INITIALIZATION_TIMEOUT_MS / SYS_TASK_CYCLE_CONTEXT_MS) + 1; i++) {
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            CAN_IsCurrentSensorPresent_ExpectAndReturn(s, false);
        }
        TEST_SYS_RunStateMachine(&sys_stateCurSens3);
    }

    /* state SYS_FSM_STATE_ERROR */
    TEST_SYS_RunStateMachine(&sys_stateCurSens3);
    TEST_ASSERT_EQUAL(SYS_FSM_STATE_ERROR, sys_stateCurSens3.currentState);
}

void testSYS_SetState(void) {
    uint16_t idleTime = 100u;

    /* ======= RT1/5: NULL_PTR for sys state */
    TEST_ASSERT_FAIL_ASSERT(TEST_SYS_SetState(NULL_PTR, SYS_FSM_STATE_DUMMY, SYS_FSM_SUBSTATE_DUMMY, idleTime));

    /* ======= RT2/5: current state and substate equal next state and substate */
    SYS_STATE_s sys_stateEqual      = {0};
    sys_stateEqual.previousState    = SYS_FSM_STATE_DUMMY;
    sys_stateEqual.previousSubstate = SYS_FSM_SUBSTATE_DUMMY;
    sys_stateEqual.currentState     = SYS_FSM_STATE_PRE_RUNNING;
    sys_stateEqual.currentSubstate  = SYS_FSM_SUBSTATE_START_CURRENT_SENSOR_PRESENCE_CHECK;
    sys_stateEqual.nextState        = SYS_FSM_STATE_PRE_RUNNING;
    sys_stateEqual.nextSubstate     = SYS_FSM_SUBSTATE_WAIT_CURRENT_SENSOR_PRESENCE_CHECK;

    TEST_SYS_SetState(
        &sys_stateEqual, SYS_FSM_STATE_PRE_RUNNING, SYS_FSM_SUBSTATE_START_CURRENT_SENSOR_PRESENCE_CHECK, idleTime);

    TEST_ASSERT_EQUAL(SYS_FSM_STATE_PRE_RUNNING, sys_stateEqual.previousState);
    TEST_ASSERT_EQUAL(SYS_FSM_STATE_PRE_RUNNING, sys_stateEqual.currentState);
    TEST_ASSERT_EQUAL(SYS_FSM_STATE_DUMMY, sys_stateEqual.nextState);

    TEST_ASSERT_EQUAL(SYS_FSM_SUBSTATE_START_CURRENT_SENSOR_PRESENCE_CHECK, sys_stateEqual.previousSubstate);
    TEST_ASSERT_EQUAL(SYS_FSM_SUBSTATE_START_CURRENT_SENSOR_PRESENCE_CHECK, sys_stateEqual.currentSubstate);
    TEST_ASSERT_EQUAL(SYS_FSM_SUBSTATE_DUMMY, sys_stateEqual.nextSubstate);

    TEST_ASSERT_EQUAL(idleTime, sys_stateEqual.timer);

    /* ======= RT3/5: change state to next valid state (not error state) */
    SYS_STATE_s sys_stateNextValid      = {0};
    sys_stateNextValid.previousState    = SYS_FSM_STATE_DUMMY;
    sys_stateNextValid.previousSubstate = SYS_FSM_SUBSTATE_DUMMY;
    sys_stateNextValid.currentState     = SYS_FSM_STATE_INITIALIZATION;
    sys_stateNextValid.currentSubstate  = SYS_FSM_SUBSTATE_SEND_BOOT_MESSAGE;
    sys_stateNextValid.nextState        = SYS_FSM_STATE_PRE_RUNNING;
    sys_stateNextValid.nextSubstate     = SYS_FSM_SUBSTATE_ENTRY;

    TEST_SYS_SetState(&sys_stateNextValid, SYS_FSM_STATE_PRE_RUNNING, SYS_FSM_SUBSTATE_SEND_BOOT_MESSAGE, idleTime);

    TEST_ASSERT_EQUAL(SYS_FSM_STATE_INITIALIZATION, sys_stateNextValid.previousState);
    TEST_ASSERT_EQUAL(SYS_FSM_STATE_PRE_RUNNING, sys_stateNextValid.currentState);
    TEST_ASSERT_EQUAL(SYS_FSM_STATE_DUMMY, sys_stateNextValid.nextState);

    TEST_ASSERT_EQUAL(SYS_FSM_SUBSTATE_SEND_BOOT_MESSAGE, sys_stateNextValid.previousSubstate);
    TEST_ASSERT_EQUAL(SYS_FSM_SUBSTATE_ENTRY, sys_stateNextValid.currentSubstate);
    TEST_ASSERT_EQUAL(SYS_FSM_SUBSTATE_DUMMY, sys_stateNextValid.nextSubstate);

    TEST_ASSERT_EQUAL(idleTime, sys_stateNextValid.timer);

    /* ======= RT4/5: change state to error state */
    SYS_STATE_s sys_stateNextError      = {0};
    sys_stateNextError.previousState    = SYS_FSM_STATE_DUMMY;
    sys_stateNextError.previousSubstate = SYS_FSM_SUBSTATE_DUMMY;
    sys_stateNextError.currentState     = SYS_FSM_STATE_INITIALIZATION;
    sys_stateNextError.currentSubstate  = SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_SBC;

    TEST_SYS_SetState(&sys_stateNextError, SYS_FSM_STATE_ERROR, SYS_FSM_SUBSTATE_SBC_INITIALIZATION_ERROR, idleTime);

    TEST_ASSERT_EQUAL(SYS_FSM_STATE_INITIALIZATION, sys_stateNextError.previousState);
    TEST_ASSERT_EQUAL(SYS_FSM_STATE_ERROR, sys_stateNextError.currentState);

    TEST_ASSERT_EQUAL(SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_SBC, sys_stateNextError.previousSubstate);
    TEST_ASSERT_EQUAL(SYS_FSM_SUBSTATE_SBC_INITIALIZATION_ERROR, sys_stateNextError.currentSubstate);

    TEST_ASSERT_EQUAL(idleTime, sys_stateNextError.timer);

    /* ======= RT5/5: change only substate */
    SYS_STATE_s sys_stateOnlySub      = {0};
    sys_stateOnlySub.previousState    = SYS_FSM_STATE_DUMMY;
    sys_stateOnlySub.previousSubstate = SYS_FSM_SUBSTATE_DUMMY;
    sys_stateOnlySub.currentState     = SYS_FSM_STATE_INITIALIZATION;
    sys_stateOnlySub.currentSubstate  = SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_SBC;

    TEST_SYS_SetState(&sys_stateOnlySub, SYS_FSM_STATE_INITIALIZATION, SYS_FSM_SUBSTATE_INITIALIZATION_CAN, idleTime);

    TEST_ASSERT_EQUAL(SYS_FSM_STATE_INITIALIZATION, sys_stateOnlySub.previousState);
    TEST_ASSERT_EQUAL(SYS_FSM_STATE_INITIALIZATION, sys_stateOnlySub.currentState);

    TEST_ASSERT_EQUAL(SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_SBC, sys_stateOnlySub.previousSubstate);
    TEST_ASSERT_EQUAL(SYS_FSM_SUBSTATE_INITIALIZATION_CAN, sys_stateOnlySub.currentSubstate);

    TEST_ASSERT_EQUAL(idleTime, sys_stateOnlySub.timer);
}
