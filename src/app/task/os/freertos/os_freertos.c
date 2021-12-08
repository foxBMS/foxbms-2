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
 * @file    os_freertos.c
 * @author  foxBMS Team
 * @date    2021-11-18 (date of creation)
 * @updated 2021-12-01 (date of last update)
 * @ingroup OS
 * @prefix  OS
 *
 * @brief   FreeRTOS specific implementation of the tasks and resources used by
 *          the system
 */

/*========== Includes =======================================================*/
#include "os.h"

#include "ftask.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

void OS_StartScheduler(void) {
    vTaskStartScheduler();
}

void vApplicationGetIdleTaskMemory(
    StaticTask_t **ppxIdleTaskTCBBuffer,
    StackType_t **ppxIdleTaskStackBuffer,
    uint32_t *pulIdleTaskStackSize) {
    /** Buffer for the Idle Task's structure */
    static StaticTask_t os_idleTask = {0};
    /** @brief Stack for the Idle task */
    static StackType_t os_stackSizeIdle[OS_IDLE_TASK_STACK_SIZE] = {0};
    FAS_ASSERT(ppxIdleTaskTCBBuffer != NULL_PTR);
    FAS_ASSERT(ppxIdleTaskStackBuffer != NULL_PTR);
    FAS_ASSERT(pulIdleTaskStackSize != NULL_PTR);
    *ppxIdleTaskTCBBuffer   = &os_idleTask;
    *ppxIdleTaskStackBuffer = &os_stackSizeIdle[0];
    *pulIdleTaskStackSize   = OS_IDLE_TASK_STACK_SIZE;
}

#if (configUSE_TIMERS > 0) && (configSUPPORT_STATIC_ALLOCATION == 1)
void vApplicationGetTimerTaskMemory(
    StaticTask_t **ppxTimerTaskTCBBuffer,
    StackType_t **ppxTimerTaskStackBuffer,
    uint32_t *pulTimerTaskStackSize) {
#if (configUSE_TIMERS > 0) && (configSUPPORT_STATIC_ALLOCATION == 1)
    /** Buffer for the Timer Task's structure */
    static StaticTask_t os_timerTask;
#endif /* configUSE_TIMERS */

#if (configUSE_TIMERS > 0) && (configSUPPORT_STATIC_ALLOCATION == 1)
    /** Stack for the Timer Task */
    static StackType_t os_stackSizeTimer[OS_TIMER_TASK_STACK_SIZE];
#endif /* configUSE_TIMERS */
    *ppxTimerTaskTCBBuffer   = &os_timerTask;
    *ppxTimerTaskStackBuffer = &os_stackSizeTimer[0];
    *pulTimerTaskStackSize   = configTIMER_TASK_STACK_DEPTH;
}
#endif /* configUSE_TIMERS */

void vApplicationIdleHook(void) {
    FTSK_RunUserCodeIdle();
}

#if (configCHECK_FOR_STACK_OVERFLOW > 0)
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    FAS_ASSERT(FAS_TRAP);
}
#endif /* configCHECK_FOR_STACK_OVERFLOW */

void OS_EnterTaskCritical(void) {
    taskENTER_CRITICAL();
}

void OS_ExitTaskCritical(void) {
    taskEXIT_CRITICAL();
}

uint32_t OS_GetTickCount(void) {
    return xTaskGetTickCount(); /*TMS570 does not support nested interrupts*/
}

void OS_DelayTaskUntil(uint32_t *pPreviousWakeTime, uint32_t milliseconds) {
#if INCLUDE_vTaskDelayUntil
    FAS_ASSERT(pPreviousWakeTime != NULL_PTR);
    FAS_ASSERT(milliseconds > 0u);
    TickType_t ticks = ((TickType_t)milliseconds / portTICK_PERIOD_MS);
    if ((uint32_t)ticks < 1u) {
        ticks = 1u; /* Minimum delay is 1 tick */
    }
    vTaskDelayUntil((TickType_t *)pPreviousWakeTime, ticks);

#else
#error "Can't use OS_taskDelayUntil."
#endif
}

void OS_SystemTickHandler(void) {
#if (INCLUDE_xTaskGetSchedulerState == 1)
    /* Only increment operating systick timer if scheduler started */
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        xTaskIncrementTick();
    }
#else
    xTaskIncrementTick();
#endif /* INCLUDE_xTaskGetSchedulerState */
}

extern void OS_MarkTaskAsRequiringFpuContext(void) {
    vPortTaskUsesFPU();
}

extern OS_STD_RETURN_e OS_ReceiveFromQueue(OS_QUEUE xQueue, void *const pvBuffer, uint32_t ticksToWait) {
    FAS_ASSERT(pvBuffer != NULL_PTR);

    OS_STD_RETURN_e queueReceiveSucessfull = OS_FAIL;
    /* FreeRTOS: This function must not be used in an interrupt service routine. */
    BaseType_t xQueueReceiveSuccess = xQueueReceive(xQueue, pvBuffer, (TickType_t)ticksToWait);
    /* FreeRTOS:xQueueReceive returns pdTRUE if an item was successfully received from the queue (otherwise pdFALSE). */
    if (xQueueReceiveSuccess == pdTRUE) {
        queueReceiveSucessfull = OS_SUCCESS;
    }
    return queueReceiveSucessfull;
}

extern OS_STD_RETURN_e OS_SendToBackOfQueue(OS_QUEUE xQueue, const void *const pvItemToQueue, TickType_t ticksToWait) {
    FAS_ASSERT(pvItemToQueue != NULL_PTR);

    OS_STD_RETURN_e queueSendSucessfull = OS_FAIL;
    BaseType_t xQueueSendSuccess        = xQueueSendToBack(xQueue, pvItemToQueue, ticksToWait);
    /* FreeRTOS:xQueueSendToBack returns pdTRUE if the item was successfully posted (otherwise errQUEUE_FULL). */
    if (xQueueSendSuccess == pdTRUE) {
        queueSendSucessfull = OS_SUCCESS;
    }
    return queueSendSucessfull;
}

extern OS_STD_RETURN_e OS_SendToBackOfQueueFromIsr(
    OS_QUEUE xQueue,
    const void *const pvItemToQueue,
    long *const pxHigherPriorityTaskWoken) {
    FAS_ASSERT(pvItemToQueue != NULL_PTR);

    OS_STD_RETURN_e queueSendSucessfull = OS_FAIL;
    BaseType_t xQueueSendSuccess =
        xQueueSendToBackFromISR(xQueue, pvItemToQueue, (BaseType_t *)pxHigherPriorityTaskWoken);
    /* FreeRTOS:xQueueSendToBackFromISR returns pdTRUE if the item was successfully posted (otherwise errQUEUE_FULL). */
    if (xQueueSendSuccess == pdTRUE) {
        queueSendSucessfull = OS_SUCCESS;
    }
    return queueSendSucessfull;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST

#endif
