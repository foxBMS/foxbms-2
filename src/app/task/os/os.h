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
 * @file    os.h
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

#ifndef FOXBMS__OS_H_
#define FOXBMS__OS_H_

/*========== Includes =======================================================*/

#include "fstd_types.h"

#if defined(FOXBMS_USES_FREERTOS)
#include "FreeRTOS.h"
#include "queue.h"
#define OS_TASK_HANDLE          TaskHandle_t
#define OS_QUEUE                QueueHandle_t
#define OS_IDLE_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE) /**< stack size of the idle task */
#define OS_TICK_RATE_MS         (portTICK_RATE_MS)         /**< FreeRTOS name of the tick rate */
#define OS_ENABLE_CACHE         (false)                    /**< true: Enable cache, false: Disable cache */
#endif

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

#if (configUSE_TIMERS > 0) && (configSUPPORT_STATIC_ALLOCATION == 1)
#define OS_TIMER_TASK_STACK_SIZE configTIMER_TASK_STACK_DEPTH
#endif /* configUSE_TIMERS */

/** enum to encapsulate function returns from the OS-wrapper layer */
typedef enum {
    OS_SUCCESS, /**< OS-dependent operation successful      */
    OS_FAIL,    /**< OS-dependent operation unsuccessful    */
} OS_STD_RETURN_e;

/**
 * @brief   typedef for thread priority. The higher the value, the higher the
 *          priority.
 */
typedef enum {
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
typedef enum {
    OS_OFF,                                    /**< system is off                                                   */
    OS_INITIALIZE_SCHEDULER,                   /**< state right before initializing the scheduler                    */
    OS_CREATE_QUEUES,                          /**< state right before queues are created                           */
    OS_CREATE_TASKS,                           /**< state right before tasks are created                            */
    OS_INIT_PRE_OS,                            /**< state right after tasks are created                             */
    OS_SCHEDULER_RUNNING,                      /**< scheduler is running                                            */
    OS_ENGINE_RUNNING,                         /**< state right after scheduler is started and engine is initalized */
    OS_PRE_CYCLIC_INITIALIZATION_HAS_FINISHED, /**< state after the pre-cyclic init has finished                    */
    OS_SYSTEM_RUNNING,                         /**< system is running                                               */
    OS_INIT_OS_FATALERROR_SCHEDULER,           /**< error in scheduler                                              */
    OS_INIT_OS_FATALERROR,                     /**< fatal error                                                     */
    OS_BOOT_STATE_MAX,                         /**< DO NOT CHANGE, MUST BE THE LAST ENTRY */
} OS_BOOT_STATE_e;

/** @brief  OS timer */
typedef struct {
    uint8_t timer_1ms;   /**< milliseconds     */
    uint8_t timer_10ms;  /**< 10 milliseconds  */
    uint8_t timer_100ms; /**< 100 milliseconds */
    uint8_t timer_sec;   /**< seconds          */
    uint8_t timer_min;   /**< minutes          */
    uint8_t timer_h;     /**< hours            */
    uint16_t timer_d;    /**< days             */
} OS_TIMER_s;

/** @brief  struct for FreeRTOS task definition */
typedef struct {
    OS_PRIORITY_e priority; /*!< priority of the task */
    uint32_t phase;         /*!< shift in ms of the first start of the task */
    uint32_t cycleTime;     /*!< time in ms that will be waited between each task cycle */
    uint32_t stackSize_B;   /*!< Defines the size, in bytes, of the stack allocated to the task */
    void *pvParameters;     /*!< value that is passed as the parameter to the task. */
} OS_TASK_DEFINITION_s;

/*========== Extern Constant and Variable Declarations ======================*/
/** boot state of the system */
extern volatile OS_BOOT_STATE_e os_boot;

/** @brief  Scheduler "zero" time for task phase control */
extern uint32_t os_schedulerStartTime;

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief Initialization function for the scheduler
 */
extern void OS_InitializeScheduler(void);

/**
 * @brief   Starts the operating system scheduler
 */
extern void OS_StartScheduler(void);

/**
 * @brief   Initialization the RTOS interface
 * @details This function initializes the scheduler and then creates queues and
 *          tasks.
 */
extern void OS_InitializeOperatingSystem(void);

#if (configUSE_TIMERS > 0) && (configSUPPORT_STATIC_ALLOCATION == 1)
/**
 * @brief   Supplies the memory for the timer task.
 * @details This is necessary for the combination of
 *          configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS.
 *          This is an FreeRTOS function an does not adhere to foxBMS function
 *          naming conventions.
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
 *          naming conventions
 */
extern void vApplicationIdleHook(void);

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
 */
extern void OS_IncrementTimer(void);

/**
 * @brief   Returns OS based system tick value.
 * @details TODO
 * @return  time stamp in milliseconds, based on the operating system time.
 */
extern uint32_t OS_GetTickCount(void);

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
 * @brief   Marks the current task as requiring FPU context
 * @details In order to avoid corruption of the registers of the floating
 *          point unit during a task switch, every task that uses the FPU has
 *          to call this function at its start.
 *
 *          This instructs the underlying operating system to store the context
 *          of the FPU when switching a task.
 *
 *          This function has to be called from within a task.
 */
extern void OS_MarkTaskAsRequiringFpuContext(void);

/**
 * @brief   Wait for a notification
 * @details This function needs to implement the wrapper to OS specific
 *          blocking task and waiting for notification.
 *          This function must not be called from within an interrupt service
 *          routine (due to the FreeRTOS compatibility of the the wrapper).
 *          This function blocks the current tasks and wait for a notification.
 *          Typically the notification is made in an interrupt.
 *          The notified value is passed by the notifying function.
 *          The tasks unblocks when the notification is received or when
 *          timeout milliseconds have passed.
 * @param   pNotifiedValue  Value passed by the notify function
 * @param   timeout         ticks to wait before unblocking task if no
 *                          notification is received
 * @return  #OS_SUCCESS if a notification was successfully received, otherwise
 *          #OS_FAIL
 */
extern OS_STD_RETURN_e OS_WaitForNotification(uint32_t *pNotifiedValue, uint32_t timeout);

/**
 * @brief   Notify a task
 * @details This function needs to implement the wrapper to OS specific
 *          task notification.
 *          This function is to be called from within an interrupt service
 *          routine (due to the FreeRTOS compatibility of the the wrapper).
 *          This function makes a notification to the task whose handle is
 *          is passed as parameter.
 *          Typically the notification is made in an interrupt.
 *          The value to notify is passed to the notified task.
 * @param   taskToNotify   Handle of task to notify.
 * @param   notifiedValue  Value to pass to the notified task
 * @return  #OS_SUCCESS if a notification was successfully made, otherwise
 *          #OS_FAIL
 */
extern OS_STD_RETURN_e OS_NotifyFromIsr(TaskHandle_t taskToNotify, uint32_t notifiedValue);

/**
 * @brief   Wait for a notification, with index
 * @details This function needs to implement the wrapper to OS specific
 *          blocking task and waiting for notification.
 *          This function must not be called from within an interrupt service
 *          routine (due to the FreeRTOS compatibility of the the wrapper).
 *          This function blocks the current tasks and wait for a notification
 *          and a specific index.
 *          Typically the notification is made in an interrupt.
 *          The notified value and index are passed by the notifying function.
 *          The tasks unblocks when the notification is received or when
 *          timeout milliseconds have passed.
 * @param   indexToWaitOn   index to wait on
 * @param   pNotifiedValue  Value passed by the notify function
 * @param   timeout         ticks to wait before unblocking task if no
 *                          notification is received
 * @return  #OS_SUCCESS if a notification was successfully received, otherwise
 *          #OS_FAIL
 */
extern OS_STD_RETURN_e OS_WaitForNotificationIndexed(
    uint32_t indexToWaitOn,
    uint32_t *pNotifiedValue,
    uint32_t timeout);

/**
 * @brief   Notify a task, with index
 * @details This function needs to implement the wrapper to OS specific
 *          task notification.
 *          This function is to be called from within an interrupt service
 *          routine (due to the FreeRTOS compatibility of the the wrapper).
 *          This function makes a notification with an index to the task whose
 *          handle is passed as parameter.
 *          Typically the notification is made in an interrupt.
 *          The value to notify and the index are passed to the notified task.
 * @param   taskToNotify   Handle of task to notify
 * @param   indexToNotify  Index to notify
 * @param   notifiedValue  Value to pass to the notified task
 * @return  #OS_SUCCESS if a notification was successfully made, otherwise
 *          #OS_FAIL.
 */
extern OS_STD_RETURN_e OS_NotifyIndexedFromIsr(
    TaskHandle_t taskToNotify,
    uint32_t indexToNotify,
    uint32_t notifiedValue);

/**
 * @brief   Clear pending notification of a task, with index
 * @details This function needs to implement the wrapper to OS specific
 *          task notification clearing.
 *          It clears the pending notification for the calling task.
 *          This function must not be called from within an interrupt service
 *          routine (due to the FreeRTOS compatibility of the the wrapper).
 *          This function clears a pending notification corresponding to the
 *          index passed as parameter.
 * @param   indexToClear   Index of the notification to clear
 * @return  #OS_SUCCESS if a notification was pending, otherwise #OS_FAIL
 */
extern OS_STD_RETURN_e OS_ClearNotificationIndexed(uint32_t indexToClear);

/**
 * @brief   Receive an item from a queue
 * @details This function needs to implement the wrapper to OS specific queue
 *          posting.
 *          The queue needs to be implement in a FreeRTOS compatible way.
 *          This function must not be called from within an interrupt service
 *          routine (due to the FreeRTOS compatibility of the the wrapper).
 * @param   xQueue      FreeRTOS compatible queue handle that should be posted
 *                      to
 * @param   pvBuffer    Pointer to the buffer into which the received item is
 *                      posted to.
 * @param   ticksToWait ticks to wait
 * @return  #OS_SUCCESS if an item was successfully received, otherwise
 *          #OS_FAIL.
 */
extern OS_STD_RETURN_e OS_ReceiveFromQueue(OS_QUEUE xQueue, void *const pvBuffer, uint32_t ticksToWait);

/**
 * @brief   Post an item to the back the provided queue
 * @details This function needs to implement the wrapper to OS specific queue
 *          posting.
 *          The queue needs to be implement in a FreeRTOS compatible way.
 * @param   xQueue          FreeRTOS compatible queue handle that should be
 *                          posted to.
 * @param   pvItemToQueue   Pointer to the item to be posted in the queue.
 * @param   ticksToWait     ticks to wait
 * @return #OS_SUCCESS if the item was successfully posted, otherwise #OS_FAIL.
 */
extern OS_STD_RETURN_e OS_SendToBackOfQueue(OS_QUEUE xQueue, const void *const pvItemToQueue, uint32_t ticksToWait);

/**
 * @brief   Post an item to the back the provided queue during an ISR
 * @details This function needs to implement the wrapper to OS specific queue
 *          posting.
 * @param   xQueue                      queue handle that should be posted to.
 * @param   pvItemToQueue               Pointer to the item to be posted in the
 *                                      queue.
 * @param   pxHigherPriorityTaskWoken   Indicates whether a context switch is
 *                                      required or not.
 *                                      If the parameter is a NULL_PTR, the
 *                                      context switch will happen at the next
 *                                      tick.
 * @return #OS_SUCCESS if the item was successfully posted, otherwise #OS_FAIL.
 */
extern OS_STD_RETURN_e OS_SendToBackOfQueueFromIsr(
    OS_QUEUE xQueue,
    const void *const pvItemToQueue,
    long *const pxHigherPriorityTaskWoken);

/**
 * @brief   Check if messages are waiting for queue
 * @details This function needs to implement the wrapper to OS specific queue
 *          posting.
 * @param   xQueue                   queue handle that should be posted to.
 * @return number of message currently stored in xQueue
 */
extern uint32_t OS_GetNumberOfStoredMessagesInQueue(OS_QUEUE xQueue);

/**
 * @brief   This function checks if timeToPass has passed since the last timestamp to now
 * @details This function retrieves the current time stamp with #OS_GetTickCount(),
 *          compares it to the oldTimestamp_ms and checks if more or equal of
 *          timeToPass_ms timer increments have passed.
 * @param[in]   oldTimeStamp_ms timestamp that shall be compared to the current time in ms
 * @param[in]   timeToPass_ms   timer increments (in ms) that shall pass between oldTimeStamp_ms and now
 * @returns     true in the case that more than timeToPass_ms timer increments have passed, otherwise false
 */
extern bool OS_CheckTimeHasPassed(uint32_t oldTimeStamp_ms, uint32_t timeToPass_ms);

/**
 * @brief   This function checks if timeToPass has passed since the last timestamp to now
 * @details This function is passed the current time stamp as argument currentTimeStamp_ms,
 *          compares it to the oldTimestamp_ms and checks if more or equal of
 *          timeToPass_ms timer increments have passed.
 * @param[in]   oldTimeStamp_ms     timestamp that shall be compared to the current time in ms
 * @param[in]   currentTimeStamp_ms timestamp of the current time in ms
 * @param[in]   timeToPass_ms       timer increments (in ms) that shall pass between oldTimeStamp_ms and now
 * @returns     true in the case that more than timeToPass_ms timer increments have passed, otherwise false
 */
extern bool OS_CheckTimeHasPassedWithTimestamp(
    uint32_t oldTimeStamp_ms,
    uint32_t currentTimeStamp_ms,
    uint32_t timeToPass_ms);

/**
 * @brief   Does a self check if the #OS_CheckTimeHasPassedWithTimestamp works as expected
 * @details This functions tests some values with #OS_CheckTimeHasPassedWithTimestamp().
 *          It is intended to be side-effect free and to be callable any time to verify
 *          from the running program if this portion of the software is working as
 *          expected.
 * returns  STD_OK if the self check passes successfully, STD_NOT_OK otherwise
 */
extern STD_RETURN_TYPE_e OS_CheckTimeHasPassedSelfTest(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern OS_TIMER_s *TEST_OS_GetOsTimer();
#endif

#endif /* FOXBMS__OS_H_ */
