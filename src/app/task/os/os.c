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
 * @file    os.c
 * @author  foxBMS Team
 * @date    2019-08-27 (date of creation)
 * @updated 2021-07-23 (date of last update)
 * @ingroup OS
 * @prefix  OS
 *
 * @brief   Implementation of the tasks used by the system
 *
 */

/*========== Includes =======================================================*/
#include "os.h"

#include "ftask.h"

/*========== Macros and Definitions =========================================*/

/** stack size of the idle task */
#define OS_IDLE_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

#if (configUSE_TIMERS > 0) && (configSUPPORT_STATIC_ALLOCATION == 1)
#define OS_TIMER_TASK_STACK_SIZE configTIMER_TASK_STACK_DEPTH
#endif /* configUSE_TIMERS */

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/
/** boot state of the OS */
volatile OS_BOOT_STATE_e os_boot = OS_OFF;
/** system timer variable */
volatile OS_TIMER_s os_timer = {0, 0, 0, 0, 0, 0, 0};
/** timestamp of the scheduler start */
uint32_t os_schedulerStartTime = 0;

/** Buffer for the Idle Task's structure */
static StaticTask_t os_idleTaskTcbBuffer;

/** @brief Stack for the Idle task */
static StackType_t os_stackSizeIdle[OS_IDLE_TASK_STACK_SIZE];

#if (configUSE_TIMERS > 0) && (configSUPPORT_STATIC_ALLOCATION == 1)
/** Buffer for the Timer Task's structure */
static StaticTask_t os_timerTaskTcbBuffer;
#endif /* configUSE_TIMERS */

#if (configUSE_TIMERS > 0) && (configSUPPORT_STATIC_ALLOCATION == 1)
/** Stack for the Timer Task */
static StackType_t os_stackSizeTimer[OS_TIMER_TASK_STACK_SIZE];
#endif /* configUSE_TIMERS */

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

void OS_StartScheduler(void) {
    vTaskStartScheduler();
}

void OS_InitializeOperatingSystem(void) {
    /* operating system configuration (Queues, Tasks) */
    os_boot = OS_CREATE_QUEUES;
    FTSK_CreateQueues();
    os_boot = OS_CREATE_TASKS;
    FTSK_CreateTasks();
    os_boot = OS_INIT_PRE_OS;
}

void vApplicationGetIdleTaskMemory(
    StaticTask_t **ppxIdleTaskTCBBuffer,
    StackType_t **ppxIdleTaskStackBuffer,
    uint32_t *pulIdleTaskStackSize) {
    *ppxIdleTaskTCBBuffer   = &os_idleTaskTcbBuffer;
    *ppxIdleTaskStackBuffer = &os_stackSizeIdle[0];
    *pulIdleTaskStackSize   = OS_IDLE_TASK_STACK_SIZE;
}

#if (configUSE_TIMERS > 0) && (configSUPPORT_STATIC_ALLOCATION == 1)
void vApplicationGetTimerTaskMemory(
    StaticTask_t **ppxTimerTaskTCBBuffer,
    StackType_t **ppxTimerTaskStackBuffer,
    uint32_t *pulTimerTaskStackSize) {
    *ppxTimerTaskTCBBuffer   = &os_timerTaskTcbBuffer;
    *ppxTimerTaskStackBuffer = &os_stackSizeTimer[0];
    *pulTimerTaskStackSize   = configTIMER_TASK_STACK_DEPTH;
}
#endif /* configUSE_TIMERS */

void vApplicationIdleHook(void) {
    FTSK_RunUserCodeIdle();
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    FAS_ASSERT(FAS_TRAP);
}

void OS_TriggerTimer(volatile OS_TIMER_s *timer) {
    if (++timer->timer_1ms > 9) {
        /* 10ms */
        timer->timer_1ms = 0;

        if (++timer->timer_10ms > 9) {
            /* 100ms */
            timer->timer_10ms = 0;

            if (++timer->timer_100ms > 9) {
                /* 1s */
                timer->timer_100ms = 0;

                if (++timer->timer_sec > 59) {
                    /* 1min */
                    timer->timer_sec = 0;

                    if (++timer->timer_min > 59) {
                        /* 1h */
                        timer->timer_min = 0;

                        if (++timer->timer_h > 23) {
                            /* 1d */
                            timer->timer_h = 0;
                            ++timer->timer_d;
                        }
                    }
                }
            }
        }
    }
}

void OS_EnterTaskCritical(void) {
    taskENTER_CRITICAL();
}

void OS_ExitTaskCritical(void) {
    taskEXIT_CRITICAL();
}

uint32_t OS_GetTickCount(void) {
    return xTaskGetTickCount(); /*TMS570 does not support nested interrupts*/
}

void OS_DelayTask(uint32_t delay_ms) {
#if INCLUDE_vTaskDelay
    TickType_t ticks = delay_ms / portTICK_PERIOD_MS;

    vTaskDelay(ticks ? ticks : 1); /* Minimum delay = 1 tick */
#else
#error "Can't use OS_taskDelay."
#endif
}

void OS_DelayTaskUntil(uint32_t *pPreviousWakeTime, uint32_t milliseconds) {
#if INCLUDE_vTaskDelayUntil
    TickType_t ticks = (milliseconds / portTICK_PERIOD_MS);
    vTaskDelayUntil((TickType_t *)pPreviousWakeTime, ticks ? ticks : 1);

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

/*========== Externalized Static Function Implementations (Unit Test) =======*/
