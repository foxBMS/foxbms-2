/**
 *
 * @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * - &Prime;This product uses parts of foxBMS&reg;&Prime;
 * - &Prime;This product includes parts of foxBMS&reg;&Prime;
 * - &Prime;This product is derived from foxBMS&reg;&Prime;
 *
 */

/**
 * @file    test_algorithm.c
 * @author  foxBMS Team
 * @date    2020-06-30 (date of creation)
 * @updated 2020-06-30 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the algorithm module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockalgorithm_cfg.h"
#include "Mockos.h"
#include "Mocktest_algorithm_stubs.h"

#include "algorithm.h"
#include "test_assert_helper.h"

/*========== Definitions and Implementations for Unit Test ==================*/
ALGO_TASKS_s algo_algorithms[] = {
    {ALGO_UNINITIALIZED, 100, 1000, 0, NULL_PTR, &TEST_AlgorithmComputeFunction},
    {ALGO_UNINITIALIZED, 100, 1000, 0, &TEST_AlgorithmInitializationFunction, &TEST_AlgorithmComputeFunction},
};

const uint16_t algo_length = sizeof(algo_algorithms) / sizeof(algo_algorithms[0]);

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    /* uninitialize everything */
    for (uint16_t i = 0u; i < algo_length; i++) {
        algo_algorithms[i].state = ALGO_UNINITIALIZED;
    }

    /* relock initialization */
    TEST_ALGO_ResetInitializationRequest();
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/* TODO: check behavior when timer flows over for the runtime analysis */

void testUninitializedCallsNothing(void) {
    /* when no algorithm is initialized and init is not unlocked then nothing should be called */
    OS_EnterTaskCritical_Ignore();
    OS_ExitTaskCritical_Ignore();
    ALGO_MainFunction();
    TEST_ASSERT_EQUAL(ALGO_UNINITIALIZED, algo_algorithms[0].state);
    TEST_ASSERT_EQUAL(ALGO_UNINITIALIZED, algo_algorithms[1].state);
}

void testUnlockInitialization(void) {
    /* when no algorithm is initialized and init is not unlocked then nothing should be called */
    OS_EnterTaskCritical_Ignore();
    OS_ExitTaskCritical_Ignore();
    ALGO_MainFunction();
    TEST_ASSERT_EQUAL(ALGO_UNINITIALIZED, algo_algorithms[0].state);
    TEST_ASSERT_EQUAL(ALGO_UNINITIALIZED, algo_algorithms[1].state);

    /* after that if we unlock, then the init process will be handled and the
     first calculation computed */
    ALGO_UnlockInitialization();

    /* call to the init function of the second entry (first one has no init) */
    TEST_AlgorithmInitializationFunction_ExpectAndReturn(STD_OK);

    /* this is the retrieval of the start time for both algorithms and
       after that both algorithms should be called */
    OS_GetTickCount_ExpectAndReturn(0u);
    TEST_AlgorithmComputeFunction_Expect();
    ALGO_MarkAsDone_Expect(0u);
    OS_GetTickCount_ExpectAndReturn(0u);
    TEST_AlgorithmComputeFunction_Expect();
    ALGO_MarkAsDone_Expect(1u);

    ALGO_MainFunction();

    /* afterwards the algorithms should have switched to running as we are
       mocking ALGO_MarkAsDone without function
       (normally the algorithm would then switch back its state upon completion) */
    TEST_ASSERT_EQUAL(ALGO_RUNNING, algo_algorithms[0].state);
    TEST_ASSERT_EQUAL(ALGO_RUNNING, algo_algorithms[1].state);
}

void testUnlockInitializationInvalidAlgorithmConfiguration(void) {
    /* after that if we unlock, then the init process will be handled and the
     first calculation computed */
    OS_EnterTaskCritical_Ignore();
    OS_ExitTaskCritical_Ignore();
    ALGO_UnlockInitialization();

    const uint32_t storeCycleTime = algo_algorithms[0].cycleTime_ms;
    /* set to an invalid cycle time */
    algo_algorithms[0].cycleTime_ms = ALGO_TICK_MS + 1u;

    TEST_ASSERT_FAIL_ASSERT(ALGO_MainFunction());

    TEST_ASSERT_EQUAL(ALGO_UNINITIALIZED, algo_algorithms[0].state);
    TEST_ASSERT_EQUAL(ALGO_UNINITIALIZED, algo_algorithms[1].state);

    /* restore cycle time for other tests */
    algo_algorithms[0].cycleTime_ms = storeCycleTime;
}

void testUnsuccessfulInitialization(void) {
    /* unlock so that we can continue */
    OS_EnterTaskCritical_Ignore();
    OS_ExitTaskCritical_Ignore();
    ALGO_UnlockInitialization();

    /* call to the init function of the second entry (first one has no init)
       this time we indicate a failure */
    TEST_AlgorithmInitializationFunction_ExpectAndReturn(STD_NOT_OK);

    /* now the algorithm without init should be running and the other one in
       error state */
    OS_GetTickCount_ExpectAndReturn(0u);
    TEST_AlgorithmComputeFunction_Expect();
    ALGO_MarkAsDone_Expect(0u);

    ALGO_MainFunction();

    /* afterwards the algorithms should have switched to running and failure state */
    TEST_ASSERT_EQUAL(ALGO_RUNNING, algo_algorithms[0].state);
    TEST_ASSERT_EQUAL(ALGO_FAILED_INIT, algo_algorithms[1].state);

    /* subsequent calls after that should not change anything */
    ALGO_MainFunction();

    TEST_ASSERT_EQUAL(ALGO_RUNNING, algo_algorithms[0].state);
    TEST_ASSERT_EQUAL(ALGO_FAILED_INIT, algo_algorithms[1].state);
}

void testTwoTimesInitialization(void) {
    OS_EnterTaskCritical_Ignore();
    OS_ExitTaskCritical_Ignore();
    OS_GetTickCount_IgnoreAndReturn(0u);
    TEST_AlgorithmComputeFunction_Ignore();
    ALGO_MarkAsDone_Ignore();

    /* unlock so that we can continue */
    ALGO_UnlockInitialization();

    /* call to the init function of the second entry (first one has no init)
       this time we indicate a failure */
    TEST_AlgorithmInitializationFunction_ExpectAndReturn(STD_OK);

    ALGO_MainFunction();

    /* afterwards the algorithms should have switched to running and failure state */
    TEST_ASSERT_EQUAL(ALGO_RUNNING, algo_algorithms[0].state);
    TEST_ASSERT_EQUAL(ALGO_RUNNING, algo_algorithms[1].state);

    /* unlock so that we can continue */
    ALGO_UnlockInitialization();

    /* subsequent calls after that should not change anything */
    ALGO_MainFunction();

    TEST_ASSERT_EQUAL(ALGO_RUNNING, algo_algorithms[0].state);
    TEST_ASSERT_EQUAL(ALGO_RUNNING, algo_algorithms[1].state);
}

void testWrongInitializationImplementation(void) {
    /* unlock so that we can continue */
    OS_EnterTaskCritical_Ignore();
    OS_ExitTaskCritical_Ignore();
    ALGO_UnlockInitialization();

    /* call to the init function of the second entry (first one has no init)
       this time we send something that cannot be returned normally */
    TEST_AlgorithmInitializationFunction_ExpectAndReturn(42u);

    /* as a result the system will fail on init and assert */
    TEST_ASSERT_FAIL_ASSERT(ALGO_MainFunction());
}

void testCycleTimeZero(void) {
    /* this test aims to test what a cycle time of zero does */

    /* unlock so that we can continue */
    OS_EnterTaskCritical_Ignore();
    OS_ExitTaskCritical_Ignore();
    ALGO_UnlockInitialization();

    /* inject a cycle time of zero */
    algo_algorithms[0].cycleTime_ms = 0u;

    /* call the main function;
       if this crashes we ran probably into a division by zero;*/
    TEST_AlgorithmInitializationFunction_ExpectAndReturn(STD_OK);
    OS_GetTickCount_ExpectAndReturn(0u);
    TEST_AlgorithmComputeFunction_Expect();
    ALGO_MarkAsDone_Expect(0u);
    OS_GetTickCount_ExpectAndReturn(0u);
    TEST_AlgorithmComputeFunction_Expect();
    ALGO_MarkAsDone_Expect(1u);
    ALGO_MainFunction();
}

void testMonitorFunctionPassBecauseNotRunning(void) {
    const uint32_t startTime   = 500u;
    const uint32_t currentTime = 0u;
    const ALGO_STATE_e state   = ALGO_READY;

    algo_algorithms[0].startTime = startTime;
    algo_algorithms[0].state     = state;
    OS_GetTickCount_ExpectAndReturn(currentTime);
    ALGO_MonitorExecutionTime();
    TEST_ASSERT_EQUAL(state, algo_algorithms[0].state);
}

void testMonitorFunctionPassBecauseInTime(void) {
    const uint32_t startTime   = 500u;
    const uint32_t currentTime = 500u;
    const ALGO_STATE_e state   = ALGO_RUNNING;

    algo_algorithms[0].startTime = startTime;
    algo_algorithms[0].state     = state;
    OS_GetTickCount_ExpectAndReturn(currentTime);
    ALGO_MonitorExecutionTime();
    TEST_ASSERT_EQUAL(state, algo_algorithms[0].state);
}

void testMonitorFunctionStopBecauseOutOfTime(void) {
    const uint32_t startTime   = 500u;
    const uint32_t currentTime = 50000u;
    const ALGO_STATE_e state   = ALGO_RUNNING;

    algo_algorithms[0].startTime = startTime;
    algo_algorithms[0].state     = state;
    OS_GetTickCount_ExpectAndReturn(currentTime);
    ALGO_MonitorExecutionTime();
    TEST_ASSERT_EQUAL(ALGO_BLOCKED, algo_algorithms[0].state);
}
