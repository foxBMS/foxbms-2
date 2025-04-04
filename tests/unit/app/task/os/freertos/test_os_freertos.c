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
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
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

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/** test that #OS_StartScheduler calls the relevant FreeRTOS function */
void testOS_StartScheduler(void) {
    vTaskStartScheduler_Expect();
    TEST_ASSERT_FAIL_ASSERT(OS_StartScheduler());
}

void testvApplicationIdleHookCallsUserCodeIdle(void) {
    /* TODO: Fix the multiple definition through mocking a user definied function in Issue #1063
    * FreeRTOSConfig.h needs to bee included in os related unit tests. As the some functions are user
    * implemented, FreeRTOS only supplies the declaration of them in task.h. In os_freertos.c this collides with the
    * actual definition. When implementing vApplicationIdleHook as weak the unit test fails as then the mock is tested.
    */

    /*
    FTSK_RunUserCodeIdle_Expect();
    vApplicationIdleHook();
    */
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

void testOS_SendToBackOfQueueFromIsr(void) {
    OS_QUEUE testQueue = {0};
    uint8_t dummyVar   = 1u;
    xQueueGenericSendFromISR_ExpectAndReturn(testQueue, (void *)&dummyVar, NULL_PTR, queueSEND_TO_BACK, pdTRUE);
    TEST_ASSERT_EQUAL(OS_SUCCESS, OS_SendToBackOfQueueFromIsr(testQueue, (void *)&dummyVar, NULL_PTR));

    xQueueGenericSendFromISR_ExpectAndReturn(testQueue, (void *)&dummyVar, NULL_PTR, queueSEND_TO_BACK, pdFAIL);
    TEST_ASSERT_EQUAL(OS_FAIL, OS_SendToBackOfQueueFromIsr(testQueue, (void *)&dummyVar, NULL_PTR));
}

void testOS_SendToBackOfQueue(void) {
    OS_QUEUE testQueue   = {0};
    uint8_t dummyVar     = 1u;
    uint32_t ticksToWait = 1u;
    xQueueGenericSend_ExpectAndReturn(testQueue, (void *)&dummyVar, ticksToWait, queueSEND_TO_BACK, pdTRUE);
    TEST_ASSERT_EQUAL(OS_SUCCESS, OS_SendToBackOfQueue(testQueue, (void *)&dummyVar, ticksToWait));

    xQueueGenericSend_ExpectAndReturn(testQueue, (void *)&dummyVar, ticksToWait, queueSEND_TO_BACK, errQUEUE_FULL);
    TEST_ASSERT_EQUAL(OS_FAIL, OS_SendToBackOfQueue(testQueue, (void *)&dummyVar, ticksToWait));
}
