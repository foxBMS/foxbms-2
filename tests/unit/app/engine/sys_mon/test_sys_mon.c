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
 * @file    test_sys_mon.c
 * @author  foxBMS Team
 * @date    2020-04-02 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the System Monitoring module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdiag.h"
#include "Mockfram.h"
#include "Mockfram_cfg.h"
#include "Mockos.h"
#include "Mocksys_mon_cfg.h"

#include "fassert.h"
#include "sys_mon.h"
#include "test_assert_helper.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/fram")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/engine/sys_mon")

/*========== Definitions and Implementations for Unit Test ==================*/
#define DUMMY_TASK_ID_0  (0)
#define DUMMY_TASK_ID_1  (1)
#define DUMMY_TASK_ID_2  (2)
#define DUMMY_CYCLE_TIME (10)
#define DUMMY_MAX_JITTER (1)

void TEST_SYSM_DummyCallback_0(SYSM_TASK_ID_e taskId) {
    TEST_ASSERT_EQUAL(DUMMY_TASK_ID_0, taskId);
}

void TEST_SYSM_DummyCallback_1(SYSM_TASK_ID_e taskId) {
    TEST_ASSERT_EQUAL(DUMMY_TASK_ID_1, taskId);
}

void TEST_SYSM_DummyCallback_2(SYSM_TASK_ID_e taskId) {
    TEST_ASSERT_EQUAL(DUMMY_TASK_ID_2, taskId);
}

SYSM_MONITORING_CFG_s sysm_ch_cfg[3] = {
    {DUMMY_TASK_ID_0,
     SYSM_ENABLED,
     DUMMY_CYCLE_TIME,
     DUMMY_MAX_JITTER,
     SYSM_RECORDING_ENABLED,
     SYSM_HANDLING_SWITCH_OFF_CONTACTOR,
     TEST_SYSM_DummyCallback_0},
    {DUMMY_TASK_ID_1,
     SYSM_DISABLED,
     DUMMY_CYCLE_TIME,
     DUMMY_MAX_JITTER,
     SYSM_RECORDING_ENABLED,
     SYSM_HANDLING_SWITCH_OFF_CONTACTOR,
     TEST_SYSM_DummyCallback_1},
    {DUMMY_TASK_ID_2,
     SYSM_ENABLED,
     DUMMY_CYCLE_TIME,
     DUMMY_MAX_JITTER,
     SYSM_RECORDING_DISABLED,
     SYSM_HANDLING_SWITCH_OFF_CONTACTOR,
     TEST_SYSM_DummyCallback_2},
};

/** placeholder variable for the FRAM entry of sys mon */
FRAM_SYS_MON_RECORD_s fram_sysMonViolationRecord = {false, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    SYSM_NOTIFICATION_s *notifications            = TEST_SYSM_GetNotifications();
    notifications[DUMMY_TASK_ID_0].timestampEnter = 0;
    notifications[DUMMY_TASK_ID_0].timestampExit  = 0;
    notifications[DUMMY_TASK_ID_0].duration       = 0;
    notifications[DUMMY_TASK_ID_1].timestampEnter = 0;
    notifications[DUMMY_TASK_ID_1].timestampExit  = 0;
    notifications[DUMMY_TASK_ID_1].duration       = 0;
    notifications[DUMMY_TASK_ID_2].timestampEnter = 0;
    notifications[DUMMY_TASK_ID_2].timestampExit  = 0;
    notifications[DUMMY_TASK_ID_2].duration       = 0;

    fram_sysMonViolationRecord.anyTimingIssueOccurred              = false;
    fram_sysMonViolationRecord.taskEngineViolatingDuration         = 0u;
    fram_sysMonViolationRecord.taskEngineEnterTimestamp            = 0u;
    fram_sysMonViolationRecord.task1msViolatingDuration            = 0u;
    fram_sysMonViolationRecord.task1msEnterTimestamp               = 0u;
    fram_sysMonViolationRecord.task10msViolatingDuration           = 0u;
    fram_sysMonViolationRecord.task10msEnterTimestamp              = 0u;
    fram_sysMonViolationRecord.task100msViolatingDuration          = 0u;
    fram_sysMonViolationRecord.task100msEnterTimestamp             = 0u;
    fram_sysMonViolationRecord.task100msAlgorithmViolatingDuration = 0u;
    fram_sysMonViolationRecord.task100msAlgorithmEnterTimestamp    = 0u;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testSYSM_RecordTimingViolation(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_SYSM_RecordTimingViolation(SYSM_TASK_ID_MAX, 0u, 0u));
    TEST_SYSM_RecordTimingViolation(SYSM_TASK_ID_ENGINE, 0, 0);
    TEST_SYSM_RecordTimingViolation(SYSM_TASK_ID_CYCLIC_1ms, 0, 0);
    TEST_SYSM_RecordTimingViolation(SYSM_TASK_ID_CYCLIC_10ms, 0, 0);
    TEST_SYSM_RecordTimingViolation(SYSM_TASK_ID_CYCLIC_100ms, 0, 0);
    TEST_SYSM_RecordTimingViolation(SYSM_TASK_ID_CYCLIC_ALGORITHM_100ms, 0, 0);
    TEST_SYSM_RecordTimingViolation(-1, 0, 0);
}

void testSYSM_Initialize(void) {
    FRAM_ReadData_ExpectAndReturn(FRAM_BLOCK_ID_SYS_MON_RECORD, FRAM_ACCESS_OK);
    SYSM_Initialize();
}

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
        DIAG_ID_SYSTEM_MONITORING, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, DUMMY_TASK_ID_0, DIAG_HANDLER_RETURN_OK);
    SYSM_CheckNotifications();
}

/** same test as #testSYSM_CheckNotificationsProvokeDurationViolation() but with recording enabled */
void testSYSM_CheckNotificationsProvokeDurationViolationWithRecording(void) {
    /* provoke the violation of the task duration */
    OS_GetTickCount_ExpectAndReturn(0u);
    SYSM_CheckNotifications();

    SYSM_NOTIFICATION_s *notifications            = TEST_SYSM_GetNotifications();
    notifications[DUMMY_TASK_ID_0].timestampEnter = 0;
    notifications[DUMMY_TASK_ID_0].timestampExit  = 100;
    notifications[DUMMY_TASK_ID_0].duration       = 100;

    OS_GetTickCount_ExpectAndReturn(100u);
    DIAG_Handler_ExpectAndReturn(
        DIAG_ID_SYSTEM_MONITORING, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, DUMMY_TASK_ID_0, DIAG_HANDLER_RETURN_OK);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    SYSM_CheckNotifications();
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    /* check if violation has been recorded */
    FRAM_WriteData_ExpectAndReturn(FRAM_BLOCK_ID_SYS_MON_RECORD, STD_OK);
    SYSM_UpdateFramData();
    TEST_ASSERT_EQUAL(true, fram_sysMonViolationRecord.anyTimingIssueOccurred);
}

void testSYSM_NotifyInvalidTaskID(void) {
    /* give an invalid Task ID to Notify */
    TEST_ASSERT_FAIL_ASSERT(SYSM_Notify(SYSM_TASK_ID_MAX + 1u, SYSM_NOTIFY_ENTER, 424242));
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

    const uint32_t exitTime = 100;
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
    /* use a notify type of INT8_MAX, as this is likely an illegal notify type */
    TEST_ASSERT_FAIL_ASSERT(SYSM_Notify(DUMMY_TASK_ID_0, INT8_MAX, UINT32_MAX));
    SYSM_NOTIFICATION_s *notifications = TEST_SYSM_GetNotifications();
    /* check that nothing has been written */
    TEST_ASSERT_NOT_EQUAL(UINT32_MAX, notifications[DUMMY_TASK_ID_0].timestampEnter);
    TEST_ASSERT_NOT_EQUAL(UINT32_MAX, notifications[DUMMY_TASK_ID_0].timestampExit);
}

/**
 * @brief   Testing extern function #SYSM_GetRecordedTimingViolations
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for pAnswer &rarr; assert
 *          - Routine validation:
 *            - RT1/2:
 *            - RT2/2:
 */
void testSYSM_GetRecordedTimingViolations(void) {
    /* ======= AT1/2: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(SYSM_GetRecordedTimingViolations(NULL_PTR));

    /* ======= Routine tests =============================================== */
    SYSM_TIMING_VIOLATION_RESPONSE_s violationResponse = {0};
    /* ======= RT1/2: Test implementation */
    /* default state is no violation, therefore no flag should be set */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    /* ======= RT1/2: call function under test */
    SYSM_GetRecordedTimingViolations(&violationResponse);

    /* ======= RT1/2: test output verification */
    TEST_ASSERT_FALSE(violationResponse.recordedViolationEngine);
    TEST_ASSERT_FALSE(violationResponse.recordedViolation1ms);
    TEST_ASSERT_FALSE(violationResponse.recordedViolation10ms);
    TEST_ASSERT_FALSE(violationResponse.recordedViolation100ms);
    TEST_ASSERT_FALSE(violationResponse.recordedViolation100msAlgo);

    /* ======= RT2/2: Test implementation */
    /* when the general flag is set and one deviates in duration or entry,
     * the violation should be set */
    fram_sysMonViolationRecord.anyTimingIssueOccurred   = true;
    fram_sysMonViolationRecord.taskEngineEnterTimestamp = 100u;
    fram_sysMonViolationRecord.task1msViolatingDuration = 5u;

    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    /* ======= RT1/2: call function under test */
    SYSM_GetRecordedTimingViolations(&violationResponse);

    /* ======= RT1/2: test output verification */
    TEST_ASSERT_TRUE(violationResponse.recordedViolationAny);
    TEST_ASSERT_TRUE(violationResponse.recordedViolationEngine);
    TEST_ASSERT_TRUE(violationResponse.recordedViolation1ms);
    TEST_ASSERT_FALSE(violationResponse.recordedViolation10ms);
    TEST_ASSERT_FALSE(violationResponse.recordedViolation100ms);
    TEST_ASSERT_FALSE(violationResponse.recordedViolation100msAlgo);
}

/**
 * @brief   Testing extern function #SYSM_ClearAllTimingViolations
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - none (void function)
 *          - Routine validation:
 *            - RT1/2: all timing violations records shall be cleared
 */
void testSYSM_ClearAllTimingViolations(void) {
    /* ======= Assertion tests ============================================= */
    /* none */

    /* ======= Routine tests =============================================== */
    fram_sysMonViolationRecord.anyTimingIssueOccurred              = true;
    fram_sysMonViolationRecord.taskEngineEnterTimestamp            = 1u;
    fram_sysMonViolationRecord.taskEngineViolatingDuration         = 2u;
    fram_sysMonViolationRecord.task1msEnterTimestamp               = 3u;
    fram_sysMonViolationRecord.task1msViolatingDuration            = 4u;
    fram_sysMonViolationRecord.task10msEnterTimestamp              = 5u;
    fram_sysMonViolationRecord.task10msViolatingDuration           = 6u;
    fram_sysMonViolationRecord.task100msEnterTimestamp             = 7u;
    fram_sysMonViolationRecord.task100msViolatingDuration          = 8u;
    fram_sysMonViolationRecord.task100msAlgorithmEnterTimestamp    = 9u;
    fram_sysMonViolationRecord.task100msAlgorithmViolatingDuration = 10u;

    /* ======= RT1/1: Test implementation */
    for (SYSM_TASK_ID_e taskId = (SYSM_TASK_ID_e)0; taskId < SYSM_TASK_ID_MAX; taskId++) {
        (void)DIAG_Handler_ExpectAndReturn(
            DIAG_ID_SYSTEM_MONITORING, DIAG_EVENT_OK, DIAG_SYSTEM, (uint32_t)taskId, STD_OK);
    }
    OS_EnterTaskCritical_Expect(); /* in SYSM_ClearAllTimingViolations */
    OS_ExitTaskCritical_Expect();  /* in SYSM_ClearAllTimingViolations */

    OS_EnterTaskCritical_Expect(); /* in SYSM_UpdateFramData */
    OS_ExitTaskCritical_Expect();  /* in SYSM_UpdateFramData */
    OS_EnterTaskCritical_Expect(); /* in SYSM_UpdateFramData: in branch */
    OS_ExitTaskCritical_Expect();  /* in SYSM_UpdateFramData: in branch */

    FRAM_WriteData_ExpectAndReturn(FRAM_BLOCK_ID_SYS_MON_RECORD, STD_OK);

    /* the called function alters a module static variable;
     * get the value to restore it */
    bool oldValue = TEST_SYSM_GetStaticVariableFlagFramCopyHasChanges();

    /* ======= RT1/1: call function under test */
    SYSM_ClearAllTimingViolations();

    /* ======= RT1/1: test output verification */
    TEST_ASSERT_FALSE(fram_sysMonViolationRecord.anyTimingIssueOccurred);
    TEST_ASSERT_EQUAL_UINT32(fram_sysMonViolationRecord.taskEngineEnterTimestamp, 0u);
    TEST_ASSERT_EQUAL_UINT32(fram_sysMonViolationRecord.taskEngineViolatingDuration, 0u);
    TEST_ASSERT_EQUAL_UINT32(fram_sysMonViolationRecord.task1msEnterTimestamp, 0u);
    TEST_ASSERT_EQUAL_UINT32(fram_sysMonViolationRecord.task1msViolatingDuration, 0u);
    TEST_ASSERT_EQUAL_UINT32(fram_sysMonViolationRecord.task10msEnterTimestamp, 0u);
    TEST_ASSERT_EQUAL_UINT32(fram_sysMonViolationRecord.task10msViolatingDuration, 0u);
    TEST_ASSERT_EQUAL_UINT32(fram_sysMonViolationRecord.task100msEnterTimestamp, 0u);
    TEST_ASSERT_EQUAL_UINT32(fram_sysMonViolationRecord.task100msViolatingDuration, 0u);
    TEST_ASSERT_EQUAL_UINT32(fram_sysMonViolationRecord.task100msAlgorithmEnterTimestamp, 0u);
    TEST_ASSERT_EQUAL_UINT32(fram_sysMonViolationRecord.task100msAlgorithmViolatingDuration, 0u);

    /* restore old value */
    (void)TEST_SYSM_SetStaticVariableFlagFramCopyHasChanges(oldValue);
}

/**
 * @brief   Testing extern function #SYSM_UpdateFramData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - none (void function)
 *          - Routine validation:
 *            - RT1/2: no update are required, i.e., nothing to do.
 *            - RT2/2: an update is due, check that the updated values get
 *                     copied.
 */
void testSYSM_UpdateFramData(void) {
    /* ======= Assertion tests ============================================= */
    /* none */

    /* ======= Routine tests =============================================== */
    /* ensure the test is meaningful, i.e., input and output are not equal */

    /* ======= RT1/2: Test implementation */
    /* store the old value, so that we can reset the variable after the test */
    bool oldValue = TEST_SYSM_SetStaticVariableFlagFramCopyHasChanges(false);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    /* ======= RT1/2: call function under test */

    SYSM_UpdateFramData();
    /* ======= RT1/2: test output verification */
    /* nothing shall be changed */
    /* TODO: check memory */

    /* restore old value */
    (void)TEST_SYSM_SetStaticVariableFlagFramCopyHasChanges(oldValue);

    /* ======= RT2/2: Test implementation */
    /* store the old value, so that we can reset the variable after the test */
    oldValue = TEST_SYSM_SetStaticVariableFlagFramCopyHasChanges(true);

    /* ======= RT1/2: call function under test */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    /* we do not need the information, whether FRAM_WriteData was successful or not */
    (void)FRAM_WriteData_ExpectAndReturn(FRAM_BLOCK_ID_SYS_MON_RECORD, STD_OK);

    SYSM_UpdateFramData();
    /* ======= RT1/2: test output verification */
    /* values should have been changed */
    /* TODO: check memory */

    /* restore old value */
    (void)TEST_SYSM_SetStaticVariableFlagFramCopyHasChanges(oldValue);
}

/**
 * @brief   Testing extern function #SYSM_CopyFramStruct
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: NULL_PTR for kpkFrom &rarr; assert
 *            - AT2/2: NULL_PTR for pTo &rarr; assert
 *          - Routine validation:
 *            - RT1/1: all elements get copied.
 */
void testSYSM_CopyFramStruct(void) {
    /* ======= Assertion tests ============================================= */
    FRAM_SYS_MON_RECORD_s assertionDummy = {
        .anyTimingIssueOccurred              = false,
        .task100msAlgorithmEnterTimestamp    = 0u,
        .task100msAlgorithmViolatingDuration = 0u,
        .task100msEnterTimestamp             = 0u,
        .task100msViolatingDuration          = 0u,
        .task10msEnterTimestamp              = 0u,
        .task10msViolatingDuration           = 0u,
        .task1msEnterTimestamp               = 0u,
        .task1msViolatingDuration            = 0u,
        .taskEngineEnterTimestamp            = 0u,
        .taskEngineViolatingDuration         = 0u,
    };
    /* ======= AT1/2: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(SYSM_CopyFramStruct(NULL_PTR, &assertionDummy));
    /* ======= AT1/2: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(SYSM_CopyFramStruct(&assertionDummy, NULL_PTR));

    /* ======= Routine tests =============================================== */
    FRAM_SYS_MON_RECORD_s input = {
        .anyTimingIssueOccurred              = true,
        .task100msAlgorithmEnterTimestamp    = 1u,
        .task100msAlgorithmViolatingDuration = 2u,
        .task100msEnterTimestamp             = 3u,
        .task100msViolatingDuration          = 4u,
        .task10msEnterTimestamp              = 5u,
        .task10msViolatingDuration           = 6u,
        .task1msEnterTimestamp               = 7u,
        .task1msViolatingDuration            = 8u,
        .taskEngineEnterTimestamp            = 9u,
        .taskEngineViolatingDuration         = 10u,
    };
    FRAM_SYS_MON_RECORD_s output = {
        .anyTimingIssueOccurred              = false,
        .task100msAlgorithmEnterTimestamp    = 0u,
        .task100msAlgorithmViolatingDuration = 0u,
        .task100msEnterTimestamp             = 0u,
        .task100msViolatingDuration          = 0u,
        .task10msEnterTimestamp              = 0u,
        .task10msViolatingDuration           = 0u,
        .task1msEnterTimestamp               = 0u,
        .task1msViolatingDuration            = 0u,
        .taskEngineEnterTimestamp            = 0u,
        .taskEngineViolatingDuration         = 0u,
    };

    /* ======= RT1/1: Test implementation */
    /* ensure the test is meaningful, i.e., input and output are not equal */
    TEST_ASSERT_NOT_EQUAL(input.anyTimingIssueOccurred, output.anyTimingIssueOccurred);

    /* ======= RT1/1: call function under test */
    SYSM_CopyFramStruct(&input, &output);

    /* ======= RT1/1: test output verification */
    TEST_ASSERT_EQUAL(input.anyTimingIssueOccurred, output.anyTimingIssueOccurred);
    TEST_ASSERT_EQUAL(input.task100msAlgorithmEnterTimestamp, output.task100msAlgorithmEnterTimestamp);
    TEST_ASSERT_EQUAL(input.task100msAlgorithmViolatingDuration, output.task100msAlgorithmViolatingDuration);
    TEST_ASSERT_EQUAL(input.task100msEnterTimestamp, output.task100msEnterTimestamp);
    TEST_ASSERT_EQUAL(input.task100msViolatingDuration, output.task100msViolatingDuration);
    TEST_ASSERT_EQUAL(input.task10msEnterTimestamp, output.task10msEnterTimestamp);
    TEST_ASSERT_EQUAL(input.task10msViolatingDuration, output.task10msViolatingDuration);
    TEST_ASSERT_EQUAL(input.task1msEnterTimestamp, output.task1msEnterTimestamp);
    TEST_ASSERT_EQUAL(input.task1msViolatingDuration, output.task1msViolatingDuration);
    TEST_ASSERT_EQUAL(input.taskEngineEnterTimestamp, output.taskEngineEnterTimestamp);
    TEST_ASSERT_EQUAL(input.taskEngineViolatingDuration, output.taskEngineViolatingDuration);
}
