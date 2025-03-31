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
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
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
    SYS_STATE_s sys_state = {0};
    sys_state.state       = SYS_STATEMACH_UNINITIALIZED;

    /* Test State Machine naturally, without special input */
    /* state SYS_STATEMACH_UNINITIALIZED */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SYS_SetStateRequest(SYS_STATE_INITIALIZATION_REQUEST);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_SYS_RunStateMachine(&sys_state);

    /* state SYS_STATEMACH_INITIALIZATION */
    FRAM_ReadData_ExpectAndReturn(FRAM_BLOCK_ID_DEEP_DISCHARGE_FLAG, STD_OK);
    TEST_SYS_RunStateMachine(&sys_state);

    /* state SYS_STATEMACH_INITIALIZE_SBC, substate SYS_ENTRY */
    SBC_SetStateRequest_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_state);
    /* substate SYS_WAIT_INITIALIZATION_SBC */
    SBC_GetState_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATEMACHINE_RUNNING);
    TEST_SYS_RunStateMachine(&sys_state);

    /* state SYS_STATEMACH_INITIALIZE_CAN */
    CAN_Initialize_Expect();
    TEST_SYS_RunStateMachine(&sys_state);

    /* state SYS_STATEMACH_SYSTEM_BIST */
    DATA_ExecuteDataBist_Expect();
    TEST_SYS_RunStateMachine(&sys_state);

    /* state SYS_STATEMACH_INITIALIZED */
    SYS_SendBootMessage_Expect();
    TEST_SYS_RunStateMachine(&sys_state);

    /* state state SYS_STATEMACH_INITIALIZE_INTERLOCK */
    ILCK_SetStateRequest_ExpectAndReturn(ILCK_STATE_INITIALIZATION_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_state);

    /* state SYS_STATEMACH_INITIALIZE_BALANCING, substate SYS_ENTRY */
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_state);
    /* substate SYS_WAIT_INITIALIZATION_BAL */
    BAL_GetInitializationState_ExpectAndReturn(STD_OK);
    TEST_SYS_RunStateMachine(&sys_state);
    /* substate SYS_WAIT_INITIALIZATION_BAL_GLOBAL_ENABLE */
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_GLOBAL_DISABLE_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_state);

    /* state SYS_STATEMACH_FIRST_MEASUREMENT_CYCLE, substate SYS_ENTRY */
    MEAS_StartMeasurement_ExpectAndReturn(STD_OK);
    TEST_SYS_RunStateMachine(&sys_state);
    /* substate SYS_WAIT_FIRST_MEASUREMENT_CYCLE */
    MEAS_IsFirstMeasurementCycleFinished_ExpectAndReturn(true);
    ALGO_UnlockInitialization_Expect();
    TEST_SYS_RunStateMachine(&sys_state);

    /* state SYS_STATEMACH_INITIALIZE_MISC */
    CAN_EnablePeriodic_Expect(true);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        SE_InitializeSoc_Expect(false, s);
        SE_InitializeSoe_Expect(false, s);
        SE_InitializeSoh_Expect(s);
    }
    TEST_SYS_RunStateMachine(&sys_state);

    /* state SYS_STATEMACH_INITIALIZE_IMD, substate SYS_ENTRY */
    IMD_RequestInitialization_ExpectAndReturn(IMD_REQUEST_OK);
    TEST_SYS_RunStateMachine(&sys_state);

    /* state SYS_STATEMACH_INITIALIZE_BMS, substate SYS_ENTRY */
    BMS_SetStateRequest_ExpectAndReturn(BMS_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_state);

    /* state SYS_STATEMACH_INITIALIZE_BMS, substate SYS_WAIT_INITIALIZATION_BMS */
    BMS_GetInitializationState_ExpectAndReturn(STD_OK);
    TEST_SYS_RunStateMachine(&sys_state);

    /* state SYS_STATEMACH_RUNNING */
    TEST_SYS_RunStateMachine(&sys_state);
}

void testSYS_RunStateMachineErrorState(void) {
    /* ======= Routine tests =============================================== */
    SYS_STATE_s sys_stateSbcError = {0};
    sys_stateSbcError.state       = SYS_STATEMACH_UNINITIALIZED;

    /* ======= RT1/7: Test implementation: Error in SBC Initialization */
    /* state SYS_STATEMACH_UNINITIALIZED */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SYS_SetStateRequest(SYS_STATE_INITIALIZATION_REQUEST);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_SYS_RunStateMachine(&sys_stateSbcError);

    /* state SYS_STATEMACH_INITIALIZATION */
    FRAM_ReadData_ExpectAndReturn(FRAM_BLOCK_ID_DEEP_DISCHARGE_FLAG, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateSbcError);

    /* state SYS_STATEMACH_INITIALIZE_SBC, substate SYS_ENTRY */
    SBC_SetStateRequest_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateSbcError);
    /* substate SYS_WAIT_INITIALIZATION_SBC */
    for (uint16_t i = 0; i < (SYS_STATEMACHINE_SBC_INIT_TIMEOUT_MS / SYS_TASK_CYCLE_CONTEXT_MS) + 2; i++) {
        SBC_GetState_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATEMACHINE_ERROR);
        TEST_SYS_RunStateMachine(&sys_stateSbcError);
    }
    /* state SYS_STATEMACH_ERROR */
    TEST_SYS_RunStateMachine(&sys_stateSbcError);

    /* ======= RT2/7: Test implementation: Error in Balancing Initialization */
    SYS_STATE_s sys_stateBalancingError = {0};
    sys_stateBalancingError.state       = SYS_STATEMACH_UNINITIALIZED;
    /* state SYS_STATEMACH_UNINITIALIZED */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SYS_SetStateRequest(SYS_STATE_INITIALIZATION_REQUEST);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_SYS_RunStateMachine(&sys_stateBalancingError);

    /* state SYS_STATEMACH_INITIALIZATION */
    FRAM_ReadData_ExpectAndReturn(FRAM_BLOCK_ID_DEEP_DISCHARGE_FLAG, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateBalancingError);

    /* state SYS_STATEMACH_INITIALIZE_SBC, substate SYS_ENTRY */
    SBC_SetStateRequest_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateBalancingError);
    /* substate SYS_WAIT_INITIALIZATION_SBC */
    SBC_GetState_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATEMACHINE_RUNNING);
    TEST_SYS_RunStateMachine(&sys_stateBalancingError);

    /* state SYS_STATEMACH_INITIALIZE_CAN */
    CAN_Initialize_Expect();
    TEST_SYS_RunStateMachine(&sys_stateBalancingError);

    /* state SYS_STATEMACH_SYSTEM_BIST */
    DATA_ExecuteDataBist_Expect();
    TEST_SYS_RunStateMachine(&sys_stateBalancingError);

    /* state SYS_STATEMACH_INITIALIZED */
    SYS_SendBootMessage_Expect();
    TEST_SYS_RunStateMachine(&sys_stateBalancingError);

    /* state state SYS_STATEMACH_INITIALIZE_INTERLOCK */
    ILCK_SetStateRequest_ExpectAndReturn(ILCK_STATE_INITIALIZATION_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateBalancingError);

    /* state SYS_STATEMACH_INITIALIZE_BALANCING, substate SYS_ENTRY */
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateBalancingError);
    /* substate SYS_WAIT_INITIALIZATION_BAL */
    for (uint16_t i = 0; i < (SYS_STATEMACH_BAL_INITIALIZATION_TIMEOUT_MS / SYS_TASK_CYCLE_CONTEXT_MS) + 2; i++) {
        BAL_GetInitializationState_ExpectAndReturn(STD_NOT_OK);
        TEST_SYS_RunStateMachine(&sys_stateBalancingError);
    }
    /* state SYS_STATEMACH_ERROR */
    TEST_SYS_RunStateMachine(&sys_stateBalancingError);

    /* ======= RT3/7: Test implementation: Error in Global Balancing Initialization */
    SYS_STATE_s sys_stateGlobalBalancingError = {0};
    sys_stateGlobalBalancingError.state       = SYS_STATEMACH_UNINITIALIZED;
    /* state SYS_STATEMACH_UNINITIALIZED */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SYS_SetStateRequest(SYS_STATE_INITIALIZATION_REQUEST);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_SYS_RunStateMachine(&sys_stateGlobalBalancingError);

    /* state SYS_STATEMACH_INITIALIZATION */
    FRAM_ReadData_ExpectAndReturn(FRAM_BLOCK_ID_DEEP_DISCHARGE_FLAG, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateGlobalBalancingError);

    /* state SYS_STATEMACH_INITIALIZE_SBC, substate SYS_ENTRY */
    SBC_SetStateRequest_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateGlobalBalancingError);
    /* substate SYS_WAIT_INITIALIZATION_SBC */
    SBC_GetState_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATEMACHINE_RUNNING);
    TEST_SYS_RunStateMachine(&sys_stateGlobalBalancingError);

    /* state SYS_STATEMACH_INITIALIZE_CAN */
    CAN_Initialize_Expect();
    TEST_SYS_RunStateMachine(&sys_stateGlobalBalancingError);

    /* state SYS_STATEMACH_SYSTEM_BIST */
    DATA_ExecuteDataBist_Expect();
    TEST_SYS_RunStateMachine(&sys_stateGlobalBalancingError);

    /* state SYS_STATEMACH_INITIALIZED */
    SYS_SendBootMessage_Expect();
    TEST_SYS_RunStateMachine(&sys_stateGlobalBalancingError);

    /* state state SYS_STATEMACH_INITIALIZE_INTERLOCK */
    ILCK_SetStateRequest_ExpectAndReturn(ILCK_STATE_INITIALIZATION_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateGlobalBalancingError);

    /* state SYS_STATEMACH_INITIALIZE_BALANCING, substate SYS_ENTRY */
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateGlobalBalancingError);
    /* substate SYS_WAIT_INITIALIZATION_BAL */
    BAL_GetInitializationState_ExpectAndReturn(STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateGlobalBalancingError);
    /* substate SYS_WAIT_INITIALIZATION_BAL_GLOBAL_ENABLE */
    for (uint16_t i = 0; i < (SYS_STATEMACH_INITIALIZATION_TIMEOUT_MS / SYS_TASK_CYCLE_CONTEXT_MS) + 2; i++) {
        BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_GLOBAL_DISABLE_REQUEST, STD_NOT_OK);
        TEST_SYS_RunStateMachine(&sys_stateGlobalBalancingError);
    }
    /* state SYS_STATEMACH_ERROR */
    TEST_SYS_RunStateMachine(&sys_stateGlobalBalancingError);

    /* ======= RT4/7: Test implementation: Error in Balancing */
    SYS_STATE_s sys_stateFirstMeasurementError = {0};
    sys_stateFirstMeasurementError.state       = SYS_STATEMACH_UNINITIALIZED;
    /* state SYS_STATEMACH_UNINITIALIZED */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SYS_SetStateRequest(SYS_STATE_INITIALIZATION_REQUEST);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);

    /* state SYS_STATEMACH_INITIALIZATION */
    FRAM_ReadData_ExpectAndReturn(FRAM_BLOCK_ID_DEEP_DISCHARGE_FLAG, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);

    /* state SYS_STATEMACH_INITIALIZE_SBC, substate SYS_ENTRY */
    SBC_SetStateRequest_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);
    /* substate SYS_WAIT_INITIALIZATION_SBC */
    SBC_GetState_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATEMACHINE_RUNNING);
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);

    /* state SYS_STATEMACH_INITIALIZE_CAN */
    CAN_Initialize_Expect();
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);

    /* state SYS_STATEMACH_SYSTEM_BIST */
    DATA_ExecuteDataBist_Expect();
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);

    /* state SYS_STATEMACH_INITIALIZED */
    SYS_SendBootMessage_Expect();
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);

    /* state state SYS_STATEMACH_INITIALIZE_INTERLOCK */
    ILCK_SetStateRequest_ExpectAndReturn(ILCK_STATE_INITIALIZATION_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);

    /* state SYS_STATEMACH_INITIALIZE_BALANCING, substate SYS_ENTRY */
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);
    /* substate SYS_WAIT_INITIALIZATION_BAL */
    BAL_GetInitializationState_ExpectAndReturn(STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);
    /* substate SYS_WAIT_INITIALIZATION_BAL_GLOBAL_ENABLE */
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_GLOBAL_DISABLE_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);

    /* state SYS_STATEMACH_FIRST_MEASUREMENT_CYCLE, substate SYS_ENTRY */
    MEAS_StartMeasurement_ExpectAndReturn(STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);
    /* substate SYS_WAIT_FIRST_MEASUREMENT_CYCLE */
    for (uint16_t i = 0; i < (SYS_STATEMACH_INITIALIZATION_TIMEOUT_MS / SYS_TASK_CYCLE_CONTEXT_MS) + 2; i++) {
        MEAS_IsFirstMeasurementCycleFinished_ExpectAndReturn(false);
        TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);
    }

    /* state SYS_STATEMACH_ERROR */
    TEST_SYS_RunStateMachine(&sys_stateFirstMeasurementError);

    /* ======= RT5/7: Test implementation: Error in IMD Entry */
    SYS_STATE_s sys_stateImdError = {0};
    sys_stateImdError.state       = SYS_STATEMACH_UNINITIALIZED;
    /* state SYS_STATEMACH_UNINITIALIZED */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SYS_SetStateRequest(SYS_STATE_INITIALIZATION_REQUEST);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_SYS_RunStateMachine(&sys_stateImdError);

    /* state SYS_STATEMACH_INITIALIZATION */
    FRAM_ReadData_ExpectAndReturn(FRAM_BLOCK_ID_DEEP_DISCHARGE_FLAG, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdError);

    /* state SYS_STATEMACH_INITIALIZE_SBC, substate SYS_ENTRY */
    SBC_SetStateRequest_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdError);
    /* substate SYS_WAIT_INITIALIZATION_SBC */
    SBC_GetState_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATEMACHINE_RUNNING);
    TEST_SYS_RunStateMachine(&sys_stateImdError);

    /* state SYS_STATEMACH_INITIALIZE_CAN */
    CAN_Initialize_Expect();
    TEST_SYS_RunStateMachine(&sys_stateImdError);

    /* state SYS_STATEMACH_SYSTEM_BIST */
    DATA_ExecuteDataBist_Expect();
    TEST_SYS_RunStateMachine(&sys_stateImdError);

    /* state SYS_STATEMACH_INITIALIZED */
    SYS_SendBootMessage_Expect();
    TEST_SYS_RunStateMachine(&sys_stateImdError);

    /* state state SYS_STATEMACH_INITIALIZE_INTERLOCK */
    ILCK_SetStateRequest_ExpectAndReturn(ILCK_STATE_INITIALIZATION_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdError);

    /* state SYS_STATEMACH_INITIALIZE_BALANCING, substate SYS_ENTRY */
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdError);
    /* substate SYS_WAIT_INITIALIZATION_BAL */
    BAL_GetInitializationState_ExpectAndReturn(STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdError);
    /* substate SYS_WAIT_INITIALIZATION_BAL_GLOBAL_ENABLE */
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_GLOBAL_DISABLE_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdError);

    /* state SYS_STATEMACH_FIRST_MEASUREMENT_CYCLE, substate SYS_ENTRY */
    MEAS_StartMeasurement_ExpectAndReturn(STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdError);
    /* substate SYS_WAIT_FIRST_MEASUREMENT_CYCLE */
    MEAS_IsFirstMeasurementCycleFinished_ExpectAndReturn(true);
    ALGO_UnlockInitialization_Expect();
    TEST_SYS_RunStateMachine(&sys_stateImdError);

    /* state SYS_STATEMACH_INITIALIZE_MISC */
    CAN_EnablePeriodic_Expect(true);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        SE_InitializeSoc_Expect(false, s);
        SE_InitializeSoe_Expect(false, s);
        SE_InitializeSoh_Expect(s);
    }
    TEST_SYS_RunStateMachine(&sys_stateImdError);

    /* state SYS_STATEMACH_INITIALIZE_IMD, substate SYS_ENTRY */
    for (uint16_t i = 0; i < SYS_STATEMACH_INITIALIZATION_REQUEST_RETRY_COUNTER; i++) {
        IMD_RequestInitialization_ExpectAndReturn(IMD_REQUEST_PENDING);
        TEST_SYS_RunStateMachine(&sys_stateImdError);
    }

    /* state SYS_STATEMACH_ERROR */
    TEST_SYS_RunStateMachine(&sys_stateImdError);

    /* ======= RT6/7: Test implementation: Error in IMD Initialization */
    SYS_STATE_s sys_stateImdInitError = {0};
    sys_stateImdInitError.state       = SYS_STATEMACH_UNINITIALIZED;
    /* state SYS_STATEMACH_UNINITIALIZED */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SYS_SetStateRequest(SYS_STATE_INITIALIZATION_REQUEST);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);

    /* state SYS_STATEMACH_INITIALIZATION */
    FRAM_ReadData_ExpectAndReturn(FRAM_BLOCK_ID_DEEP_DISCHARGE_FLAG, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);

    /* state SYS_STATEMACH_INITIALIZE_SBC, substate SYS_ENTRY */
    SBC_SetStateRequest_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);
    /* substate SYS_WAIT_INITIALIZATION_SBC */
    SBC_GetState_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATEMACHINE_RUNNING);
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);

    /* state SYS_STATEMACH_INITIALIZE_CAN */
    CAN_Initialize_Expect();
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);

    /* state SYS_STATEMACH_SYSTEM_BIST */
    DATA_ExecuteDataBist_Expect();
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);

    /* state SYS_STATEMACH_INITIALIZED */
    SYS_SendBootMessage_Expect();
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);

    /* state state SYS_STATEMACH_INITIALIZE_INTERLOCK */
    ILCK_SetStateRequest_ExpectAndReturn(ILCK_STATE_INITIALIZATION_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);

    /* state SYS_STATEMACH_INITIALIZE_BALANCING, substate SYS_ENTRY */
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);
    /* substate SYS_WAIT_INITIALIZATION_BAL */
    BAL_GetInitializationState_ExpectAndReturn(STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);
    /* substate SYS_WAIT_INITIALIZATION_BAL_GLOBAL_ENABLE */
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_GLOBAL_DISABLE_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);

    /* state SYS_STATEMACH_FIRST_MEASUREMENT_CYCLE, substate SYS_ENTRY */
    MEAS_StartMeasurement_ExpectAndReturn(STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);
    /* substate SYS_WAIT_FIRST_MEASUREMENT_CYCLE */
    MEAS_IsFirstMeasurementCycleFinished_ExpectAndReturn(true);
    ALGO_UnlockInitialization_Expect();
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);

    /* state SYS_STATEMACH_INITIALIZE_MISC */
    CAN_EnablePeriodic_Expect(true);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        SE_InitializeSoc_Expect(false, s);
        SE_InitializeSoe_Expect(false, s);
        SE_InitializeSoh_Expect(s);
    }
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);

    /* state SYS_STATEMACH_INITIALIZE_IMD, substate SYS_ENTRY */
    IMD_RequestInitialization_ExpectAndReturn(IMD_REQUEST_PENDING);
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);
    /* state SYS_STATEMACH_INITIALIZE_IMD, substate SYS_WAIT_INITIALIZATION_IMD */
    sys_stateImdInitError.substate = SYS_WAIT_INITIALIZATION_IMD;
    for (uint16_t i = 0; i < (SYS_STATEMACH_IMD_INITIALIZATION_TIMEOUT_MS / SYS_TASK_CYCLE_CONTEXT_MS) + 1; i++) {
        IMD_GetInitializationState_ExpectAndReturn(false);
        TEST_SYS_RunStateMachine(&sys_stateImdInitError);
    }

    /* state SYS_STATEMACH_ERROR */
    TEST_SYS_RunStateMachine(&sys_stateImdInitError);

    /* ======= RT7/7: Test implementation: Error in BMS Initialization */
    SYS_STATE_s sys_stateBmsInitError = {0};
    sys_stateBmsInitError.state       = SYS_STATEMACH_UNINITIALIZED;
    /* state SYS_STATEMACH_UNINITIALIZED */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SYS_SetStateRequest(SYS_STATE_INITIALIZATION_REQUEST);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);

    /* state SYS_STATEMACH_INITIALIZATION */
    FRAM_ReadData_ExpectAndReturn(FRAM_BLOCK_ID_DEEP_DISCHARGE_FLAG, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);

    /* state SYS_STATEMACH_INITIALIZE_SBC, substate SYS_ENTRY */
    SBC_SetStateRequest_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);
    /* substate SYS_WAIT_INITIALIZATION_SBC */
    SBC_GetState_ExpectAndReturn(&sbc_stateMcuSupervisor, SBC_STATEMACHINE_RUNNING);
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);

    /* state SYS_STATEMACH_INITIALIZE_CAN */
    CAN_Initialize_Expect();
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);

    /* state SYS_STATEMACH_SYSTEM_BIST */
    DATA_ExecuteDataBist_Expect();
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);

    /* state SYS_STATEMACH_INITIALIZED */
    SYS_SendBootMessage_Expect();
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);

    /* state state SYS_STATEMACH_INITIALIZE_INTERLOCK */
    ILCK_SetStateRequest_ExpectAndReturn(ILCK_STATE_INITIALIZATION_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);

    /* state SYS_STATEMACH_INITIALIZE_BALANCING, substate SYS_ENTRY */
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);
    /* substate SYS_WAIT_INITIALIZATION_BAL */
    BAL_GetInitializationState_ExpectAndReturn(STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);
    /* substate SYS_WAIT_INITIALIZATION_BAL_GLOBAL_ENABLE */
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_GLOBAL_DISABLE_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);

    /* state SYS_STATEMACH_FIRST_MEASUREMENT_CYCLE, substate SYS_ENTRY */
    MEAS_StartMeasurement_ExpectAndReturn(STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);
    /* substate SYS_WAIT_FIRST_MEASUREMENT_CYCLE */
    MEAS_IsFirstMeasurementCycleFinished_ExpectAndReturn(true);
    ALGO_UnlockInitialization_Expect();
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);

    /* state SYS_STATEMACH_INITIALIZE_MISC */
    CAN_EnablePeriodic_Expect(true);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        SE_InitializeSoc_Expect(false, s);
        SE_InitializeSoe_Expect(false, s);
        SE_InitializeSoh_Expect(s);
    }
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);

    /* state SYS_STATEMACH_INITIALIZE_IMD, substate SYS_ENTRY */
    IMD_RequestInitialization_ExpectAndReturn(IMD_REQUEST_OK);
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);

    /* state SYS_STATEMACH_INITIALIZE_BMS, substate SYS_ENTRY */
    BMS_SetStateRequest_ExpectAndReturn(BMS_STATE_INIT_REQUEST, STD_OK);
    TEST_SYS_RunStateMachine(&sys_stateBmsInitError);
    /* state SYS_STATEMACH_INITIALIZE_BMS, substate SYS_WAIT_INITIALIZATION_BMS */
    for (uint16_t i = 0; i < (SYS_STATEMACH_INITIALIZATION_TIMEOUT_MS / SYS_TASK_CYCLE_CONTEXT_MS) + 2; i++) {
        BMS_GetInitializationState_ExpectAndReturn(STD_NOT_OK);
        TEST_SYS_RunStateMachine(&sys_stateBmsInitError);
    }

    /* state SYS_STATEMACH_ERROR */
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
    TEST_ASSERT_PASS_ASSERT(SYS_GeneralMacroBist());
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
