/**
 *
 * @copyright &copy; 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    test_ethernet_freertos.c
 * @author  foxBMS Team
 * @date    2026-01-23 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of some module
 * @details Detailed Test Description
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockFreeRTOS_Routing.h"
#include "MockFreeRTOS_Sockets.h"
#include "Mockmpu_prototypes.h"
#include "Mockos.h"

#include "ethernet_freertos.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("ethernet_freertos.c")
TEST_INCLUDE_PATH("../../src/app/application/ethernet")
TEST_INCLUDE_PATH("../../src/os/freertos/freertos-plus/freertos-plus-tcp/source/include")
TEST_INCLUDE_PATH("../../src/os/freertos/freertos-plus/freertos-plus-tcp/source/portable/Compiler/CCS")

/*========== Definitions and Implementations for Unit Test ==================*/
/* Mock handles */
OS_TASK_HANDLE mock_task_handle_listening   = (OS_TASK_HANDLE)0x12345678;
OS_TASK_HANDLE mock_task_handle_echo_server = (OS_TASK_HANDLE)0x12347679;

/* Test variables */
TaskFunction_t test_task_function  = (TaskFunction_t)0x11111111;
TaskFunction_t test_task_function2 = (TaskFunction_t)0x11111112;
NetworkEndPoint_t test_endpoint;
Socket_t test_socket = (Socket_t)0x22222222;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/**
 * @brief   Testing externalized function
 *          #ETH_CreateListeningTask
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: Invalid taskFunction pointer &rarr; assert
 *            - AT2/2: Invalid Endpoint pointer &rarr; assert
 *          - Routine validation:
 *            - RT1/1: Create Task
 *
 */
void testETH_CreateListeningTask(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(ETH_CreateListeningTask(NULL_PTR, &test_endpoint));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(ETH_CreateListeningTask(test_task_function, NULL_PTR));
    /* ======= Routine tests =============================================== */
    static StackType_t eth_stackListenforConnection[2048] = {0};
    static StaticTask_t eth_taskListenforConnection       = {0};
    /* ======= RT1/1: Test implementation */

    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    MPU_xTaskCreateStatic_ExpectAndReturn(
        test_task_function,
        "Listening-Task",
        512, /* Size in words */
        &(test_endpoint),
        PRIORITY_BELOW_NORMAL,
        &eth_stackListenforConnection[0],
        &eth_taskListenforConnection,
        mock_task_handle_listening);

    /* ======= RT1/1: Call function under test */
    ETH_RETURN_TYPE_e result = ETH_CreateListeningTask(test_task_function, &test_endpoint);

    TEST_ASSERT_EQUAL(ETH_OK, result);
}

/**
 * @brief   Testing externalized function
 *          #ETH_CreateEchoServerTask
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: Invalid taskFunction pointer &rarr; assert

 *          - Routine validation:
 *            - RT1/1: Call function the first time
 *
 */
void testETH_CreateEchoServerTask(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(ETH_CreateEchoServerTask(NULL_PTR));

    /* ======= Routine tests =============================================== */
    static StackType_t eth_stackEchoServer[4096] = {0};
    static StaticTask_t eth_taskEchoServer       = {0};
    /* ======= RT1/1: Test implementation */

    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    MPU_xTaskCreateStatic_ExpectAndReturn(
        test_task_function2,
        "Server-Task",
        1024, /* Size in words */
        NULL_PTR,
        PRIORITY_BELOW_NORMAL,
        &eth_stackEchoServer[0],
        &eth_taskEchoServer,
        mock_task_handle_echo_server);

    /* ======= RT1/1: Call function under test */
    ETH_RETURN_TYPE_e result = ETH_CreateEchoServerTask(test_task_function2);
    TEST_ASSERT_EQUAL(ETH_OK, result);
}

/**
 * @brief   Testing externalized function
 *          #ETH_DeleteCurrentTask
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/2: Call function under test
 */
void testETH_DeleteCurrentTask(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    MPU_vTaskDelete_Expect(NULL);
    /* ======= RT1/2: Call function under test */
    ETH_DeleteCurrentTask();
}
