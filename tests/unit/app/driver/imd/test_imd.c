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
 * @file    test_imd.c
 * @author  foxBMS Team
 * @date    2021-11-15 (date of creation)
 * @updated 2024-12-20 (date of last update)
 * @version v1.8.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the Insulation Monitoring Device (IMD) driver
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockos.h"

#include "imd.h"
#include "no-imd.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("imd.c")

TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/imd")
TEST_INCLUDE_PATH("../../src/app/driver/imd/none")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/
IMD_STATE_s test_imd_state;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    test_imd_state = (IMD_STATE_s){
        .timer                                 = 0u,
        .triggerEntry                          = 0u,
        .nextState                             = IMD_FSM_STATE_DUMMY,
        .stateRequest                          = IMD_STATE_NO_REQUEST,
        .currentState                          = IMD_FSM_STATE_HAS_NEVER_RUN,
        .previousState                         = IMD_FSM_STATE_DUMMY,
        .nextSubstate                          = IMD_FSM_SUBSTATE_DUMMY,
        .currentSubstate                       = IMD_FSM_SUBSTATE_DUMMY,
        .previousSubstate                      = IMD_FSM_SUBSTATE_DUMMY,
        .information.isStateMachineInitialized = false,
        .information.switchImdDeviceOn         = false,
        .pTableImd                             = NULL_PTR,
    };
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testIMD_CheckStateRequest(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_IMD_CheckStateRequest(NULL_PTR, IMD_STATE_INITIALIZE_REQUEST));

    test_imd_state.currentState = IMD_FSM_STATE_UNINITIALIZED;
    TEST_ASSERT_EQUAL(IMD_REQUEST_OK, TEST_IMD_CheckStateRequest(&test_imd_state, IMD_STATE_INITIALIZE_REQUEST));

    test_imd_state.currentState = IMD_FSM_STATE_HAS_NEVER_RUN;
    TEST_ASSERT_EQUAL(
        IMD_ALREADY_INITIALIZED, TEST_IMD_CheckStateRequest(&test_imd_state, IMD_STATE_INITIALIZE_REQUEST));

    test_imd_state.currentState = IMD_FSM_STATE_SHUTDOWN;
    TEST_ASSERT_EQUAL(IMD_REQUEST_OK, TEST_IMD_CheckStateRequest(&test_imd_state, IMD_STATE_SWITCH_ON_REQUEST));

    test_imd_state.currentState = IMD_FSM_STATE_IMD_ENABLE;
    TEST_ASSERT_EQUAL(IMD_REQUEST_OK, TEST_IMD_CheckStateRequest(&test_imd_state, IMD_STATE_SWITCH_ON_REQUEST));

    test_imd_state.currentState = IMD_FSM_STATE_HAS_NEVER_RUN;
    TEST_ASSERT_EQUAL(IMD_ILLEGAL_REQUEST, TEST_IMD_CheckStateRequest(&test_imd_state, IMD_STATE_SWITCH_ON_REQUEST));

    test_imd_state.stateRequest = IMD_STATE_SWITCH_ON_REQUEST;
    TEST_ASSERT_EQUAL(IMD_REQUEST_PENDING, TEST_IMD_CheckStateRequest(&test_imd_state, IMD_STATE_INITIALIZE_REQUEST));
}

void testIMD_SetStateRequest(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_IMD_SetStateRequest(NULL_PTR, IMD_STATE_INITIALIZE_REQUEST));

    test_imd_state.currentState = IMD_FSM_STATE_UNINITIALIZED;
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_ASSERT_EQUAL(IMD_REQUEST_OK, TEST_IMD_SetStateRequest(&test_imd_state, IMD_STATE_INITIALIZE_REQUEST));
}

void testIMD_TransferStateRequest(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_IMD_TransferStateRequest(NULL_PTR));

    test_imd_state.stateRequest = IMD_STATE_INITIALIZE_REQUEST;
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_ASSERT_EQUAL(IMD_STATE_INITIALIZE_REQUEST, TEST_IMD_TransferStateRequest(&test_imd_state));
}

void test_IMD_CheckMultipleCalls(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_IMD_CheckMultipleCalls(NULL_PTR));

    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_ASSERT_EQUAL(IMD_MULTIPLE_CALLS_NO, TEST_IMD_CheckMultipleCalls(&test_imd_state));

    test_imd_state.triggerEntry = 10u;
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_ASSERT_EQUAL(IMD_MULTIPLE_CALLS_YES, TEST_IMD_CheckMultipleCalls(&test_imd_state));
}

void test_IMD_SetState(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_IMD_SetState(NULL_PTR, IMD_FSM_STATE_DUMMY, IMD_FSM_SUBSTATE_DUMMY, 100u));

    test_imd_state.currentState    = IMD_FSM_STATE_IMD_ENABLE;
    test_imd_state.currentSubstate = IMD_FSM_SUBSTATE_RUNNING_0;
    TEST_IMD_SetState(&test_imd_state, IMD_FSM_STATE_IMD_ENABLE, IMD_FSM_SUBSTATE_RUNNING_0, 100u);

    TEST_ASSERT_EQUAL(IMD_FSM_STATE_DUMMY, test_imd_state.nextState);
    TEST_ASSERT_EQUAL(IMD_FSM_SUBSTATE_DUMMY, test_imd_state.nextSubstate);
    TEST_ASSERT_EQUAL(100u, test_imd_state.timer);
}
