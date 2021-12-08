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
 * @file    test_sys_mon.c
 * @author  foxBMS Team
 * @date    2020-04-02 (date of creation)
 * @updated 2020-05-28 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the sys_mon module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdiag.h"
#include "Mockos.h"
#include "Mocksys_mon_cfg.h"

#include "fassert.h"
#include "sys_mon.h"
#include "test_assert_helper.h"

/*========== Definitions and Implementations for Unit Test ==================*/
#define DUMMY_TASK_ID_0  0
#define DUMMY_TASK_ID_1  1
#define DUMMY_CYCLETIME  10
#define DUMMY_MAX_JITTER 1

void TEST_SYSM_DummyCallback_0(SYSM_TASK_ID_e taskId) {
    TEST_ASSERT_EQUAL(DUMMY_TASK_ID_0, taskId);
}

void TEST_SYSM_DummyCallback_1(SYSM_TASK_ID_e taskId) {
    TEST_ASSERT_EQUAL(DUMMY_TASK_ID_1, taskId);
}

SYSM_MONITORING_CFG_s sysm_ch_cfg[2] = {
    {DUMMY_TASK_ID_0,
     SYSM_ENABLED,
     DUMMY_CYCLETIME,
     DUMMY_MAX_JITTER,
     SYSM_RECORDING_ENABLED,
     SYSM_HANDLING_SWITCHOFFCONTACTOR,
     TEST_SYSM_DummyCallback_0},
    {DUMMY_TASK_ID_1,
     SYSM_DISABLED,
     DUMMY_CYCLETIME,
     DUMMY_MAX_JITTER,
     SYSM_RECORDING_ENABLED,
     SYSM_HANDLING_SWITCHOFFCONTACTOR,
     TEST_SYSM_DummyCallback_1},
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    SYSM_NOTIFICATION_s *notifications            = TEST_SYSM_GetNotifications();
    notifications[DUMMY_TASK_ID_0].timestampEnter = 0;
    notifications[DUMMY_TASK_ID_0].timestampExit  = 0;
    notifications[DUMMY_TASK_ID_0].duration       = 0;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testSYSM_CheckNotificationsEarlyExitOnTimestampEquality(void) {
    /* Internal timestamp should be 0, expect an early return
       (if not returning early, test would fail since other functions
       are called; also make sure that the code path that would call the
       DIAG_Handler would be hit if not returning early) */
    SYSM_NOTIFICATION_s *notifications            = TEST_SYSM_GetNotifications();
    notifications[DUMMY_TASK_ID_0].timestampEnter = 0;
    notifications[DUMMY_TASK_ID_0].timestampExit  = 100;
    notifications[DUMMY_TASK_ID_0].duration       = 100;
    OS_GetTickCount_ExpectAndReturn(0u);
    SYSM_CheckNotifications();
}

void testSYSM_CheckNotificationsSYSMDisabled(void) {
    /* call a task with Monitoring disabled */
    OS_GetTickCount_ExpectAndReturn(0u);
    SYSM_CheckNotifications();

    OS_GetTickCount_ExpectAndReturn(100u);
    SYSM_CheckNotifications();
}

void testSYSM_CheckNotificationsProvokeDurationViolation(void) {
    /* provoke the violation of the task duration */
    OS_GetTickCount_ExpectAndReturn(0u);
    SYSM_CheckNotifications();

    SYSM_NOTIFICATION_s *notifications            = TEST_SYSM_GetNotifications();
    notifications[DUMMY_TASK_ID_0].timestampEnter = 0;
    notifications[DUMMY_TASK_ID_0].timestampExit  = 100;
    notifications[DUMMY_TASK_ID_0].duration       = 100;

    OS_GetTickCount_ExpectAndReturn(100u);
    DIAG_Handler_ExpectAndReturn(
        DIAG_ID_SYSTEMMONITORING, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, DUMMY_TASK_ID_0, DIAG_HANDLER_RETURN_OK);
    SYSM_CheckNotifications();
}

void testSYSM_NotifyInvalidTaskID(void) {
    /* give an invalid Task ID to Notify */
    TEST_ASSERT_PASS_ASSERT(SYSM_Notify(SYSM_TASK_ID_MAX + 1u, SYSM_NOTIFY_ENTER, 424242));
    SYSM_NOTIFICATION_s *notifications = TEST_SYSM_GetNotifications();
    TEST_ASSERT_NOT_EQUAL(424242, notifications[SYSM_TASK_ID_MAX + 1u].timestampEnter);
}

void testSYSM_NotifyEnterTimestampProperlySet(void) {
    /* check whether Notify properly sets the entry timestamp */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_ASSERT_PASS_ASSERT(SYSM_Notify(DUMMY_TASK_ID_0, SYSM_NOTIFY_ENTER, UINT32_MAX));
    SYSM_NOTIFICATION_s *notifications = TEST_SYSM_GetNotifications();
    TEST_ASSERT_EQUAL(UINT32_MAX, notifications[DUMMY_TASK_ID_0].timestampEnter);

    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_ASSERT_PASS_ASSERT(SYSM_Notify(DUMMY_TASK_ID_0, SYSM_NOTIFY_ENTER, 0));
    notifications = TEST_SYSM_GetNotifications();
    TEST_ASSERT_EQUAL(0, notifications[DUMMY_TASK_ID_0].timestampEnter);
}

void testSYSM_NotifyExitTimestampProperlySetAndDurationCalculated(void) {
    /* check whether Notify properly sets the exit timestamp and computes duration */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_ASSERT_PASS_ASSERT(SYSM_Notify(DUMMY_TASK_ID_0, SYSM_NOTIFY_ENTER, UINT32_MAX));
    SYSM_NOTIFICATION_s *notifications = TEST_SYSM_GetNotifications();
    TEST_ASSERT_EQUAL(UINT32_MAX, notifications[DUMMY_TASK_ID_0].timestampEnter);

    const uint32 exitTime = 100;
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_ASSERT_PASS_ASSERT(SYSM_Notify(DUMMY_TASK_ID_0, SYSM_NOTIFY_EXIT, exitTime));
    notifications = TEST_SYSM_GetNotifications();
    TEST_ASSERT_EQUAL(exitTime, notifications[DUMMY_TASK_ID_0].timestampExit);
    /* we go over the overflow, so it should be exit time plus one */
    TEST_ASSERT_EQUAL(exitTime + 1, notifications[DUMMY_TASK_ID_0].duration);
}

void testSYSM_NotifyHitAssertWithIllegalNotifyType(void) {
    /* This test hits the assert with an illegal notify type */
    OS_EnterTaskCritical_Ignore();
    /* use a notify type of INT8_MAX, as this is likely an illegal notify type */
    TEST_ASSERT_FAIL_ASSERT(SYSM_Notify(DUMMY_TASK_ID_0, INT8_MAX, UINT32_MAX));
    SYSM_NOTIFICATION_s *notifications = TEST_SYSM_GetNotifications();
    /* check that nothing has been written */
    TEST_ASSERT_NOT_EQUAL(UINT32_MAX, notifications[DUMMY_TASK_ID_0].timestampEnter);
    TEST_ASSERT_NOT_EQUAL(UINT32_MAX, notifications[DUMMY_TASK_ID_0].timestampExit);
}
