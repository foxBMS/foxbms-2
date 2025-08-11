/*
 * FreeRTOS Kernel V11.1.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/******************************************************************************/
/* Hardware description related definitions. **********************************/
/******************************************************************************/

#include "config_cpu_clock_hz.h"
#define configCPU_CLOCK_HZ    ( ( unsigned portLONG ) HALCOGEN_CPU_CLOCK_HZ )                      /* Timer clock. */

/******************************************************************************/
/* Scheduling behaviour related definitions. **********************************/
/******************************************************************************/

#define configTICK_RATE_HZ                         ( ( TickType_t ) 1000 )
#define configUSE_PREEMPTION                       ( 1 )
#define configUSE_TIME_SLICING                     ( 0 )
#define configUSE_PORT_OPTIMISED_TASK_SELECTION    ( 1 )
#define configUSE_TICKLESS_IDLE                    ( 1 )
#define configMAX_PRIORITIES                       ( 15 )
#define configMINIMAL_STACK_SIZE                   ( ( unsigned portSHORT ) 200 )
#define configMAX_TASK_NAME_LEN                    ( 40 )
#define configTICK_TYPE_WIDTH_IN_BITS              TICK_TYPE_WIDTH_32_BITS
#define configIDLE_SHOULD_YIELD                    ( 1 )
#define configTASK_NOTIFICATION_ARRAY_ENTRIES      ( 3 )
#define configQUEUE_REGISTRY_SIZE                  ( 0 )
#define configENABLE_BACKWARD_COMPATIBILITY        ( 1 )
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS    ( 1 )

/* Enables the test whereby a stack larger than the total heap size is
 * requested. */
#define configSTACK_DEPTH_TYPE                     size_t
#define configMESSAGE_BUFFER_LENGTH_TYPE           size_t
#define configUSE_NEWLIB_REENTRANT                 ( 0 )

/******************************************************************************/
/* Software timer related definitions. ****************************************/
/******************************************************************************/

#define configUSE_TIMERS                ( 1 )
#define configTIMER_TASK_PRIORITY       ( 0 )
#define configTIMER_QUEUE_LENGTH        ( 4 )
#define configTIMER_TASK_STACK_DEPTH    ( configMINIMAL_STACK_SIZE * 2 )

/******************************************************************************/
/* Memory allocation related definitions. *************************************/
/******************************************************************************/

#define configSUPPORT_STATIC_ALLOCATION     ( 1 )
#define configSUPPORT_DYNAMIC_ALLOCATION    ( 1 )
#define configTOTAL_HEAP_SIZE               ( ( size_t ) 2 * 1024 )
#define configAPPLICATION_ALLOCATED_HEAP    ( 0 )

/******************************************************************************/
/* Interrupt nesting behaviour configuration. *********************************/
/******************************************************************************/

/******************************************************************************/
/* Hook and callback function related definitions. ****************************/
/******************************************************************************/

#define configUSE_IDLE_HOOK               ( 1 )
#define configUSE_TICK_HOOK               ( 0 )
#define configUSE_MALLOC_FAILED_HOOK      ( 0 )
#define configCHECK_FOR_STACK_OVERFLOW    ( 2 )

/******************************************************************************/
/* Run time and task stats gathering related definitions. *********************/
/******************************************************************************/

#define configGENERATE_RUN_TIME_STATS    ( 0 )
#define configUSE_TRACE_FACILITY         ( 0 )

/******************************************************************************/
/* Definitions that include or exclude functionality. *************************/
/******************************************************************************/

#define configUSE_TASK_NOTIFICATIONS           ( 1 )
/* Mutexes */
#define configUSE_MUTEXES                      ( 0 )
#define configUSE_RECURSIVE_MUTEXES            ( 0 )
/* Semaphores */
#define configUSE_COUNTING_SEMAPHORES          ( 1 )
#define INCLUDE_vTaskPrioritySet               ( 1 )
#define INCLUDE_uxTaskPriorityGet              ( 1 )
#define INCLUDE_vTaskDelete                    ( 1 )
#define INCLUDE_vTaskCleanUpResources          ( 0 )
#define INCLUDE_vTaskSuspend                   ( 1 )
#define INCLUDE_xTaskResumeFromISR             ( 1 )
#define INCLUDE_vTaskDelayUntil                ( 1 )
#define INCLUDE_vTaskDelay                     ( 1 )
#define INCLUDE_xTaskGetSchedulerState         ( 1 )
#define INCLUDE_uxTaskGetStackHighWaterMark    ( 1 )
#define INCLUDE_xTaskGetCurrentTaskHandle      ( 1 )
#define INCLUDE_xTaskAbortDelay                ( 1 )
#define INCLUDE_eTaskGetState                  ( 1 )
#define INCLUDE_xTaskGetHandle                 ( 1 )
#define INCLUDE_xTaskGetIdleTaskHandle         ( 1 )
#define configUSE_FPU                          ( 1 )
#define configUSE_TASK_FPU_SUPPORT             ( 1 )
#define configNUMBER_OF_CORES                  ( 1 )
#define configRECORD_STACK_HIGH_ADDRESS        ( 1 )
/* Co-routine definitions. */
#define configUSE_CO_ROUTINES                  ( 0 )
#define configMAX_CO_ROUTINE_PRIORITIES        ( 2 )
/* Memory Protection Unit */
#define configUSE_MPU_WRAPPERS_V1              ( 1 )

/* When using SMP (i.e. configNUMBER_OF_CORES is greater than one), set
 * configUSE_PASSIVE_IDLE_HOOK to 1 to allow the application writer to use
 * the passive idle task hook to add background functionality without the
 * overhead of a separate task. Defaults to 0 if left undefined. */
#define configUSE_PASSIVE_IDLE_HOOK            ( 0 )

#define configASSERT( x )    if( ( x ) == pdFALSE ) { taskDISABLE_INTERRUPTS(); for( ; ; ) {; } }

#endif /* FREERTOS_CONFIG_H */
