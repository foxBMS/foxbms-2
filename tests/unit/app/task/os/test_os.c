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
 * @updated 2021-07-23 (date of last update)
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
#include "Mocktask.h"

#include "os.h"

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
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

void testvApplicationIdleHookCallsUserCodeIdle(void) {
    FTSK_RunUserCodeIdle_Expect();

    vApplicationIdleHook();
}

void testOS_TriggerTimer(void) {
    OS_TIMER_s timer         = {0};
    OS_TIMER_s timerExpected = {0};

    /* First call of function -> expected value afterwards 1ms has passed */
    OS_TriggerTimer(&timer);
    timerExpected.timer_1ms = 1; /* Set expected value */
    TEST_ASSERT_EQUAL_MEMORY(&timerExpected, &timer, sizeof(OS_TIMER_s));

    /* Call trigger function 9 more times -> 9ms + 1ms have passed: 10ms in total */
    for (uint8_t i = 0; i < 9; i++) {
        OS_TriggerTimer(&timer);
    }
    timerExpected.timer_1ms  = 0; /* Set expected value */
    timerExpected.timer_10ms = 1;
    TEST_ASSERT_EQUAL_MEMORY(&timerExpected, &timer, sizeof(OS_TIMER_s));

    /* Set timer to 99ms and call trigger function one more time -> 100ms passed */
    timer.timer_1ms  = 9;
    timer.timer_10ms = 9;
    OS_TriggerTimer(&timer);
    timerExpected.timer_1ms   = 0; /* Set extpected value */
    timerExpected.timer_10ms  = 0;
    timerExpected.timer_100ms = 1;
    TEST_ASSERT_EQUAL_MEMORY(&timerExpected, &timer, sizeof(OS_TIMER_s));

    /* Set timer to 999ms and call trigger function one more time -> 1s passed */
    timer.timer_1ms   = 9;
    timer.timer_10ms  = 9;
    timer.timer_100ms = 9;
    timer.timer_sec   = 0;
    timer.timer_min   = 0;
    timer.timer_h     = 0;
    timer.timer_d     = 0;
    OS_TriggerTimer(&timer);
    timerExpected.timer_1ms   = 0; /* Set extpected value */
    timerExpected.timer_10ms  = 0;
    timerExpected.timer_100ms = 0;
    timerExpected.timer_sec   = 1;
    timerExpected.timer_min   = 0;
    timerExpected.timer_h     = 0;
    timerExpected.timer_d     = 0;
    TEST_ASSERT_EQUAL_MEMORY(&timerExpected, &timer, sizeof(OS_TIMER_s));

    /* Set timer to 59.999s and call trigger function one more time -> 1min passed */
    timer.timer_1ms   = 9;
    timer.timer_10ms  = 9;
    timer.timer_100ms = 9;
    timer.timer_sec   = 59;
    timer.timer_min   = 0;
    timer.timer_h     = 0;
    timer.timer_d     = 0;
    OS_TriggerTimer(&timer);
    timerExpected.timer_1ms   = 0; /* Set extpected value */
    timerExpected.timer_10ms  = 0;
    timerExpected.timer_100ms = 0;
    timerExpected.timer_sec   = 0;
    timerExpected.timer_min   = 1;
    timerExpected.timer_h     = 0;
    timerExpected.timer_d     = 0;
    TEST_ASSERT_EQUAL_MEMORY(&timerExpected, &timer, sizeof(OS_TIMER_s));

    /* Set timer to 59min 59.999s and call trigger function one more time -> 1h passed */
    timer.timer_1ms   = 9;
    timer.timer_10ms  = 9;
    timer.timer_100ms = 9;
    timer.timer_sec   = 59;
    timer.timer_min   = 59;
    timer.timer_h     = 0;
    timer.timer_d     = 0;
    OS_TriggerTimer(&timer);
    timerExpected.timer_1ms   = 0; /* Set extpected value */
    timerExpected.timer_10ms  = 0;
    timerExpected.timer_100ms = 0;
    timerExpected.timer_sec   = 0;
    timerExpected.timer_min   = 0;
    timerExpected.timer_h     = 1;
    timerExpected.timer_d     = 0;
    TEST_ASSERT_EQUAL_MEMORY(&timerExpected, &timer, sizeof(OS_TIMER_s));

    /* Set timer to 59h 59min 59.999s and call trigger function one more time -> 1d passed */
    timer.timer_1ms   = 9;
    timer.timer_10ms  = 9;
    timer.timer_100ms = 9;
    timer.timer_sec   = 59;
    timer.timer_min   = 59;
    timer.timer_h     = 59;
    timer.timer_d     = 0;
    OS_TriggerTimer(&timer);
    timerExpected.timer_1ms   = 0; /* Set extpected value */
    timerExpected.timer_10ms  = 0;
    timerExpected.timer_100ms = 0;
    timerExpected.timer_sec   = 0;
    timerExpected.timer_min   = 0;
    timerExpected.timer_h     = 0;
    timerExpected.timer_d     = 1;
    TEST_ASSERT_EQUAL_MEMORY(&timerExpected, &timer, sizeof(OS_TIMER_s));
}

void testOS_TriggerTimerOverflow(void) {
    /* checks whether the overflow of the timer is sanely handled. */
    OS_TIMER_s timer         = {0};
    OS_TIMER_s timerExpected = {0};

    /* Set timer to last tick before overflow and call trigger function one more time */
    timer.timer_1ms   = 9;
    timer.timer_10ms  = 9;
    timer.timer_100ms = 9;
    timer.timer_sec   = 59;
    timer.timer_min   = 59;
    timer.timer_h     = 23;
    timer.timer_d     = UINT16_MAX;
    OS_TriggerTimer(&timer);
    timerExpected.timer_1ms   = 0; /* Set extpected value */
    timerExpected.timer_10ms  = 0;
    timerExpected.timer_100ms = 0;
    timerExpected.timer_sec   = 0;
    timerExpected.timer_min   = 0;
    timerExpected.timer_h     = 0;
    timerExpected.timer_d     = 0;
    TEST_ASSERT_EQUAL_MEMORY(&timerExpected, &timer, sizeof(OS_TIMER_s));
}
