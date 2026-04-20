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
 * @file    ethernet_freertos.c
 * @author  foxBMS Team
 * @date    2026-01-23 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup APPLICATION
 * @prefix  ETH
 *
 * @brief   FreeRTOS abstraction for ethernet application
 * @details As the TCP-Support is build optionally the abstraction is placed here
 */

/*========== Includes =======================================================*/
#include "foxbms_config.h"

#include "general.h"

#include "ethernet_freertos.h"

#include "ethernet_cfg.h"

#include "fassert.h"

/*========== Macros and Definitions =========================================*/

#define ETH_BYTES_TO_WORDS(VARIABLE_IN_BYTES) ((VARIABLE_IN_BYTES) / GEN_BYTES_PER_WORD)

/*========== Static Constant and Variable Definitions =======================*/

OS_TASK_HANDLE eth_taskHandleListening;
OS_TASK_HANDLE eth_taskHandleEchoServer;

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern ETH_RETURN_TYPE_e ETH_CreateListeningTask(TaskFunction_t taskFunction, NetworkEndPoint_t *pEndPoint) {
    FAS_ASSERT(taskFunction != NULL_PTR);
    FAS_ASSERT(pEndPoint != NULL_PTR);

    static bool taskAlreadyCreated                                                   = false;
    static StackType_t eth_stackListenforConnection[ETH_LISTENING_TASK_STACK_SIZE_B] = {0};
    static StaticTask_t eth_taskListenforConnection                                  = {0};

    OS_EnterTaskCritical();
    if (taskAlreadyCreated == true) {
        OS_ExitTaskCritical();
        return ETH_TASK_ALREADY_CREATED;
    }
    taskAlreadyCreated = true;
    OS_ExitTaskCritical();

    /* Spawn listening task */
    eth_taskHandleListening = xTaskCreateStatic(
        taskFunction,
        "Listening-Task",
        ETH_BYTES_TO_WORDS(ETH_LISTENING_TASK_STACK_SIZE_B),
        pEndPoint,
        (uint32_t)PRIORITY_BELOW_NORMAL,
        &eth_stackListenforConnection[0],
        &eth_taskListenforConnection /* Holds the task's data structure. */
    );
    FAS_ASSERT(eth_taskHandleListening != NULL); /* Trap if initialization failed */

    return ETH_OK;
}

extern ETH_RETURN_TYPE_e ETH_CreateEchoServerTask(TaskFunction_t taskFunction) {
    FAS_ASSERT(taskFunction != NULL_PTR);

    static bool taskAlreadyCreated                                            = false;
    static StackType_t eth_stackEchoServer[ETH_ECHO_SERVER_TASK_STACK_SIZE_B] = {0};
    static StaticTask_t eth_taskEchoServer                                    = {0};

    OS_EnterTaskCritical();
    if (taskAlreadyCreated == true) {
        OS_ExitTaskCritical();
        return ETH_TASK_ALREADY_CREATED;
    }
    taskAlreadyCreated = true;
    OS_ExitTaskCritical();

    /* Spawn a task to handle the connection. */
    eth_taskHandleEchoServer = xTaskCreateStatic(
        taskFunction,
        "Server-Task",
        ETH_BYTES_TO_WORDS(ETH_ECHO_SERVER_TASK_STACK_SIZE_B),
        NULL_PTR,
        (uint32_t)PRIORITY_BELOW_NORMAL,
        &eth_stackEchoServer[0],
        &eth_taskEchoServer);
    FAS_ASSERT(eth_taskHandleEchoServer != NULL); /* Trap if initialization failed */

    return ETH_OK;
}

extern void ETH_DeleteCurrentTask(void) {
    vTaskDelete(NULL);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
