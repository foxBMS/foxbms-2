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
 * @file    test_os_freertos.c
 * @author  foxBMS Team
 * @date    2021-11-26 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the OS implementation for FreeRTOS
 * @details Test functions:
 *          - testOS_StartScheduler
 *          - testvApplicationIdleHookCallsUserCodeIdle
 *          - testOS_MarkTaskAsRequiringFpuContext
 *          - testOS_GetNumberOfStoredMessagesInQueue
 *          - testOS_SendToBackOfQueueFromIsr
 *          - testOS_SendToBackOfQueue
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockHL_sys_core.h"
#include "Mockcan_cbs_tx_crash-dump.h"
#include "Mockftask.h"
#include "Mockftask_cfg.h"
#include "Mockportmacro.h"
#include "Mockqueue.h"
#include "Mockrtc.h"
#include "Mocktask.h"

#include "os.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("os_freertos.c")

TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/tx-async")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/

TaskHandle_t ftsk_testtaskHandle;
OS_QUEUE ftsk_testQueue;

/* Declaration for mocked test functions used */
void mock_vApplicationGetIdleTaskMemory(
    StaticTask_t **ppxIdleTaskTCBBuffer,
    StackType_t **ppxIdleTaskStackBuffer,
    configSTACK_DEPTH_TYPE *pulIdleTaskStackSize);
void mock_vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testOS_InitializeScheduler(void) {
    _cacheDisable__Expect();
    OS_InitializeScheduler();
}

/** test that #OS_StartScheduler calls the relevant FreeRTOS function */
void testOS_StartScheduler(void) {
    vTaskStartScheduler_Expect();
    TEST_ASSERT_FAIL_ASSERT(OS_StartScheduler());
}

#if (configUSE_TIMERS > 0) && (configSUPPORT_STATIC_ALLOCATION == 1)
void testvApplicationGetTimerTaskMemory(void) {
    StaticTask_t *ppxTimerTaskTCBBuffer  = NULL;
    StackType_t *ppxTimerTaskStackBuffer = NULL;
    configSTACK_DEPTH_TYPE pulTimerTaskStackSize;
    vApplicationGetTimerTaskMemory(&ppxTimerTaskTCBBuffer, &ppxTimerTaskStackBuffer, &pulTimerTaskStackSize);
}
#endif /* configUSE_TIMERS */

void testvApplicationGetIdleTaskMemory(void) {
    StaticTask_t *pxIdleTaskTCBBuffer  = NULL;
    StackType_t *pxIdleTaskStackBuffer = NULL;
    configSTACK_DEPTH_TYPE uxIdleTaskStackSize;
    /* Assertion tests */
    TEST_ASSERT_FAIL_ASSERT(vApplicationGetIdleTaskMemory(NULL_PTR, &pxIdleTaskStackBuffer, &uxIdleTaskStackSize));
    TEST_ASSERT_FAIL_ASSERT(vApplicationGetIdleTaskMemory(&pxIdleTaskTCBBuffer, NULL_PTR, &uxIdleTaskStackSize));
    TEST_ASSERT_FAIL_ASSERT(vApplicationGetIdleTaskMemory(&pxIdleTaskTCBBuffer, &pxIdleTaskStackBuffer, NULL_PTR));

    /* Unit test */
    vApplicationGetIdleTaskMemory(&pxIdleTaskTCBBuffer, &pxIdleTaskStackBuffer, &uxIdleTaskStackSize);
}

void testvApplicationIdleHookCallsUserCodeIdle(void) {
    FTSK_RunUserCodeIdle_Expect();
    vApplicationIdleHook();
}

#if (configCHECK_FOR_STACK_OVERFLOW > 0)
void testvApplicationStackOverflowHook(void) {
    TaskHandle_t *pxOverflowTask = NULL;
    char *pcOverflowTaskName     = "test-overflow";
    CANTX_CrashDump_Expect(CANTX_FATAL_ERRORS_ACTIONS_STACK_OVERFLOW);
    vApplicationStackOverflowHook((TaskHandle_t)pxOverflowTask, pcOverflowTaskName);
}
#endif /* configCHECK_FOR_STACK_OVERFLOW */

void testOS_EnterTaskCritical(void) {
    vPortEnterCritical_Expect();
    OS_EnterTaskCritical();
}

void testOS_ExitTaskCritical(void) {
    vPortExitCritical_Expect();
    OS_ExitTaskCritical();
}

void testOS_GetTickCount(void) {
    xTaskGetTickCount_ExpectAndReturn(0u);
    OS_GetTickCount();
}

void testOS_DelayTask(void) {
    TEST_ASSERT_FAIL_ASSERT(OS_DelayTask(0u));
    vTaskDelay_Expect((TickType_t)1u);

    OS_DelayTask(1u);
}

void testOS_DelayTaskUntil(void) {
    uint32_t pPreviousWakeTime = 0u;
    uint32_t milliseconds      = 1u;
    TEST_ASSERT_FAIL_ASSERT(OS_DelayTaskUntil(NULL_PTR, milliseconds));
    TEST_ASSERT_FAIL_ASSERT(OS_DelayTaskUntil(&pPreviousWakeTime, 0u));

    /* Test with valid delay */
    uint32_t ticks = (milliseconds / OS_TICK_RATE_MS);
    xTaskDelayUntil_ExpectAndReturn((TickType_t *)&pPreviousWakeTime, (TickType_t)ticks, 0u);
    OS_DelayTaskUntil(&pPreviousWakeTime, milliseconds);

    /* Testing if minimal delay is used */
    milliseconds = 0u;
    if ((uint32_t)ticks < 1u) {
        ticks = 1u; /* Minimum delay is 1 tick */
    }
    xTaskDelayUntil_ExpectAndReturn((TickType_t *)&pPreviousWakeTime, (TickType_t)ticks, 0u);
    OS_DelayTaskUntil(&pPreviousWakeTime, milliseconds);
}

void testOS_WaitForNotification(void) {
    uint32_t pNotifiedValue = 0u;
    uint32_t timeout        = 1u;
    TEST_ASSERT_FAIL_ASSERT(OS_WaitForNotification(NULL_PTR, timeout));

    /* Test function with Notification true */
    xTaskGenericNotifyWait_ExpectAndReturn(
        tskDEFAULT_INDEX_TO_NOTIFY, UINT32_MAX, UINT32_MAX, &pNotifiedValue, timeout, pdTRUE);
    OS_WaitForNotification(&pNotifiedValue, timeout);

    /* Test function with Notification false */
    xTaskGenericNotifyWait_ExpectAndReturn(
        tskDEFAULT_INDEX_TO_NOTIFY, UINT32_MAX, UINT32_MAX, &pNotifiedValue, timeout, pdFALSE);
    OS_WaitForNotification(&pNotifiedValue, timeout);
}

void testOS_NotifyFromIsr(void) {
    TaskHandle_t taskToNotify = ftsk_testtaskHandle;
    uint32_t notifiedValue    = 0u;
    TEST_ASSERT_FAIL_ASSERT(OS_NotifyFromIsr(NULL_PTR, notifiedValue));

    /* Test function with Notification true */
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskGenericNotifyFromISR_ExpectAndReturn(
        taskToNotify,
        tskDEFAULT_INDEX_TO_NOTIFY,
        notifiedValue,
        eSetValueWithOverwrite,
        NULL,
        &xHigherPriorityTaskWoken,
        pdTRUE);
    OS_NotifyFromIsr(taskToNotify, notifiedValue);

    /* Test function with Notification false */
    xTaskGenericNotifyFromISR_ExpectAndReturn(
        taskToNotify,
        tskDEFAULT_INDEX_TO_NOTIFY,
        notifiedValue,
        eSetValueWithOverwrite,
        NULL,
        &xHigherPriorityTaskWoken,
        pdFALSE);
    OS_NotifyFromIsr(taskToNotify, notifiedValue);
}

void testOS_WaitForNotificationIndexed(void) {
    uint32_t pNotifiedValue = 0u;
    uint32_t timeout        = 1u;
    TEST_ASSERT_FAIL_ASSERT(OS_WaitForNotificationIndexed(2u, NULL_PTR, timeout));

    /* Test function with Notification true */
    xTaskGenericNotifyWait_ExpectAndReturn(2u, UINT32_MAX, UINT32_MAX, &pNotifiedValue, timeout, pdTRUE);
    OS_WaitForNotificationIndexed(2u, &pNotifiedValue, timeout);

    /* Test function with Notification false */
    xTaskGenericNotifyWait_ExpectAndReturn(2u, UINT32_MAX, UINT32_MAX, &pNotifiedValue, timeout, pdFALSE);
    OS_WaitForNotificationIndexed(2u, &pNotifiedValue, timeout);
}

void testOS_NotifyIndexedFromIsr(void) {
    TaskHandle_t taskToNotify = NULL;
    uint32_t notifiedValue    = 0u;
    TEST_ASSERT_FAIL_ASSERT(OS_NotifyIndexedFromIsr(NULL_PTR, 2u, notifiedValue));

    /* Test function with Notification true */
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskGenericNotifyFromISR_ExpectAndReturn(
        taskToNotify, 2u, notifiedValue, eSetValueWithOverwrite, NULL, &xHigherPriorityTaskWoken, pdTRUE);
    OS_NotifyIndexedFromIsr(taskToNotify, 2u, notifiedValue);

    /* Test function with Notification false */
    xTaskGenericNotifyFromISR_ExpectAndReturn(
        taskToNotify, 2u, notifiedValue, eSetValueWithOverwrite, NULL, &xHigherPriorityTaskWoken, pdFALSE);
    OS_NotifyIndexedFromIsr(taskToNotify, 2u, notifiedValue);
}

void testOS_ClearNotificationIndexed(void) {
    uint32_t indexToClear = 0u;
    /* Test function with Notification true */
    xTaskGenericNotifyStateClear_ExpectAndReturn(NULL, indexToClear, pdTRUE);
    OS_ClearNotificationIndexed(indexToClear);

    /* Test function with Notification false */
    xTaskGenericNotifyStateClear_ExpectAndReturn(NULL, indexToClear, pdFALSE);
    OS_ClearNotificationIndexed(indexToClear);
}

void testOS_ReceiveFromQueue(void) {
    uint32_t ticksToWait = 0u;
    TEST_ASSERT_FAIL_ASSERT(OS_ReceiveFromQueue(ftsk_testQueue, NULL_PTR, ticksToWait));

    /* Test function with Notification true */
    xQueueReceive_ExpectAndReturn(ftsk_testQueue, (void *)0u, (TickType_t)ticksToWait, pdTRUE);
    OS_ReceiveFromQueue(ftsk_testQueue, (void *)0u, ticksToWait);

    /* Test function with Notification false */
    xQueueReceive_ExpectAndReturn(ftsk_testQueue, (void *)0u, (TickType_t)ticksToWait, pdFALSE);
    OS_ReceiveFromQueue(ftsk_testQueue, (void *)0u, ticksToWait);
}

/** test that #OS_MarkTaskAsRequiringFpuContext calls the relevant FreeRTOS function */
void testOS_MarkTaskAsRequiringFpuContext(void) {
    vPortTaskUsesFPU_Expect();
    OS_MarkTaskAsRequiringFpuContext();
}

void testOS_GetNumberOfStoredMessagesInQueue(void) {
    OS_QUEUE testQueue = {0};
    uxQueueMessagesWaiting_ExpectAndReturn(testQueue, 5u);
    uint32_t numberOfMessages = OS_GetNumberOfStoredMessagesInQueue(testQueue);
    TEST_ASSERT_EQUAL(5u, numberOfMessages);
}

void testOS_SuspendTask(void) {
    TaskHandle_t taskToSuspend = ftsk_testtaskHandle;
    vTaskSuspend_Expect(taskToSuspend);

    OS_SuspendTask(taskToSuspend);
}

void testOS_ResumeTask(void) {
    TaskHandle_t taskToResume = ftsk_testtaskHandle;
    TEST_ASSERT_FAIL_ASSERT(OS_ResumeTask(taskToResume););

    vTaskResume_Expect(taskToResume);

    OS_ResumeTask(taskToResume);
}

void testOS_SendToBackOfQueueFromIsr(void) {
    /* ======= Assertion tests ============================================= */
    OS_QUEUE testQueue0 = {0};
    TEST_ASSERT_FAIL_ASSERT(OS_SendToBackOfQueueFromIsr(testQueue0, NULL_PTR, 0u));

    /* ======= Routine tests =============================================== */
    OS_QUEUE testQueue = {0};
    uint8_t dummyVar   = 1u;

    TEST_ASSERT_FAIL_ASSERT(OS_SendToBackOfQueueFromIsr(testQueue, NULL_PTR, NULL_PTR));

    xQueueGenericSendFromISR_ExpectAndReturn(testQueue, (void *)&dummyVar, NULL_PTR, queueSEND_TO_BACK, pdTRUE);
    TEST_ASSERT_EQUAL(OS_SUCCESS, OS_SendToBackOfQueueFromIsr(testQueue, (void *)&dummyVar, NULL_PTR));

    /* ======= RT1/2: Test implementation */
    xQueueGenericSendFromISR_ExpectAndReturn(testQueue, (void *)&dummyVar, NULL_PTR, queueSEND_TO_BACK, pdTRUE);
    /* ======= RT1/2: call function under test */
    OS_STD_RETURN_e success = OS_SendToBackOfQueueFromIsr(testQueue, (void *)&dummyVar, NULL_PTR);
    /* ======= RT1/2: test output verification */
    TEST_ASSERT_EQUAL(OS_SUCCESS, success);

    /* ======= RT2/2: Test implementation */
    xQueueGenericSendFromISR_ExpectAndReturn(testQueue, (void *)&dummyVar, NULL_PTR, queueSEND_TO_BACK, pdFAIL);
    /* ======= RT2/2: call function under test */
    OS_STD_RETURN_e failure = OS_SendToBackOfQueueFromIsr(testQueue, (void *)&dummyVar, NULL_PTR);
    /* ======= RT2/2: test output verification */
    TEST_ASSERT_EQUAL(OS_FAIL, failure);
}

void testOS_SendToBackOfQueue(void) {
    /* ======= Assertion tests ============================================= */
    OS_QUEUE testQueue0 = {0};
    TEST_ASSERT_FAIL_ASSERT(OS_SendToBackOfQueue(testQueue0, NULL_PTR, 0u));

    /* ======= Routine tests =============================================== */
    OS_QUEUE testQueue   = {0};
    uint32_t ticksToWait = 1u;

    uint8_t dummyVar = 1u;
    TEST_ASSERT_FAIL_ASSERT(OS_SendToBackOfQueue(testQueue, NULL_PTR, ticksToWait));

    xQueueGenericSend_ExpectAndReturn(testQueue, (void *)&dummyVar, ticksToWait, queueSEND_TO_BACK, pdTRUE);
    TEST_ASSERT_EQUAL(OS_SUCCESS, OS_SendToBackOfQueue(testQueue, (void *)&dummyVar, ticksToWait));

    /* ======= RT1/2: Test implementation */
    xQueueGenericSend_ExpectAndReturn(testQueue, (void *)&dummyVar, ticksToWait, queueSEND_TO_BACK, pdTRUE);

    /* ======= RT1/2: call function under test */
    OS_STD_RETURN_e success = OS_SendToBackOfQueue(testQueue, (void *)&dummyVar, ticksToWait);

    /* ======= RT1/2: test output verification */
    TEST_ASSERT_EQUAL(OS_SUCCESS, success);

    /* ======= RT2/2: Test implementation */
    xQueueGenericSend_ExpectAndReturn(testQueue, (void *)&dummyVar, ticksToWait, queueSEND_TO_BACK, errQUEUE_FULL);

    /* ======= RT2/2: call function under test */
    OS_STD_RETURN_e failure = OS_SendToBackOfQueue(testQueue, (void *)&dummyVar, ticksToWait);

    /* ======= RT2/2: test output verification */
    TEST_ASSERT_EQUAL(OS_FAIL, failure);
}

void testOS_TaskNotifyGiveFromISR(void) {
    TaskHandle_t taskToNotify           = ftsk_testtaskHandle;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskGenericNotifyGiveFromISR_Expect(ftsk_testtaskHandle, 0u, &xHigherPriorityTaskWoken);
    OS_TaskNotifyGiveFromISR(taskToNotify, &xHigherPriorityTaskWoken);

    TEST_ASSERT_EQUAL(xHigherPriorityTaskWoken, pdFALSE);
}
