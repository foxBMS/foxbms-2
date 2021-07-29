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
 * @updated 2021-07-23 (date of last update)
 * @ingroup TASK
 * @prefix  FTSK
 *
 * @brief   Implementation of engine task driver
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "ftask.h"

#include "can_cfg.h"

#include "FreeRTOS.h"
#include "stream_buffer.h"

#include "database.h"
#include "sys_mon.h"

/*========== Macros and Definitions =========================================*/
/** Length of queue that is used in the database */
#define FTSK_DATABASE_QUEUE_LENGTH (1u)

/** Size of queue item that is used in the database */
#define FTSK_DATABASE_QUEUE_ITEM_SIZE (sizeof(DATA_QUEUE_MESSAGE_s))

/** Length of queue that is used in the insulation measurement device (IMD) */
#define FTSK_IMD_QUEUE_LENGTH (5u)
/** Size of queue item that is used in the IMD driver */
#define FTSK_IMD_QUEUE_ITEM_SIZE (sizeof(CAN_BUFFERELEMENT_s))

/** Length of queue that is used in the can module for receiving messages */
#define FTSK_CAN_RX_QUEUE_LENGTH (50u)
/** Size of queue item that is used in the can driver */
#define FTSK_CAN_RX_QUEUE_ITEM_SIZE (sizeof(CAN_BUFFERELEMENT_s))

/*========== Static Constant and Variable Definitions =======================*/
/**
 * @brief   size of storage area for the database queue
 * @details The array that is used for the queue's storage area.
 *          This must be at least
 *          #FTSK_DATABASE_QUEUE_LENGTH * #FTSK_DATABASE_QUEUE_ITEM_SIZE
 */
static uint8_t ftsk_databaseQueueStorageArea[FTSK_DATABASE_QUEUE_LENGTH * FTSK_DATABASE_QUEUE_ITEM_SIZE];
/** structure for static database queue */
static StaticQueue_t ftsk_databaseQueueStructure;

/**
 * @brief   size of storage area for the IMD queue
 * @details The array that is used for the queue's storage area.
 *          This must be at least
 *          #FTSK_IMD_QUEUE_LENGTH * #FTSK_IMD_QUEUE_ITEM_SIZE
 */
static uint8_t ftsk_imdQueueStorageArea[FTSK_IMD_QUEUE_LENGTH * FTSK_IMD_QUEUE_ITEM_SIZE];
/** structure for static data queue */
static StaticQueue_t ftsk_imdQueueStructure;

/* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE can-documentation-rx-queue-vars-start-include */
/** structure for static data queue */
static StaticQueue_t ftsk_canRxQueueStructure = {0};
/**
 * @brief   size of storage area for the CAN Rx queue
 * @details The array that is used for the queue's storage area.
 *          This must be at least
 *          #FTSK_CAN_RX_QUEUE_LENGTH * #FTSK_CAN_RX_QUEUE_ITEM_SIZE
 */
static uint8_t ftsk_canRxQueueStorageArea[FTSK_CAN_RX_QUEUE_LENGTH * FTSK_CAN_RX_QUEUE_ITEM_SIZE] = {0};
/* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE can-documentation-rx-queue-vars-stop-include */

/********* Engine Task *****************************************************/
/** Definition of task handle of the engine task */
static TaskHandle_t ftsk_taskHandleEngine;

/** Task Struct for #ftsk_taskHandleEngine */
static StaticTask_t ftsk_taskEngine;

/** Stack of #ftsk_taskHandleEngine */
static StackType_t ftsk_stackSizeEngine[FTSK_TASK_ENGINE_STACK_SIZE];

/********* Cyclic 1 ms Task *************************************************/
/** Definition of task handle for the cyclic 1 ms task */
static TaskHandle_t ftsk_taskHandleCyclic1ms;

/** Task Struct for #ftsk_taskHandleCyclic1ms */
static StaticTask_t ftsk_taskCyclic1ms;

/** Stack of #ftsk_taskHandleCyclic1ms */
static StackType_t ftsk_stackSizeCyclic1ms[FTSK_TASK_CYCLIC_1MS_STACK_SIZE];

/********* Cyclic 10 ms Task ************************************************/
/** Definition of task handle for the cyclic 10 ms task */
static TaskHandle_t ftsk_taskHandleCyclic10ms;

/** Task Struct for #ftsk_taskHandleCyclic10ms */
static StaticTask_t ftsk_taskCyclic10ms;

/** Stack of #ftsk_taskHandleCyclic10ms */
static StackType_t ftsk_stackSizeCyclic10ms[FTSK_TASK_CYCLIC_10MS_STACK_SIZE];

/********* Cyclic 100 ms Task ***********************************************/
/** Definition of task handle for the cyclic 100 ms task */
static TaskHandle_t ftsk_taskHandleCyclic100ms;

/** Task Struct for #ftsk_taskHandleCyclic100ms */
static StaticTask_t ftsk_taskCyclic100ms;

/** Stack of #ftsk_taskHandleCyclic100ms */
static StackType_t ftsk_stackSizeCyclic100ms[FTSK_TASK_CYCLIC_100MS_STACK_SIZE];

/********* Cyclic 100 ms Task for Algorithms ********************************/
/** Definition of task handle for the cyclic 100 ms task for algorithms */
static TaskHandle_t ftsk_taskHandleCyclicAlgorithm100ms;

/** Task Struct for #ftsk_taskHandleCyclicAlgorithm100ms */
static StaticTask_t ftsk_taskCyclicAlgorithm100ms;

/** Stack of #ftsk_taskHandleCyclicAlgorithm100ms */
static StackType_t ftsk_stackSizeCyclicAlgorithm100ms[FTSK_TASK_CYCLIC_ALGORITHM_100MS_STACKSIZE];

/*========== Extern Constant and Variable Definitions =======================*/
volatile bool ftsk_allQueuesCreated = false;

QueueHandle_t ftsk_databaseQueue = NULL_PTR;

QueueHandle_t ftsk_imdCanDataQueue = NULL_PTR;

/* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE can-documentation-rx-queue-handle-start-include */
QueueHandle_t ftsk_canRxQueue = NULL_PTR;
/* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE can-documentation-rx-queue-handle-stop-include */

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Database-Task
 * @details The task manages the data exchange with the database and must have a
 *          higher task priority than any task using the database.
 * @ingroup API_OS
 */
static void FTSK_CreateTaskEngine(void);

/**
 * @brief   Creation of cyclic 1 ms task
 * @details The Task calls OsStartUp() in the very beginning, this is the first
 *          active Task. Then the Task is delayed by a phase as defined in
 *          ftsk_tskdef_cyclic_1ms.phase (in milliseconds). After the phase
 *          delay, the cyclic execution starts, the entry time is saved in
 *          current_time. After one cycle, the Task is set to sleep until entry
 *          time + ftsk_tskdef_cyclic_1ms.cycleTime (in milliseconds).
 */
static void FTSK_CreateTaskCyclic1ms(void);

/**
 * @brief   Creation of cyclic 10 ms task
 * @details Task is delayed by a phase as defined in
 *          ftsk_tskdef_cyclic_10ms.phase (in milliseconds). After the phase
 *          delay, the cyclic execution starts, the entry time is saved in
 *          current_time. After one cycle, the Task is set to sleep until entry
 *          time + ftsk_tskdef_cyclic_10ms.cycleTime (in milliseconds).
 */
static void FTSK_CreateTaskCyclic10ms(void);

/**
 * @brief   Creation of cyclic 100 ms task
 * @details Task is delayed by a phase as defined in
 *          ftsk_tskdef_cyclic_100ms.phase (in milliseconds). After the phase
 *          delay, the cyclic execution starts, the entry time is saved in
 *          current_time. After one cycle, the Task is set to sleep until entry
 *          time + ftsk_tskdef_cyclic_100ms.cycleTime (in milliseconds).
 */
static void FTSK_CreateTaskCyclic100ms(void);

/**
 * @brief   Creation of cyclic 100 ms algorithm task
 * @details Task is delayed by a phase as defined in
 *          ftsk_taskDefinitionCyclicAlgorithm100ms.Phase (in milliseconds).
 *          After the phase delay, the cyclic execution starts, the entry time
 *          is saved in current_time. After one cycle, the Task is set to sleep
 *          until
 *          entry time + ftsk_taskDefinitionCyclicAlgorithm100ms.CycleTime
 *          (in milliseconds).
 */
static void FTSK_CreateTaskCyclicAlgorithm100ms(void);

/*========== Static Function Implementations ================================*/
/* tell compiler this function is a task, context save not necessary */
#pragma TASK(FTSK_CreateTaskEngine)
static void FTSK_CreateTaskEngine(void) {
    os_boot = OS_SCHEDULER_RUNNING;
    FTSK_InitializeUserCodeEngine();
    os_boot = OS_ENGINE_RUNNING;

    OS_DelayTaskUntil(&os_schedulerStartTime, ftsk_taskDefinitionEngine.phase);
    while (1) {
        /* notify system monitoring that task will be called */
        SYSM_Notify(SYSM_TASK_ID_ENGINE, SYSM_NOTIFY_ENTER, OS_GetTickCount());
        /* user code implemention */
        FTSK_RunUserCodeEngine();
        /* notify system monitoring that task has been called */
        SYSM_Notify(SYSM_TASK_ID_ENGINE, SYSM_NOTIFY_EXIT, OS_GetTickCount());
    }
}

/* tell compiler this function is a task, context save not necessary */
#pragma TASK(FTSK_CreateTaskCyclic1ms)
static void FTSK_CreateTaskCyclic1ms(void) {
    uint32_t current_time = 0;

    while (os_boot != OS_ENGINE_RUNNING) {
    }

    FTSK_InitializeUserCodePreCyclicTasks();
    os_boot = OS_PRECYCLIC_INIT_HAS_FINISHED;

    OS_DelayTaskUntil(&os_schedulerStartTime, ftsk_taskDefinitionCyclic1ms.phase);
    current_time = OS_GetTickCount();
    while (1) {
        /* notify system monitoring that task will be called */
        SYSM_Notify(SYSM_TASK_ID_CYCLIC_1ms, SYSM_NOTIFY_ENTER, OS_GetTickCount());
        /* user code implementation */
        FTSK_RunUserCodeCyclic1ms();
        /* notify system monitoring that task has been called */
        SYSM_Notify(SYSM_TASK_ID_CYCLIC_1ms, SYSM_NOTIFY_EXIT, OS_GetTickCount());
        /* let task sleep until it is due again */
        OS_DelayTaskUntil(&current_time, ftsk_taskDefinitionCyclic1ms.cycleTime);
    }
}

/* tell compiler this function is a task, context save not necessary */
#pragma TASK(FTSK_CreateTaskCyclic10ms)
static void FTSK_CreateTaskCyclic10ms(void) {
    uint32_t current_time = 0;

    while (os_boot != OS_PRECYCLIC_INIT_HAS_FINISHED) {
    }

    OS_DelayTaskUntil(&os_schedulerStartTime, ftsk_taskDefinitionCyclic10ms.phase);
    current_time = OS_GetTickCount();
    while (1) {
        /* notify system monitoring that task will be called */
        SYSM_Notify(SYSM_TASK_ID_CYCLIC_10ms, SYSM_NOTIFY_ENTER, OS_GetTickCount());
        /* user code implementation */
        FTSK_RunUserCodeCyclic10ms();
        /* notify system monitoring that task has been called */
        SYSM_Notify(SYSM_TASK_ID_CYCLIC_10ms, SYSM_NOTIFY_EXIT, OS_GetTickCount());
        /* let task sleep until it is due again */
        OS_DelayTaskUntil(&current_time, ftsk_taskDefinitionCyclic10ms.cycleTime);
    }
}

/* tell compiler this function is a task, context save not necessary */
#pragma TASK(FTSK_CreateTaskCyclic100ms)
static void FTSK_CreateTaskCyclic100ms(void) {
    uint32_t current_time = 0;

    while (os_boot != OS_PRECYCLIC_INIT_HAS_FINISHED) {
    }

    OS_DelayTaskUntil(&os_schedulerStartTime, ftsk_taskDefinitionCyclic100ms.phase);
    current_time = OS_GetTickCount();
    while (1) {
        /* notify system monitoring that task will be called */
        SYSM_Notify(SYSM_TASK_ID_CYCLIC_100ms, SYSM_NOTIFY_ENTER, OS_GetTickCount());
        /* user code implementation */
        FTSK_RunUserCodeCyclic100ms();
        /* notify system monitoring that task has been called */
        SYSM_Notify(SYSM_TASK_ID_CYCLIC_100ms, SYSM_NOTIFY_EXIT, OS_GetTickCount());
        /* let task sleep until it is due again */
        OS_DelayTaskUntil(&current_time, ftsk_taskDefinitionCyclic100ms.cycleTime);
    }
}

/* tell compiler this function is a task, context save not necessary */
#pragma TASK(FTSK_CreateTaskCyclicAlgorithm100ms)
static void FTSK_CreateTaskCyclicAlgorithm100ms(void) {
    uint32_t current_time = 0;

    while (os_boot != OS_PRECYCLIC_INIT_HAS_FINISHED) {
    }

    OS_DelayTaskUntil(&os_schedulerStartTime, ftsk_taskDefinitionCyclicAlgorithm100ms.phase);
    os_boot      = OS_SYSTEM_RUNNING;
    current_time = OS_GetTickCount();
    while (1) {
        /* notify system monitoring that task will be called */
        SYSM_Notify(SYSM_TASK_ID_CYCLIC_ALGORITHM_100ms, SYSM_NOTIFY_ENTER, OS_GetTickCount());
        /* user code implementation */
        FTSK_RunUserCodeCyclicAlgorithm100ms();
        /* notify system monitoring that task has been called */
        SYSM_Notify(SYSM_TASK_ID_CYCLIC_ALGORITHM_100ms, SYSM_NOTIFY_EXIT, OS_GetTickCount());
        /* let task sleep until it is due again */
        OS_DelayTaskUntil(&current_time, ftsk_taskDefinitionCyclicAlgorithm100ms.cycleTime);
    }
}

/*========== Extern Function Implementations ================================*/
extern void FTSK_CreateQueues(void) {
    /* Create a queue capable of containing a pointer of type DATA_QUEUE_MESSAGE_s
    Data of Messages are passed by pointer as they contain a lot of data. */
    ftsk_databaseQueue = xQueueCreateStatic(
        FTSK_DATABASE_QUEUE_LENGTH,
        FTSK_DATABASE_QUEUE_ITEM_SIZE,
        ftsk_databaseQueueStorageArea,
        &ftsk_databaseQueueStructure);
    FAS_ASSERT(ftsk_databaseQueue != NULL_PTR);
    vQueueAddToRegistry(ftsk_databaseQueue, "Database Queue");
    ftsk_imdCanDataQueue = xQueueCreateStatic(
        FTSK_IMD_QUEUE_LENGTH, FTSK_IMD_QUEUE_ITEM_SIZE, ftsk_imdQueueStorageArea, &ftsk_imdQueueStructure);
    vQueueAddToRegistry(ftsk_imdCanDataQueue, "IMD CAN Data Queue");
    FAS_ASSERT(ftsk_imdCanDataQueue != NULL_PTR);
    ftsk_canRxQueue = xQueueCreateStatic(
        FTSK_CAN_RX_QUEUE_LENGTH, FTSK_CAN_RX_QUEUE_ITEM_SIZE, ftsk_canRxQueueStorageArea, &ftsk_canRxQueueStructure);
    vQueueAddToRegistry(ftsk_canRxQueue, "CAN Receive Queue");
    FAS_ASSERT(ftsk_canRxQueue != NULL_PTR);
    OS_EnterTaskCritical();
    ftsk_allQueuesCreated = true;
    OS_ExitTaskCritical();
}

extern void FTSK_CreateTasks(void) {
    ftsk_taskHandleEngine = xTaskCreateStatic(
        (TaskFunction_t)FTSK_CreateTaskEngine,
        (const portCHAR *)"FTSK_CreateTaskEngine",
        ftsk_taskDefinitionEngine.stackSize,
        NULL,
        ftsk_taskDefinitionEngine.priority,
        ftsk_stackSizeEngine,
        &ftsk_taskEngine);
    /* Trap if initialization failed */
    FAS_ASSERT(ftsk_taskHandleEngine != NULL);

    /* Cyclic Task 1ms */
    ftsk_taskHandleCyclic1ms = xTaskCreateStatic(
        (TaskFunction_t)FTSK_CreateTaskCyclic1ms,
        (const portCHAR *)"FTSK_CreateTaskCyclic1ms",
        ftsk_taskDefinitionCyclic1ms.stackSize,
        NULL,
        ftsk_taskDefinitionCyclic1ms.priority,
        ftsk_stackSizeCyclic1ms,
        &ftsk_taskCyclic1ms);
    /* Trap if initialization failed */
    FAS_ASSERT(ftsk_taskHandleCyclic1ms != NULL);

    /* Cyclic Task 10ms */
    ftsk_taskHandleCyclic10ms = xTaskCreateStatic(
        (TaskFunction_t)FTSK_CreateTaskCyclic10ms,
        (const portCHAR *)"FTSK_CreateTaskCyclic10ms",
        ftsk_taskDefinitionCyclic10ms.stackSize,
        NULL,
        ftsk_taskDefinitionCyclic10ms.priority,
        ftsk_stackSizeCyclic10ms,
        &ftsk_taskCyclic10ms);
    /* Trap if initialization failed */
    FAS_ASSERT(ftsk_taskHandleCyclic10ms != NULL);

    /* Cyclic Task 100ms */
    ftsk_taskHandleCyclic100ms = xTaskCreateStatic(
        (TaskFunction_t)FTSK_CreateTaskCyclic100ms,
        (const portCHAR *)"FTSK_CreateTaskCyclic100ms",
        ftsk_taskDefinitionCyclic100ms.stackSize,
        NULL,
        ftsk_taskDefinitionCyclic100ms.priority,
        ftsk_stackSizeCyclic100ms,
        &ftsk_taskCyclic100ms);
    /* Trap if initialization failed */
    FAS_ASSERT(ftsk_taskHandleCyclic100ms != NULL);

    /* Cyclic Task 100ms for algorithms */
    ftsk_taskHandleCyclicAlgorithm100ms = xTaskCreateStatic(
        (TaskFunction_t)FTSK_CreateTaskCyclicAlgorithm100ms,
        (const portCHAR *)"FTSK_CreateTaskCyclicAlgorithm100ms",
        ftsk_taskDefinitionCyclicAlgorithm100ms.stackSize,
        NULL,
        ftsk_taskDefinitionCyclicAlgorithm100ms.priority,
        ftsk_stackSizeCyclicAlgorithm100ms,
        &ftsk_taskCyclicAlgorithm100ms);
    /* Trap if initialization failed */
    FAS_ASSERT(ftsk_taskHandleCyclicAlgorithm100ms != NULL);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
