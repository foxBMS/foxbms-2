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
 * @file    os_freertos.c
 * @author  foxBMS Team
 * @date    2021-11-18 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup OS
 * @prefix  OS
 *
 * @brief   FreeRTOS specific implementation of the tasks and resources used by
 *          the system
 */

/*========== Includes =======================================================*/
#include "os.h"

#include "HL_sys_core.h"

#include "ftask.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern void OS_InitializeScheduler(void) {
    if (OS_ENABLE_CACHE == true) {
        _cacheEnable_();
    }
}

void OS_StartScheduler(void) {
    vTaskStartScheduler();
    /* This function should never return */
    FAS_ASSERT(FAS_TRAP);
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
/* FreeRTOS internal function, keep FreeRTOS types */
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
    FAS_ASSERT(pPreviousWakeTime != NULL_PTR);
    FAS_ASSERT(milliseconds > 0u);
    uint32_t ticks = (milliseconds / OS_TICK_RATE_MS);
    if ((uint32_t)ticks < 1u) {
        ticks = 1u; /* Minimum delay is 1 tick */
    }
    vTaskDelayUntil((TickType_t *)pPreviousWakeTime, (TickType_t)ticks);
}

extern void OS_MarkTaskAsRequiringFpuContext(void) {
    vPortTaskUsesFPU();
}

extern OS_STD_RETURN_e OS_WaitForNotification(uint32_t *pNotifiedValue, uint32_t timeout) {
    /* AXIVION Routine Generic-MissingParameterAssert: timeout: parameter accepts whole range */
    FAS_ASSERT(pNotifiedValue != NULL_PTR);

    OS_STD_RETURN_e notificationReceived = OS_FAIL;
    /* FreeRTOS: This function must not be used in an interrupt service routine. */
    /* ulBitsToClearOnEntry and ulBitsToClearOnExit set to 0xffffffff
       to clear all the bits in the task's notification value */
    BaseType_t xNotificationReceived = xTaskNotifyWait(UINT32_MAX, UINT32_MAX, pNotifiedValue, timeout);
    /* FreeRTOS:xTaskNotifyWait returns pdTRUE if notification was received (otherwise pdFALSE). */
    if (xNotificationReceived == pdTRUE) {
        notificationReceived = OS_SUCCESS;
    }
    return notificationReceived;
}

extern OS_STD_RETURN_e OS_NotifyFromIsr(TaskHandle_t taskToNotify, uint32_t notifiedValue) {
    /* AXIVION Routine Generic-MissingParameterAssert: notifiedValue: parameter accepts whole range */
    FAS_ASSERT(taskToNotify != NULL_PTR);

    OS_STD_RETURN_e notification        = OS_FAIL;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    /* Return value dependent on the value of the eAction parameter, here set to eSetValueWithOverwrite */
    BaseType_t xNotification =
        xTaskNotifyFromISR(taskToNotify, notifiedValue, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
    if (xNotification == pdTRUE) {
        notification = OS_SUCCESS;
    }
    /* Make the scheduler yield when notification made, so that unblocked tasks is run immediately
    (if priorities allow it, instead of waiting for the next OS tick) */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    return notification;
}

extern OS_STD_RETURN_e OS_WaitForNotificationIndexed(
    uint32_t indexToWaitOn,
    uint32_t *pNotifiedValue,
    uint32_t timeout) {
    /* AXIVION Routine Generic-MissingParameterAssert: timeout: parameter accepts whole range */
    FAS_ASSERT(pNotifiedValue != NULL_PTR);

    OS_STD_RETURN_e notificationReceived = OS_FAIL;
    /* FreeRTOS: This function must not be used in an interrupt service routine. */
    /* ulBitsToClearOnEntry and ulBitsToClearOnExit set to 0xffffffff
       to clear all the bits in the task's notification value */
    BaseType_t xNotificationReceived =
        xTaskNotifyWaitIndexed(indexToWaitOn, UINT32_MAX, UINT32_MAX, pNotifiedValue, timeout);
    /* FreeRTOS:xTaskNotifyWait returns pdTRUE if notification was received (otherwise pdFALSE). */
    if (xNotificationReceived == pdTRUE) {
        notificationReceived = OS_SUCCESS;
    }
    return notificationReceived;
}

extern OS_STD_RETURN_e OS_NotifyIndexedFromIsr(
    TaskHandle_t taskToNotify,
    uint32_t indexToNotify,
    uint32_t notifiedValue) {
    /* AXIVION Routine Generic-MissingParameterAssert: notifiedValue: parameter accepts whole range */
    FAS_ASSERT(taskToNotify != NULL_PTR);

    OS_STD_RETURN_e notification        = OS_FAIL;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    /* Return value dependent on the value of the eAction parameter, here set to eSetValueWithOverwrite */
    BaseType_t xNotification = xTaskNotifyIndexedFromISR(
        taskToNotify, indexToNotify, notifiedValue, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
    if (xNotification == pdTRUE) {
        notification = OS_SUCCESS;
    }
    /* Make the scheduler yield when notification made, so that unblocked tasks is run imediately
    (if priorities allow it, instead of waiting for the next OS tick) */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    return notification;
}

extern OS_STD_RETURN_e OS_ClearNotificationIndexed(uint32_t indexToClear) {
    /* AXIVION Routine Generic-MissingParameterAssert: indexToClear: parameter accepts whole range */

    OS_STD_RETURN_e notificationWasPending = OS_FAIL;
    /* NULL passed for task handle: the clear is made for the calling task */
    BaseType_t xNotificationWasPending = xTaskNotifyStateClearIndexed(NULL, indexToClear);
    /* FreeRTOS:xTaskNotifyStateClearIndexed returns pdTRUE if a notification was pending (otherwise pdFALSE). */
    if (xNotificationWasPending == pdTRUE) {
        notificationWasPending = OS_SUCCESS;
    }
    return notificationWasPending;
}

extern OS_STD_RETURN_e OS_ReceiveFromQueue(OS_QUEUE xQueue, void *const pvBuffer, uint32_t ticksToWait) {
    FAS_ASSERT(pvBuffer != NULL_PTR);

    OS_STD_RETURN_e queueReceiveSuccessfully = OS_FAIL;
    /* FreeRTOS: This function must not be used in an interrupt service routine. */
    BaseType_t xQueueReceiveSuccess = xQueueReceive(xQueue, pvBuffer, (TickType_t)ticksToWait);
    /* FreeRTOS:xQueueReceive returns pdTRUE if an item was successfully received from the queue (otherwise pdFALSE). */
    if (xQueueReceiveSuccess == pdTRUE) {
        queueReceiveSuccessfully = OS_SUCCESS;
    }
    return queueReceiveSuccessfully;
}

extern OS_STD_RETURN_e OS_SendToBackOfQueue(OS_QUEUE xQueue, const void *const pvItemToQueue, uint32_t ticksToWait) {
    FAS_ASSERT(pvItemToQueue != NULL_PTR);

    OS_STD_RETURN_e queueSendSuccessfully = OS_FAIL;
    BaseType_t xQueueSendSuccess          = xQueueSendToBack(xQueue, pvItemToQueue, (TickType_t)ticksToWait);
    /* FreeRTOS:xQueueSendToBack returns pdTRUE if the item was successfully posted (otherwise errQUEUE_FULL). */
    if (xQueueSendSuccess == pdTRUE) {
        queueSendSuccessfully = OS_SUCCESS;
    }
    return queueSendSuccessfully;
}

extern OS_STD_RETURN_e OS_SendToBackOfQueueFromIsr(
    OS_QUEUE xQueue,
    const void *const pvItemToQueue,
    long *const pxHigherPriorityTaskWoken) {
    FAS_ASSERT(pvItemToQueue != NULL_PTR);

    OS_STD_RETURN_e queueSendSuccessfully = OS_FAIL;
    BaseType_t xQueueSendSuccess =
        xQueueSendToBackFromISR(xQueue, pvItemToQueue, (BaseType_t *)pxHigherPriorityTaskWoken);
    /* FreeRTOS:xQueueSendToBackFromISR returns pdTRUE if the item was successfully posted (otherwise errQUEUE_FULL). */
    if (xQueueSendSuccess == pdTRUE) {
        queueSendSuccessfully = OS_SUCCESS;
    }
    return queueSendSuccessfully;
}

extern uint32_t OS_GetNumberOfStoredMessagesInQueue(OS_QUEUE xQueue) {
    long numberOfMessages = uxQueueMessagesWaiting(xQueue);
    return (uint32_t)numberOfMessages;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
