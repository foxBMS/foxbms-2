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
 * @file    test_os.c
 * @author  foxBMS Team
 * @date    2020-03-13 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  OS
 *
 * @brief   Test of the os.c module
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockHL_sys_core.h"
#include "Mockcan.h"
#include "Mockcan_helper.h"
#include "Mockftask.h"
#include "Mockftask_cfg.h"
#include "Mockportmacro.h"
#include "Mockqueue.h"
#include "Mockrtc.h"
#include "Mocktask.h"

#include "can_cfg.h"

#include "can_cbs_tx_crash-dump.h"
#include "os.h"
#include "test_assert_helper.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("os.c")
TEST_SOURCE_FILE("os_freertos.c")

TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/tx-async")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/
static OS_TIMER_s *test_timer;
OS_QUEUE ftsk_imdCanDataQueue = NULL_PTR;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    test_timer = TEST_OS_GetOsTimer();
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testOSTaskInitCallsFTSKFunctions(void) {
    _cacheDisable__Expect();
    FTSK_CreateQueues_Expect();
    FTSK_CreateTasks_Expect();

    OS_InitializeOperatingSystem();

    TEST_ASSERT_EQUAL_INT8(OS_INIT_PRE_OS, os_boot);
}

void testOS_IncrementTimer(void) {
    OS_TIMER_s testTimerExpected = {0u, 0u, 0u, 0u, 0u, 0u, 0u};

    RTC_IncrementSystemTime_Expect();
    OS_IncrementTimer();
    /* Set expected value */
    testTimerExpected.timer_1ms = 1u;
    TEST_ASSERT_EQUAL_MEMORY(&testTimerExpected, test_timer, sizeof(OS_TIMER_s));

    /* Call trigger function 9 more times -> 9ms + 1ms have passed: 10ms in total */
    for (uint8_t i = 0; i < 9; i++) {
        RTC_IncrementSystemTime_Expect();
        OS_IncrementTimer();
    }
    /* Set expected value */
    testTimerExpected.timer_1ms  = 0u;
    testTimerExpected.timer_10ms = 1u;
    TEST_ASSERT_EQUAL_MEMORY(&testTimerExpected, test_timer, sizeof(OS_TIMER_s));

    /* Set timer to 99ms and call trigger function one more time -> 100ms passed */
    test_timer->timer_1ms  = 9u;
    test_timer->timer_10ms = 9u;
    RTC_IncrementSystemTime_Expect();
    OS_IncrementTimer();
    /* Set expected value */
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
    RTC_IncrementSystemTime_Expect();
    OS_IncrementTimer();
    /* Set expected value */
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
    RTC_IncrementSystemTime_Expect();
    OS_IncrementTimer();
    /* Set expected value */
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
    RTC_IncrementSystemTime_Expect();
    OS_IncrementTimer();
    /* Set expected value */
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
    RTC_IncrementSystemTime_Expect();
    OS_IncrementTimer();
    /* Set expected value */
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
    RTC_IncrementSystemTime_Expect();
    OS_IncrementTimer();
    /* Set expected value */
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
