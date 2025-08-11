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
 * @file    test_sbc.c
 * @author  foxBMS Team
 * @date    2020-07-15 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the sbc module
 * @details Test functions:
 *          - testSBC_Trigger
 *          - testSBC_TriggerWatchdogIfRequired
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdma.h"
#include "Mockio.h"
#include "Mockmcu.h"
#include "Mocknxpfs85xx.h"
#include "Mockos.h"
#include "Mockportmacro.h"
#include "Mocksbc_fs8x.h"
#include "Mocksbc_fs8x_communication.h"
#include "Mockspi.h"

#include "sbc.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("sbc.c")

TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/dma")
TEST_INCLUDE_PATH("../../src/app/driver/io")
TEST_INCLUDE_PATH("../../src/app/driver/sbc")
TEST_INCLUDE_PATH("../../src/app/driver/sbc/fs8x_driver")
TEST_INCLUDE_PATH("../../src/app/driver/spi")
TEST_INCLUDE_PATH("../../src/app/task/config")

/*========== Definitions and Implementations for Unit Test ==================*/

FS85_STATE_s fs85xx_mcuSupervisor = {0};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testSBC_SaveLastStates(void) {
    /* ======= Routine tests ============================================= */
    SBC_STATE_s sbcState = {0};
    /* ======= RT1/3: Test implementation */
    TEST_SBC_SaveLastStates(&sbcState);

    /* ======= RT2/3: Test implementation */
    sbcState.lastSubstate = SBC_INITIALIZE_SAFETY_PATH_CHECK;
    TEST_SBC_SaveLastStates(&sbcState);

    /* ======= RT3/3: Test implementation */
    sbcState.lastState = SBC_STATEMACHINE_RUNNING;
    TEST_SBC_SaveLastStates(&sbcState);
}

void testSBC_CheckStateRequest(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(TEST_SBC_CheckStateRequest(NULL_PTR, SBC_STATE_INIT_REQUEST));

    /* ======= Routine tests ============================================= */
    SBC_STATE_s sbcState             = {0};
    SBC_STATE_REQUEST_e stateRequest = SBC_STATE_ERROR_REQUEST;

    /* ======= RT1/5: Test implementation */
    TEST_SBC_CheckStateRequest(&sbcState, stateRequest);

    /* ======= RT2/5: Test implementation */
    stateRequest          = SBC_STATE_INIT_REQUEST;
    sbcState.stateRequest = SBC_STATE_NO_REQUEST;
    sbcState.state        = SBC_STATEMACHINE_UNINITIALIZED;
    TEST_SBC_CheckStateRequest(&sbcState, stateRequest);

    /* ======= RT3/5: Test implementation */
    stateRequest          = SBC_STATE_INIT_REQUEST;
    sbcState.stateRequest = SBC_STATE_NO_REQUEST;
    sbcState.state        = SBC_STATEMACHINE_RUNNING;
    TEST_SBC_CheckStateRequest(&sbcState, stateRequest);

    /* ======= RT4/5: Test implementation */
    stateRequest = SBC_STATE_NO_REQUEST;
    TEST_SBC_CheckStateRequest(&sbcState, stateRequest);

    /* ======= RT5/5: Test implementation */
    sbcState.stateRequest = SBC_STATE_ERROR_REQUEST;
    TEST_SBC_CheckStateRequest(&sbcState, stateRequest);
}

void testSBC_CheckReEntrance(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(TEST_SBC_CheckReEntrance(NULL_PTR));

    /* ======= Routine tests ============================================= */
    SBC_STATE_s sbcState = {0};

    /* ======= RT1/1: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_SBC_CheckReEntrance(&sbcState);
}

void testSBC_TransferStateRequest(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(TEST_SBC_TransferStateRequest(NULL_PTR));

    /* ======= Routine tests ============================================= */
    SBC_STATE_s sbcState = {0};

    /* ======= RT1/1: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_SBC_TransferStateRequest(&sbcState);
}

void testSBC_Trigger(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(SBC_Trigger(NULL_PTR));

    /* ======= Routine tests ============================================= */
    SBC_STATE_s sbcState = {0};

    /* ======= RT1/1: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SBC_Trigger(&sbcState);

    /* case SBC_STATEMACHINE_INITIALIZATION, substate SBC_ENTRY */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    FS85_InitializeFsPhase_ExpectAndReturn(sbcState.pFs85xxInstance, STD_OK);
    SBC_Trigger(&sbcState);
    /* substate SBC_INIT_RESET_FAULT_ERROR_COUNTER_PART1 */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    uint8_t requiredWatchdogTrigger = 0;
    FS85_InitializeNumberOfRequiredWatchdogRefreshes_ExpectAndReturn(
        sbcState.pFs85xxInstance, &requiredWatchdogTrigger, STD_OK);
    SBC_Trigger(&sbcState);
    /* substate SBC_INIT_RESET_FAULT_ERROR_COUNTER_PART2 */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    FS85_CheckFaultErrorCounter_ExpectAndReturn(sbcState.pFs85xxInstance, STD_OK);
    SBC_Trigger(&sbcState);
    /* substate SBC_INITIALIZE_SAFETY_PATH_CHECK */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    FS85_SafetyPathChecks_ExpectAndReturn(sbcState.pFs85xxInstance, STD_OK);
    SBC_Trigger(&sbcState);

    /* case SBC_STATEMACHINE_RUNNING */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    sbcState.useIgnitionForPowerDown = true;
    FS85_CheckIgnitionSignal_ExpectAndReturn(sbcState.pFs85xxInstance, true);
    SBC_Trigger(&sbcState);
}

void testSBC_TriggerInitFail(void) {
    /* ======= Routine tests ============================================= */
    SBC_STATE_s sbcState = {0};

    /* ======= RT1/1: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SBC_Trigger(&sbcState);

    /* case SBC_STATEMACHINE_INITIALIZATION, substate SBC_ENTRY */
    for (uint8_t i = 0; i <= 3u; i++) {
        OS_EnterTaskCritical_Expect();
        OS_ExitTaskCritical_Expect();
        FS85_InitializeFsPhase_ExpectAndReturn(sbcState.pFs85xxInstance, STD_NOT_OK);
        SBC_Trigger(&sbcState);
    }

    /* case SBC_STATEMACHINE_ERROR */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SBC_Trigger(&sbcState);
}

void testSBC_TriggerFirstErrorCounter(void) {
    /* ======= Routine tests ============================================= */
    SBC_STATE_s sbcState = {0};

    /* ======= RT1/1: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SBC_Trigger(&sbcState);

    /* case SBC_STATEMACHINE_INITIALIZATION, substate SBC_ENTRY */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    FS85_InitializeFsPhase_ExpectAndReturn(sbcState.pFs85xxInstance, STD_OK);
    SBC_Trigger(&sbcState);
    /* substate SBC_INIT_RESET_FAULT_ERROR_COUNTER_PART1 */
    uint8_t requiredWatchdogTrigger = 0;
    for (uint8_t i = 0; i <= 3u; i++) {
        OS_EnterTaskCritical_Expect();
        OS_ExitTaskCritical_Expect();
        FS85_InitializeNumberOfRequiredWatchdogRefreshes_ExpectAndReturn(
            sbcState.pFs85xxInstance, &requiredWatchdogTrigger, STD_NOT_OK);
        SBC_Trigger(&sbcState);
    }

    /* case SBC_STATEMACHINE_ERROR */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SBC_Trigger(&sbcState);
}

void testSBC_TriggerSecondErrorCounter(void) {
    /* ======= Routine tests ============================================= */
    SBC_STATE_s sbcState = {0};

    /* ======= RT3/1: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SBC_Trigger(&sbcState);

    /* case SBC_STATEMACHINE_INITIALIZATION, substate SBC_ENTRY */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    FS85_InitializeFsPhase_ExpectAndReturn(sbcState.pFs85xxInstance, STD_OK);
    SBC_Trigger(&sbcState);
    /* substate SBC_INIT_RESET_FAULT_ERROR_COUNTER_PART1 */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    uint8_t requiredWatchdogTrigger = 0;
    FS85_InitializeNumberOfRequiredWatchdogRefreshes_ExpectAndReturn(
        sbcState.pFs85xxInstance, &requiredWatchdogTrigger, STD_OK);
    SBC_Trigger(&sbcState);
    /* substate SBC_INIT_RESET_FAULT_ERROR_COUNTER_PART2 */
    for (uint8_t i = 0; i <= 3u; i++) {
        OS_EnterTaskCritical_Expect();
        OS_ExitTaskCritical_Expect();
        FS85_CheckFaultErrorCounter_ExpectAndReturn(sbcState.pFs85xxInstance, STD_NOT_OK);
        SBC_Trigger(&sbcState);
    }

    /* case SBC_STATEMACHINE_ERROR */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SBC_Trigger(&sbcState);
}

void testSBC_TriggerSafetyPathError(void) {
    /* ======= Routine tests ============================================= */
    SBC_STATE_s sbcState = {0};

    /* ======= RT3/1: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SBC_Trigger(&sbcState);

    /* case SBC_STATEMACHINE_INITIALIZATION, substate SBC_ENTRY */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    FS85_InitializeFsPhase_ExpectAndReturn(sbcState.pFs85xxInstance, STD_OK);
    SBC_Trigger(&sbcState);
    /* substate SBC_INIT_RESET_FAULT_ERROR_COUNTER_PART1 */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    uint8_t requiredWatchdogTrigger = 0;
    FS85_InitializeNumberOfRequiredWatchdogRefreshes_ExpectAndReturn(
        sbcState.pFs85xxInstance, &requiredWatchdogTrigger, STD_OK);
    SBC_Trigger(&sbcState);
    /* substate SBC_INIT_RESET_FAULT_ERROR_COUNTER_PART2 */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    FS85_CheckFaultErrorCounter_ExpectAndReturn(sbcState.pFs85xxInstance, STD_OK);
    SBC_Trigger(&sbcState);
    /* substate SBC_INITIALIZE_SAFETY_PATH_CHECK */
    for (uint8_t i = 0; i <= 3u; i++) {
        OS_EnterTaskCritical_Expect();
        OS_ExitTaskCritical_Expect();
        FS85_SafetyPathChecks_ExpectAndReturn(sbcState.pFs85xxInstance, STD_NOT_OK);
        SBC_Trigger(&sbcState);
    }

    /* case SBC_STATEMACHINE_ERROR */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SBC_Trigger(&sbcState);
}

void testSBC_TriggerWatchdogIfRequired(void) {
    TEST_ASSERT_FALSE(TEST_SBC_TriggerWatchdogIfRequired(&sbc_stateMcuSupervisor));
    sbc_stateMcuSupervisor.watchdogTrigger = 10u;
    /* decrement so we are going to see a trigger event */
    for (int i = 0; i < sbc_stateMcuSupervisor.watchdogPeriod_10ms - 1; i++) {
        TEST_ASSERT_FALSE(TEST_SBC_TriggerWatchdogIfRequired(&sbc_stateMcuSupervisor));
    }
    /* timer has elapsed now, we should see a trigger event */
    FS85_TriggerWatchdog_ExpectAndReturn(sbc_stateMcuSupervisor.pFs85xxInstance, STD_OK);
    TEST_ASSERT_TRUE(TEST_SBC_TriggerWatchdogIfRequired(&sbc_stateMcuSupervisor));
    TEST_ASSERT_EQUAL(sbc_stateMcuSupervisor.watchdogTrigger, sbc_stateMcuSupervisor.watchdogPeriod_10ms);

    /* elapse timer and fail to run watchdog trigger */
    sbc_stateMcuSupervisor.watchdogTrigger = 1u;
    FS85_TriggerWatchdog_ExpectAndReturn(sbc_stateMcuSupervisor.pFs85xxInstance, STD_NOT_OK);
    TEST_ASSERT_FALSE(TEST_SBC_TriggerWatchdogIfRequired(&sbc_stateMcuSupervisor));
}

void testSBC_SetStateRequest(void) {
    /* ======= Assertion tests ============================================= */
    SBC_STATE_REQUEST_e stateRequestValid = SBC_STATE_INIT_REQUEST;
    /* ======= AT1/1: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(SBC_SetStateRequest(NULL_PTR, stateRequestValid));

    /* ======= Routine tests =============================================== */
    SBC_STATE_s sbc_state            = {0};
    SBC_STATE_REQUEST_e stateRequest = SBC_STATE_ERROR_REQUEST;

    /* ======= RT1/2: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SBC_SetStateRequest(&sbc_state, stateRequest);

    /* ======= RT2/2: Test implementation */
    stateRequest = SBC_STATE_INIT_REQUEST;
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SBC_SetStateRequest(&sbc_state, stateRequest);
}

void testSBC_GetState(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(SBC_GetState(NULL_PTR));

    /* ======= Routine tests =============================================== */
    SBC_STATE_s sbc_state = {0};

    /* ======= RT1/1: Test implementation */
    SBC_GetState(&sbc_state);
}
