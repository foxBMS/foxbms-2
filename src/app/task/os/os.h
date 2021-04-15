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
 * @file    os.h
 * @author  foxBMS Team
 * @date    2019-08-27 (date of creation)
 * @updated 2020-01-21 (date of last update)
 * @ingroup OS
 * @prefix  OS
 *
 * @brief   Implementation of the tasks used by the system, headers
 *
 */

#ifndef FOXBMS__OS_H_
#define FOXBMS__OS_H_

/*========== Includes =======================================================*/
#include "general.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "semphr.h"
#include "task.h"

/*========== Macros and Definitions =========================================*/

/** @brief  Number of mutexes for the engine TODO engine what?! */
#define OS_NUM_OF_MUTEXES 0
/** @brief  Number of events for the engine TODO engine what?! */
#define OS_NUM_OF_EVENTS 0

/**
 * @brief   typedef for thread priority. The higher the value, the higher the
 *          priority.
 */
typedef enum OS_PRIORITY {
    OS_PRIORITY_IDLE,           /**< priority: idle (lowest)      */
    OS_PRIORITY_LOW,            /**< priority: low                */
    OS_PRIORITY_BELOW_NORMAL,   /**< priority: below normal       */
    OS_PRIORITY_NORMAL,         /**< priority: normal (default)   */
    OS_PRIORITY_ABOVE_NORMAL,   /**< priority: above normal       */
    OS_PRIORITY_HIGH,           /**< priority: high               */
    OS_PRIORITY_ABOVE_HIGH,     /**< priority: above high         */
    OS_PRIORITY_VERY_HIGH,      /**< priority: very high          */
    OS_PRIORITY_BELOW_REALTIME, /**< priority: below realtime     */
    OS_PRIORITY_REAL_TIME,      /**< priority: realtime (highest) */
} OS_PRIORITY_e;

/** @brief  enum of OS boot states */
typedef enum OS_BOOT_STATE {
    OS_OFF,                          /**< system is off                                                   */
    OS_CREATE_QUEUES,                /**< state right before queues are created                           */
    OS_CREATE_MUTEX,                 /**< state right before mutexes are created                          */
    OS_CREATE_EVENT,                 /**< state right before events are created                           */
    OS_CREATE_TASKS,                 /**< state right before tasks are created                            */
    OS_INIT_PRE_OS,                  /**< state right after tasks are created                             */
    OS_SCHEDULER_RUNNING,            /**< scheduler is running                                            */
    OS_ENGINE_RUNNING,               /**< state right after scheduler is started and engine is initalized */
    OS_PRECYCLIC_INIT_HAS_FINISHED,  /**< state after the precyclic init has finished                     */
    OS_SYSTEM_RUNNING,               /**< system is running                                               */
    OS_INIT_OS_FATALERROR_SCHEDULER, /**< error in scheduler                                              */
    OS_INIT_OS_FATALERROR,           /**< fatal error                                                     */
    OS_BOOT_STATE_MAX,               /**< DO NOT CHANGE, MUST BE THE LAST ENTRY */
} OS_BOOT_STATE_e;

/** @brief  OS timer */
typedef struct OS_TIMER {
    uint8_t timer_1ms;   /**< milliseconds     */
    uint8_t timer_10ms;  /**< 10 milliseconds  */
    uint8_t timer_100ms; /**< 100 milliseconds */
    uint8_t timer_sec;   /**< seconds          */
    uint8_t timer_min;   /**< minutes          */
    uint8_t timer_h;     /**< hours            */
    uint16_t timer_d;    /**< days             */
} OS_TIMER_s;

/** @brief  struct for FreeRTOS task definition */
typedef struct OS_TASK_DEFINITION {
    uint32_t phase;       /**< TODO (ms) */
    uint32_t cycleTime;   /**< TODO (ms) */
    UBaseType_t priority; /**< TODO */
    uint16_t stackSize;   /**<  Defines the size, in words, of the stack
                                   allocated to the idle task.  */
} OS_TASK_DEFINITION_s;

/*========== Extern Constant and Variable Declarations ======================*/
/** boot state of the system */
extern volatile OS_BOOT_STATE_e os_boot;
/** os timer for counting the system ticks */
extern volatile OS_TIMER_s os_timer;

/** @brief  Scheduler "zero" time for task phase control */
extern uint32_t os_schedulerStartTime;

/** handles for OS objects @{*/
extern SemaphoreHandle_t os_mutexes[];
extern EventGroupHandle_t os_events[];
/**@}*/

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   Starts the operating system scheduler
 */
extern void OS_StartScheduler(void);

/**
 * @brief   Initialization the RTOS interface
 * @details This function initializes the mutexes, eventgroups and tasks.
 */
extern void OS_InitializeTasks(void);

/**
 * @brief   Supplies the memory for the idle task.
 * @details This is needed due to the usage of configSUPPORT_STATIC_ALLOCATION.
 *          This is an FreeRTOS function an does not adhere to foxBMS function
 *          naming convetions.
 * @param   ppxIdleTaskTCBBuffer    TODO
 * @param   ppxIdleTaskStackBuffer  TODO
 * @param   pulIdleTaskStackSize    TODO
 */
extern void vApplicationGetIdleTaskMemory(
    StaticTask_t **ppxIdleTaskTCBBuffer,
    StackType_t **ppxIdleTaskStackBuffer,
    uint32_t *pulIdleTaskStackSize);

#if (configUSE_TIMERS > 0) && (configSUPPORT_STATIC_ALLOCATION == 1)
/**
 * @brief   Supplies the memory for the timer task.
 * @details This is necessary for the combination of
 *          configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS.
 *          This is an FreeRTOS function an does not adhere to foxBMS function
 *          naming convetions.
 * @param   ppxTimerTaskTCBBuffer   TODO
 * @param   ppxTimerTaskStackBuffer TODO
 * @param   pulTimerTaskStackSize   TODO
 */
extern void vApplicationGetTimerTaskMemory(
    StaticTask_t **ppxTimerTaskTCBBuffer,
    StackType_t **ppxTimerTaskStackBuffer,
    uint32_t *pulTimerTaskStackSize);
#endif /* configUSE_TIMERS */

/**
 * @brief   Hook function for the idle task
 * @details This is an FreeRTOS function an does not adhere to foxBMS function
 *          naming convetions
 */
extern void vApplicationIdleHook(void);

/**
 * @brief   Hook function for StackOverflowHandling.
 * @details This handler is used when the operation system encounters a
 *          stackoverflow in a task.
 *          This is an FreeRTOS function an does not adhere to foxBMS function
 *          naming convetions
 * @param   xTask       TODO
 * @param   pcTaskName  TODO
 */
extern void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName);

/**
 * @brief   Enter Critical interface function for use in FreeRTOS-Tasks and
 *          FreeRTOS-ISR
 * @details checks the function context (task/thread mode or interrupt
 *          (handler) mode) and calls the corresponding enter-critical
 *          function
 */
extern void OS_EnterTaskCritical(void);

/**
 * @brief   Exit Critical interface function for use in FreeRTOS-Tasks and
 *          FreeRTOS-ISR
 * @details Checks the function context (task/thread mode or interrupt
 *          (handler) mode) and calls the corresponding exit-critical
 *          function
 */
extern void OS_ExitTaskCritical(void);

/**
 * @brief   Increments the system timer os_timer
 * @details The os_timer is a runtime-counter, counting the time since the
 *          last reset.
 * @param   timer   TODO
 */
extern void OS_TriggerTimer(volatile OS_TIMER_s *timer);

/**
 * @brief   Returns OS based system tick value.
 * @details TODO
 * @return  time stamp in milliseconds, based on the operating system time.
 */
extern uint32_t OS_GetTickCount(void);

/**
 * @brief   Delays a task in milliseconds
 * @details TODO
 * @param   delay_ms    time delay value
 */
extern void OS_DelayTask(uint32_t delay_ms);

/**
 * @brief    Delay a task until a specified time
 * @details  TODO
 * @param    pPreviousWakeTime   Pointer to a variable that holds the time at
 *                               which the task was last unblocked.
 *                               PreviousWakeTime must be initialized with the
 *                               current time prior to its first use
 *                               (PreviousWakeTime = OS_osSysTick()).
 * @param    milliseconds        time delay value in milliseconds
 */
extern void OS_DelayTaskUntil(uint32_t *pPreviousWakeTime, uint32_t milliseconds);

/**
 * @brief   Handles the tick increment of operating systick timer
 * @details TODO
 */
extern void OS_SystemTickHandler(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__OS_H_ */
