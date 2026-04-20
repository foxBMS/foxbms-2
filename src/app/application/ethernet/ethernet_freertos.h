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
 * @file    ethernet_freertos.h
 * @author  foxBMS Team
 * @date    2026-01-23 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup APPLICATION
 * @prefix  ETH
 *
 * @brief   Header file of FreeRTOS abstraction
 * @details As the TCP-Support is build optionally the abstraction is placed here
 */

#ifndef FOXBMS__ETHERNET_FREERTOS_H_
#define FOXBMS__ETHERNET_FREERTOS_H_

/*========== Includes =======================================================*/

#include "FreeRTOS_Routing.h"
#include "FreeRTOS_Sockets.h"

#include "os.h"

/*========== Macros and Definitions =========================================*/
typedef enum {
    ETH_OK,
    ETH_TASK_ALREADY_CREATED,
} ETH_RETURN_TYPE_e;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief Creates a listening task for Ethernet communication.
 *
 * @details This function creates a single instance of a listening task using FreeRTOS's
 * xTaskCreateStatic. If the task has already been created, it returns an error.
 *
 * @param[in] taskFunction Pointer to the task function to be executed.
 * @param[in] pEndPoint    Pointer to the network endpoint configuration.
 *
 * @return ETH_OK if the task was successfully created,
 *         ETH_TASK_ALREADY_CREATED if the task already exists.
 */
extern ETH_RETURN_TYPE_e ETH_CreateListeningTask(TaskFunction_t taskFunction, NetworkEndPoint_t *pEndPoint);
/**
 * @brief Creates an Echo Server task for handling a connected socket.
 *
 * @details This function deletes any previously existing Echo Server task and creates
 * a new one using FreeRTOS's xTaskCreateStatic to handle the given connected socket.
 *
 * @param[in] taskFunction   Pointer to the task function to be executed.
 *
 * @return ETH_OK on successful creation,
 *         ETH_TASK_ALREADY_CREATED if the task already exists.
 */
extern ETH_RETURN_TYPE_e ETH_CreateEchoServerTask(TaskFunction_t taskFunction);
/**
 * @brief Deletes the currently running task.
 *
 * @details This function uses FreeRTOS's vTaskDelete to delete the calling task.
 * The task must have been created using the FreeRTOS API.
 */
extern void ETH_DeleteCurrentTask(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__ETHERNET_FREERTOS_H_ */
