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
 * @file    ftask.c
 * @author  foxBMS Team
 * @date    2019-08-27 (date of creation)
 * @updated 2021-12-01 (date of last update)
 * @ingroup TASK
 * @prefix  FTSK
 *
 * @brief   Implementation of OS-independent task creators
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "ftask.h"

#include "sys_mon.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
/* AXIVION Next Line Style MisraC2012Directive-1.1 MisraC2012-1.2 FaultDetection-DeadBranches: tell the CCS compiler
 * that this function is a task, context save not necessary */
#pragma TASK(FTSK_CreateTaskEngine)
extern void FTSK_CreateTaskEngine(void *const pvParameters) {
    FAS_ASSERT(pvParameters == NULL_PTR);
    OS_MarkTaskAsRequiringFpuContext();
    os_boot = OS_SCHEDULER_RUNNING;
    FTSK_InitializeUserCodeEngine();
    os_boot = OS_ENGINE_RUNNING;

    /* AXIVION Next Line Style MisraC2012-2.2 FaultDetection-DeadBranches: FreeRTOS task setup requires an infinite
     * loop for the user code (see www.freertos.org/a00125.html)*/
    while (true) {
        /* notify system monitoring that task will be called */
        SYSM_Notify(SYSM_TASK_ID_ENGINE, SYSM_NOTIFY_ENTER, OS_GetTickCount());
        /* user code implemention */
        FTSK_RunUserCodeEngine();
        /* notify system monitoring that task has been called */
        SYSM_Notify(SYSM_TASK_ID_ENGINE, SYSM_NOTIFY_EXIT, OS_GetTickCount());
    }
}

/* AXIVION Next Line Style MisraC2012Directive-1.1 MisraC2012-1.2 FaultDetection-DeadBranches: tell the CCS compiler
 * that this function is a task, context save not necessary */
#pragma TASK(FTSK_CreateTaskCyclic1ms)
extern void FTSK_CreateTaskCyclic1ms(void *const pvParameters) {
    FAS_ASSERT(pvParameters == NULL_PTR);
    OS_MarkTaskAsRequiringFpuContext();
    uint32_t currentTimeCreateTaskCyclic1ms = 0;

    while (os_boot != OS_ENGINE_RUNNING) {
    }

    FTSK_InitializeUserCodePreCyclicTasks();
    os_boot = OS_PRECYCLIC_INIT_HAS_FINISHED;

    /* cycle time (1ms) equals the minimum tick time (1ms),
     * therefore it is not possible to configure a phase for this task */
    currentTimeCreateTaskCyclic1ms = OS_GetTickCount();
    /* AXIVION Next Line Style MisraC2012-2.2 FaultDetection-DeadBranches: FreeRTOS task setup requires an infinite
     * loop for the user code (see www.freertos.org/a00125.html)*/
    while (true) {
        /* notify system monitoring that task will be called */
        SYSM_Notify(SYSM_TASK_ID_CYCLIC_1ms, SYSM_NOTIFY_ENTER, OS_GetTickCount());
        /* user code implementation */
        FTSK_RunUserCodeCyclic1ms();
        /* notify system monitoring that task has been called */
        SYSM_Notify(SYSM_TASK_ID_CYCLIC_1ms, SYSM_NOTIFY_EXIT, OS_GetTickCount());
        /* let task sleep until it is due again */
        OS_DelayTaskUntil(&currentTimeCreateTaskCyclic1ms, ftsk_taskDefinitionCyclic1ms.cycleTime);
    }
}

/* AXIVION Next Line Style MisraC2012Directive-1.1 MisraC2012-1.2 FaultDetection-DeadBranches: tell the CCS compiler
 * that this function is a task, context save not necessary */
#pragma TASK(FTSK_CreateTaskCyclic10ms)
extern void FTSK_CreateTaskCyclic10ms(void *const pvParameters) {
    FAS_ASSERT(pvParameters == NULL_PTR);
    OS_MarkTaskAsRequiringFpuContext();
    uint32_t currentTimeCreateTaskCyclic10ms = 0;

    while (os_boot != OS_PRECYCLIC_INIT_HAS_FINISHED) {
    }

    OS_DelayTaskUntil(&os_schedulerStartTime, ftsk_taskDefinitionCyclic10ms.phase);
    currentTimeCreateTaskCyclic10ms = OS_GetTickCount();
    /* AXIVION Next Line Style MisraC2012-2.2 FaultDetection-DeadBranches: FreeRTOS task setup requires an infinite
     * loop for the user code (see www.freertos.org/a00125.html)*/
    while (true) {
        /* notify system monitoring that task will be called */
        SYSM_Notify(SYSM_TASK_ID_CYCLIC_10ms, SYSM_NOTIFY_ENTER, OS_GetTickCount());
        /* user code implementation */
        FTSK_RunUserCodeCyclic10ms();
        /* notify system monitoring that task has been called */
        SYSM_Notify(SYSM_TASK_ID_CYCLIC_10ms, SYSM_NOTIFY_EXIT, OS_GetTickCount());
        /* let task sleep until it is due again */
        OS_DelayTaskUntil(&currentTimeCreateTaskCyclic10ms, ftsk_taskDefinitionCyclic10ms.cycleTime);
    }
}

/* AXIVION Next Line Style MisraC2012Directive-1.1 MisraC2012-1.2 FaultDetection-DeadBranches: tell the CCS compiler
 * that this function is a task, context save not necessary */
#pragma TASK(FTSK_CreateTaskCyclic100ms)
extern void FTSK_CreateTaskCyclic100ms(void *const pvParameters) {
    FAS_ASSERT(pvParameters == NULL_PTR);
    OS_MarkTaskAsRequiringFpuContext();
    uint32_t currentTimeCreateTaskCyclic100ms = 0;

    while (os_boot != OS_PRECYCLIC_INIT_HAS_FINISHED) {
    }

    OS_DelayTaskUntil(&os_schedulerStartTime, ftsk_taskDefinitionCyclic100ms.phase);
    currentTimeCreateTaskCyclic100ms = OS_GetTickCount();
    /* AXIVION Next Line Style MisraC2012-2.2 FaultDetection-DeadBranches: FreeRTOS task setup requires an infinite
     * loop for the user code (see www.freertos.org/a00125.html)*/
    while (true) {
        /* notify system monitoring that task will be called */
        SYSM_Notify(SYSM_TASK_ID_CYCLIC_100ms, SYSM_NOTIFY_ENTER, OS_GetTickCount());
        /* user code implementation */
        FTSK_RunUserCodeCyclic100ms();
        /* notify system monitoring that task has been called */
        SYSM_Notify(SYSM_TASK_ID_CYCLIC_100ms, SYSM_NOTIFY_EXIT, OS_GetTickCount());
        /* let task sleep until it is due again */
        OS_DelayTaskUntil(&currentTimeCreateTaskCyclic100ms, ftsk_taskDefinitionCyclic100ms.cycleTime);
    }
}

/* AXIVION Next Line Style MisraC2012Directive-1.1 MisraC2012-1.2 FaultDetection-DeadBranches: tell the CCS compiler
 * that this function is a task, context save not necessary */
#pragma TASK(FTSK_CreateTaskCyclicAlgorithm100ms)
extern void FTSK_CreateTaskCyclicAlgorithm100ms(void *const pvParameters) {
    FAS_ASSERT(pvParameters == NULL_PTR);
    OS_MarkTaskAsRequiringFpuContext();
    uint32_t currentTimeCreateTaskCyclicAlgorithms100ms = 0;

    while (os_boot != OS_PRECYCLIC_INIT_HAS_FINISHED) {
    }

    OS_DelayTaskUntil(&os_schedulerStartTime, ftsk_taskDefinitionCyclicAlgorithm100ms.phase);
    os_boot                                    = OS_SYSTEM_RUNNING;
    currentTimeCreateTaskCyclicAlgorithms100ms = OS_GetTickCount();
    /* AXIVION Next Line Style MisraC2012-2.2 FaultDetection-DeadBranches: FreeRTOS task setup requires an infinite
     * loop for the user code (see www.freertos.org/a00125.html)*/
    while (true) {
        /* notify system monitoring that task will be called */
        SYSM_Notify(SYSM_TASK_ID_CYCLIC_ALGORITHM_100ms, SYSM_NOTIFY_ENTER, OS_GetTickCount());
        /* user code implementation */
        FTSK_RunUserCodeCyclicAlgorithm100ms();
        /* notify system monitoring that task has been called */
        SYSM_Notify(SYSM_TASK_ID_CYCLIC_ALGORITHM_100ms, SYSM_NOTIFY_EXIT, OS_GetTickCount());
        /* let task sleep until it is due again */
        OS_DelayTaskUntil(
            &currentTimeCreateTaskCyclicAlgorithms100ms, ftsk_taskDefinitionCyclicAlgorithm100ms.cycleTime);
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
