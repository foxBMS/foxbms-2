/**
 *
 * @copyright &copy; 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    os_freertos_config-validation.h
 * @author  foxBMS Team
 * @date    2019-08-27 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup OS
 * @prefix  OS
 *
 * @brief   Declaration of the OS wrapper interface
 * @details This module describes the interface to different operating systems
 */

#ifndef FOXBMS__OS_FREERTOS_CONFIG_VALIDATION_H_
#define FOXBMS__OS_FREERTOS_CONFIG_VALIDATION_H_

/*========== Includes =======================================================*/
#include "general.h"

#include "ftask_cfg.h"

#include "FreeRTOSConfig.h"

#include "fassert.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

#if !defined(INCLUDE_vTaskDelayUntil)
#error \
    "foxBMS requires using FreeRTOS 'vTaskDelayUntil' (wrapped in 'OS_taskDelayUntil'), therefore 'INCLUDE_vTaskDelayUntil' must be defined to '1'."
#endif /* INCLUDE_vTaskDelayUntil */

#if !defined(INCLUDE_xTaskGetSchedulerState)
#error \
    "foxBMS requires using FreeRTOS 'xTaskGetSchedulerState' therefore 'INCLUDE_xTaskGetSchedulerState' must be defined to '1'."
#endif /* INCLUDE_xTaskGetSchedulerState */

FAS_STATIC_ASSERT(
    (configMINIMAL_STACK_SIZE * GEN_BYTES_PER_WORD) < (StackType_t)FTSK_TASK_ENGINE_STACK_SIZE_IN_BYTES,
    "Size of the 'Engine' task is too small.");
FAS_STATIC_ASSERT(
    (configMINIMAL_STACK_SIZE * GEN_BYTES_PER_WORD) < (StackType_t)FTSK_TASK_CYCLIC_1MS_STACK_SIZE_IN_BYTES,
    "Size of the '1ms cyclic' task is too small.");
FAS_STATIC_ASSERT(
    (configMINIMAL_STACK_SIZE * GEN_BYTES_PER_WORD) < (StackType_t)FTSK_TASK_CYCLIC_10MS_STACK_SIZE_IN_BYTES,
    "Size of the '10ms cyclic' task is too small.");
FAS_STATIC_ASSERT(
    (configMINIMAL_STACK_SIZE * GEN_BYTES_PER_WORD) < (StackType_t)FTSK_TASK_CYCLIC_100MS_STACK_SIZE_IN_BYTES,
    "Size of the '100ms cyclic' task is too small.");
FAS_STATIC_ASSERT(
    (configMINIMAL_STACK_SIZE * GEN_BYTES_PER_WORD) < (StackType_t)FTSK_TASK_CYCLIC_ALGORITHM_100MS_STACK_SIZE_IN_BYTES,
    "Size of the '100ms cyclic algorithm' task is too small.");
FAS_STATIC_ASSERT(
    (sizeof(StackType_t) == sizeof(uint32_t)),
    "Assumptions on the size of StackType_t have been broken.");

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__OS_FREERTOS_CONFIG_VALIDATION_H_ */
