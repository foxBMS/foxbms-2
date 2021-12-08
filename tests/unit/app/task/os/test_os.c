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
 * @file    test_os.c
 * @author  foxBMS Team
 * @date    2020-03-13 (date of creation)
 * @updated 2021-12-01 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  OS
 *
 * @brief   Test of the os.c module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockftask.h"
#include "Mockftask_cfg.h"
#include "Mockportmacro.h"
#include "Mockqueue.h"
#include "Mocktask.h"

#include "os.h"
#include "test_assert_helper.h"

TEST_FILE("os.c")
TEST_FILE("os_freertos.c")

/*========== Definitions and Implementations for Unit Test ==================*/
static OS_TIMER_s *test_timer;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    test_timer = TEST_OS_GetOsTimer();
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testOSTaskInitCallsFTSKFunctions(void) {
    FTSK_CreateQueues_Expect();
    FTSK_CreateTasks_Expect();

    OS_InitializeOperatingSystem();

    TEST_ASSERT_EQUAL_INT8(OS_INIT_PRE_OS, os_boot);
}

void testOS_IncrementTimer(void) {
    OS_TIMER_s testTimerExpected = {0u, 0u, 0u, 0u, 0u, 0u, 0u};

    OS_IncrementTimer();
    /* Set expected value */
    testTimerExpected.timer_1ms = 1u;
    TEST_ASSERT_EQUAL_MEMORY(&testTimerExpected, test_timer, sizeof(OS_TIMER_s));

    /* Call trigger function 9 more times -> 9ms + 1ms have passed: 10ms in total */
    for (uint8_t i = 0; i < 9; i++) {
        OS_IncrementTimer();
    }
    /* Set expected value */
    testTimerExpected.timer_1ms  = 0u;
    testTimerExpected.timer_10ms = 1u;
    TEST_ASSERT_EQUAL_MEMORY(&testTimerExpected, test_timer, sizeof(OS_TIMER_s));

    /* Set timer to 99ms and call trigger function one more time -> 100ms passed */
    test_timer->timer_1ms  = 9u;
    test_timer->timer_10ms = 9u;
    OS_IncrementTimer();
    /* Set extpected value */
    testTimerExpected.timer_1ms   = 0u;
    testTimerExpected.timer_10ms  = 0u;
    testTimerExpected.timer_100ms = 1u;
    TEST_ASSERT_EQUAL_MEMORY(&testTimerExpected, test_timer, sizeof(OS_TIMER_s));

    /* Set timer to 999ms and call trigger function one more time -> 1s passed */
    test_timer->timer_1ms   = 9u;
    test_timer->timer_10ms  = 9u;
    test_timer->timer_100ms = 9u;
    test_timer->timer_sec   = 0u;
    test_timer->timer_min   = 0u;
    test_timer->timer_h     = 0u;
    test_timer->timer_d     = 0u;
    OS_IncrementTimer();
    /* Set extpected value */
    testTimerExpected.timer_1ms   = 0u;
    testTimerExpected.timer_10ms  = 0u;
    testTimerExpected.timer_100ms = 0u;
    testTimerExpected.timer_sec   = 1u;
    testTimerExpected.timer_min   = 0u;
    testTimerExpected.timer_h     = 0u;
    testTimerExpected.timer_d     = 0u;
    TEST_ASSERT_EQUAL_MEMORY(&testTimerExpected, test_timer, sizeof(OS_TIMER_s));

    /* Set timer to 59.999s and call trigger function one more time -> 1min passed */
    test_timer->timer_1ms   = 9u;
    test_timer->timer_10ms  = 9u;
    test_timer->timer_100ms = 9u;
    test_timer->timer_sec   = 59u;
    test_timer->timer_min   = 0u;
    test_timer->timer_h     = 0u;
    test_timer->timer_d     = 0u;
    OS_IncrementTimer();
    /* Set extpected value */
    testTimerExpected.timer_1ms   = 0u;
    testTimerExpected.timer_10ms  = 0u;
    testTimerExpected.timer_100ms = 0u;
    testTimerExpected.timer_sec   = 0u;
    testTimerExpected.timer_min   = 1u;
    testTimerExpected.timer_h     = 0u;
    testTimerExpected.timer_d     = 0u;
    TEST_ASSERT_EQUAL_MEMORY(&testTimerExpected, test_timer, sizeof(OS_TIMER_s));

    /* Set timer to 59min 59.999s and call trigger function one more time -> 1h passed */
    test_timer->timer_1ms   = 9u;
    test_timer->timer_10ms  = 9u;
    test_timer->timer_100ms = 9u;
    test_timer->timer_sec   = 59u;
    test_timer->timer_min   = 59u;
    test_timer->timer_h     = 0u;
    test_timer->timer_d     = 0u;
    OS_IncrementTimer();
    /* Set extpected value */
    testTimerExpected.timer_1ms   = 0u;
    testTimerExpected.timer_10ms  = 0u;
    testTimerExpected.timer_100ms = 0u;
    testTimerExpected.timer_sec   = 0u;
    testTimerExpected.timer_min   = 0u;
    testTimerExpected.timer_h     = 1u;
    testTimerExpected.timer_d     = 0u;
    TEST_ASSERT_EQUAL_MEMORY(&testTimerExpected, test_timer, sizeof(OS_TIMER_s));

    /* Set timer to 59h 59min 59.999s and call trigger function one more time -> 1d passed */
    test_timer->timer_1ms   = 9u;
    test_timer->timer_10ms  = 9u;
    test_timer->timer_100ms = 9u;
    test_timer->timer_sec   = 59u;
    test_timer->timer_min   = 59u;
    test_timer->timer_h     = 23u;
    test_timer->timer_d     = 0u;
    OS_IncrementTimer();
    /* Set extpected value */
    testTimerExpected.timer_1ms   = 0u;
    testTimerExpected.timer_10ms  = 0u;
    testTimerExpected.timer_100ms = 0u;
    testTimerExpected.timer_sec   = 0u;
    testTimerExpected.timer_min   = 0u;
    testTimerExpected.timer_h     = 0u;
    testTimerExpected.timer_d     = 1u;
    TEST_ASSERT_EQUAL_MEMORY(&testTimerExpected, test_timer, sizeof(OS_TIMER_s));
}

void testOS_TriggerTimerOverflow(void) {
    /* checks whether the overflow of the timer is sanely handled. */
    OS_TIMER_s testTimerExpected = {0u, 0u, 0u, 0u, 0u, 0u, 0u};

    /* Set timer to last tick before overflow and call trigger function one more time */
    test_timer->timer_1ms   = 9u;
    test_timer->timer_10ms  = 9u;
    test_timer->timer_100ms = 9u;
    test_timer->timer_sec   = 59u;
    test_timer->timer_min   = 59u;
    test_timer->timer_h     = 23u;
    test_timer->timer_d     = UINT16_MAX;
    OS_IncrementTimer();
    /* Set extpected value */
    testTimerExpected.timer_1ms   = 0u;
    testTimerExpected.timer_10ms  = 0u;
    testTimerExpected.timer_100ms = 0u;
    testTimerExpected.timer_sec   = 0u;
    testTimerExpected.timer_min   = 0u;
    testTimerExpected.timer_h     = 0u;
    testTimerExpected.timer_d     = 0u;
    TEST_ASSERT_EQUAL_MEMORY(&testTimerExpected, test_timer, sizeof(OS_TIMER_s));
}

/** when no time shall pass, the result of #OS_CheckTimeHasPassed() will always be true, independent of the time */
void testOS_CheckTimeHasPassedNoTime(void) {
    xTaskGetTickCount_ExpectAndReturn(0u);
    TEST_ASSERT_EQUAL(true, OS_CheckTimeHasPassed(0u, 0u));

    xTaskGetTickCount_ExpectAndReturn(100u);
    TEST_ASSERT_EQUAL(true, OS_CheckTimeHasPassed(0u, 0u));

    xTaskGetTickCount_ExpectAndReturn(UINT32_MAX);
    TEST_ASSERT_EQUAL(true, OS_CheckTimeHasPassed(0u, 0u));

    xTaskGetTickCount_ExpectAndReturn(0u);
    TEST_ASSERT_EQUAL(true, OS_CheckTimeHasPassed(0u, 0u));

    xTaskGetTickCount_ExpectAndReturn(0u);
    TEST_ASSERT_EQUAL(true, OS_CheckTimeHasPassed(100u, 0u));

    xTaskGetTickCount_ExpectAndReturn(0u);
    TEST_ASSERT_EQUAL(true, OS_CheckTimeHasPassed(UINT32_MAX, 0u));
}

/** check for when 1ms shall pass with #OS_CheckTimeHasPassed() */
void testOS_CheckTimeHasPassed1ms(void) {
    xTaskGetTickCount_ExpectAndReturn(0u);
    TEST_ASSERT_EQUAL(false, OS_CheckTimeHasPassed(0u, 1u));

    xTaskGetTickCount_ExpectAndReturn(0u);
    TEST_ASSERT_EQUAL(true, OS_CheckTimeHasPassed(1u, 1u));
}

/** check behavior before the wraparound of the timestamp for when 1ms shall pass with #OS_CheckTimeHasPassed() */
void testOS_CheckTimeHasPassedTimestampAtMax1ms(void) {
    xTaskGetTickCount_ExpectAndReturn(UINT32_MAX);
    TEST_ASSERT_EQUAL(false, OS_CheckTimeHasPassed(UINT32_MAX, 1u));

    xTaskGetTickCount_ExpectAndReturn(UINT32_MAX);
    TEST_ASSERT_EQUAL(true, OS_CheckTimeHasPassed((UINT32_MAX - 1u), 1u));
}

/** check behavior around the wraparound of the timestamp for when 1ms shall pass with #OS_CheckTimeHasPassed() */
void testOS_CheckTimeHasPassedTimestampAroundMax1ms(void) {
    xTaskGetTickCount_ExpectAndReturn(0u);
    TEST_ASSERT_EQUAL(true, OS_CheckTimeHasPassed(UINT32_MAX, 1u));
}

/** check behavior for the largest time step possible in #OS_CheckTimeHasPassed() */
void testOS_CheckTimeHasPassedUINT32_MAXms(void) {
    /* edge case: it is to be assumed that here rather no time has passed */
    xTaskGetTickCount_ExpectAndReturn(0u);
    TEST_ASSERT_EQUAL(false, OS_CheckTimeHasPassed(0u, UINT32_MAX));

    xTaskGetTickCount_ExpectAndReturn(1u);
    TEST_ASSERT_EQUAL(false, OS_CheckTimeHasPassed(0u, UINT32_MAX));

    xTaskGetTickCount_ExpectAndReturn(UINT32_MAX);
    TEST_ASSERT_EQUAL(true, OS_CheckTimeHasPassed(0u, UINT32_MAX));
}

/** check that the selftest passes */
void testOS_CheckTimeHasPassedSelfTestSuccessful(void) {
    TEST_ASSERT_EQUAL(STD_OK, OS_CheckTimeHasPassedSelfTest());
}
