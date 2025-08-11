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
 * @file    test_timer.c
 * @author  foxBMS Team
 * @date    2024-10-31 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the timer wrapper
 * @details Tests for the timer wrapper
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockftask.h"
#include "Mocktask.h"
#include "Mocktimers.h"

/* To have the configuration in all files available, FreeRTOSConfig.h was
   added as include in conf/unit/app_project_<platform>.yml */
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"

#include "test_assert_helper.h"
#include "timer.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("timer.c")

TEST_INCLUDE_PATH("../../src/app/task/timer")
TEST_INCLUDE_PATH("../../src/os/freertos/freertos/include")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/
StaticTimer_t buffer;
/* Since we are in an testing environment we have no OS to create a real timer.*/
TimerHandle_t timer;

uint32_t test_timerID = 123;
void *test_idPointer  = &test_timerID;

/* Empty Callback for the tests*/
void EmptyTestCallBack(TimerHandle_t xTimer) {
}

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    timer = (TimerHandle_t)&buffer;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/* start-include-in-doc */
void testTIMER_CreateWithNullPointers(void) {
    TEST_ASSERT_FAIL_ASSERT(TIMER_Create("test_timer", 100, false, test_idPointer, &EmptyTestCallBack, NULL));

    TEST_ASSERT_FAIL_ASSERT(TIMER_Create(NULL, 100, false, test_idPointer, &EmptyTestCallBack, &buffer));

    TEST_ASSERT_FAIL_ASSERT(TIMER_Create("test_timer", 100, false, test_idPointer, NULL, &buffer));
}

void testTIMER_CreateOk(void) {
    xTimerCreateStatic_ExpectAndReturn(
        "test_timer", pdMS_TO_TICKS(100), false, test_idPointer, &EmptyTestCallBack, NULL, timer);
    TEST_ASSERT_EQUAL(TIMER_Create("test_timer", 100, false, test_idPointer, &EmptyTestCallBack, NULL), timer);
}

void testTIMER_DeleteWithNullPointer(void) {
    TEST_ASSERT(TIMER_Delete(NULL, 100) == STD_NOT_OK);
}

void testTIMER_DeleteOk(void) {
    xTimerGenericCommandFromTask_ExpectAndReturn(timer, tmrCOMMAND_DELETE, 0, 0, 100, pdPASS);
    TEST_ASSERT_EQUAL(TIMER_Delete(timer, 100), STD_OK);
}

void testTIMER_DeleteNotOk(void) {
    xTimerGenericCommandFromTask_ExpectAndReturn(timer, tmrCOMMAND_DELETE, 0, 0, 100, pdFAIL);
    TEST_ASSERT_EQUAL(TIMER_Delete(timer, 100), STD_NOT_OK);
}

void testTIMER_StartWithNullPointer(void) {
    TEST_ASSERT(TIMER_Start(NULL, 100) == STD_NOT_OK);
}

void testTIMER_StartOk(void) {
    xTaskGetTickCount_ExpectAndReturn(10);
    xTimerGenericCommandFromTask_ExpectAndReturn(timer, tmrCOMMAND_START, 10, 0, 100, pdPASS);
    TEST_ASSERT_EQUAL(TIMER_Start(timer, 100), STD_OK);
}

void testTIMER_StartNotOk(void) {
    xTaskGetTickCount_ExpectAndReturn(10);
    xTimerGenericCommandFromTask_ExpectAndReturn(timer, tmrCOMMAND_START, 10, 0, 100, pdFAIL);
    TEST_ASSERT_EQUAL(TIMER_Start(timer, 100), STD_NOT_OK);
}

void testTIMER_StopWithNullPointer(void) {
    TEST_ASSERT(TIMER_Stop(NULL, 100) == STD_NOT_OK);
}

void testTIMER_StopOk(void) {
    xTimerGenericCommandFromTask_ExpectAndReturn(timer, tmrCOMMAND_STOP, 0, 0, 100, pdPASS);
    TEST_ASSERT_EQUAL(TIMER_Stop(timer, 100), STD_OK);
}

void testTIMER_StopNotOk(void) {
    xTimerGenericCommandFromTask_ExpectAndReturn(timer, tmrCOMMAND_STOP, 0, 0, 100, pdFAIL);
    TEST_ASSERT_EQUAL(TIMER_Stop(timer, 100), STD_NOT_OK);
}

void testTIMER_ResetWithNullPointer(void) {
    TEST_ASSERT(TIMER_Reset(NULL, 100) == STD_NOT_OK);
}

void testTIMER_ResetOk(void) {
    xTaskGetTickCount_ExpectAndReturn(10);
    xTimerGenericCommandFromTask_ExpectAndReturn(timer, tmrCOMMAND_RESET, 10, 0, 100, pdPASS);
    TEST_ASSERT_EQUAL(TIMER_Reset(timer, 100), STD_OK);
}

void testTIMER_ResetNotOk(void) {
    xTaskGetTickCount_ExpectAndReturn(10);
    xTimerGenericCommandFromTask_ExpectAndReturn(timer, tmrCOMMAND_RESET, 10, 0, 100, pdFAIL);
    TEST_ASSERT_EQUAL(TIMER_Reset(timer, 100), STD_NOT_OK);
}
/* stop-include-in-doc */
