/**
 *
 * @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der
 *  angewandten Forschung e.V. All rights reserved.
 *
 * BSD 3-Clause License
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1.  Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * We kindly request you to use one or more of the following phrases to refer
 * to foxBMS in your hardware, software, documentation or advertising
 * materials:
 *
 * &Prime;This product uses parts of foxBMS&reg;&Prime;
 *
 * &Prime;This product includes parts of foxBMS&reg;&Prime;
 *
 * &Prime;This product is derived from foxBMS&reg;&Prime;
 *
 */

/**
 * @file    ftask.c
 * @author  foxBMS Team
 * @date    2019-08-27 (date of creation)
 * @updated 2020-01-21 (date of last update)
 * @ingroup TASK
 * @prefix  FTSK
 *
 * @brief   Implementation of engine task driver
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "ftask.h"

#include "FreeRTOS.h"
#include "stream_buffer.h"

#include "database.h"
#include "sys_mon.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/********* Engine Task *****************************************************/
/** @brief Definition of task handle of the engine task */
static TaskHandle_t ftsk_taskHandleEngine;

/** @brief Task Struct for #ftsk_taskHandleEngine */
static StaticTask_t ftsk_taskStructEngine;

/** @brief Stack of #ftsk_taskHandleEngine */
static StackType_t ftsk_stackSizeEngine[FTSK_TSK_ENGINE_STACK_SIZE];

/********* Cyclic 1 ms Task *************************************************/
/** @brief Definition of task handle for the cyclic 1 ms task */
static TaskHandle_t ftsk_taskHandleCyclic1ms;

/** @brief Task Struct for #ftsk_taskHandleCyclic1ms */
static StaticTask_t ftsk_taskStructCyclic1ms;

/** @brief Stack of #ftsk_taskHandleCyclic1ms */
static StackType_t ftsk_stackSizeCyclic1ms[FTSK_TSK_CYCLIC_1MS_STACK_SIZE];

/********* Cyclic 10 ms Task ************************************************/
/** @brief Definition of task handle for the cyclic 10 ms task */
static TaskHandle_t ftsk_taskHandleCyclic10ms;

/** @brief Task Struct for #ftsk_taskHandleCyclic10ms */
static StaticTask_t ftsk_taskStructCyclic10ms;

/** @brief Stack of #ftsk_taskHandleCyclic10ms */
static StackType_t ftsk_stackSizeCyclic10ms[FTSK_TSK_CYCLIC_10MS_STACK_SIZE];

/********* Cyclic 100 ms Task ***********************************************/
/** @brief Definition of task handle for the cyclic 100 ms task */
static TaskHandle_t ftsk_taskHandleCyclic100ms;

/** @brief Task Struct for #ftsk_taskHandleCyclic100ms */
static StaticTask_t ftsk_taskStructCyclic100ms;

/** @brief Stack of #ftsk_taskHandleCyclic100ms */
static StackType_t ftsk_stackSizeCyclic100ms[FTSK_TSK_CYCLIC_100MS_STACK_SIZE];

/********* Cyclic 100 ms Task for Algorithms ********************************/
/** @brief Definition of task handle for the cyclic 100 ms task for algorithms */
static TaskHandle_t ftsk_taskHandleCyclicAlgorithm100ms;

/** @brief Task Struct for #ftsk_taskHandleCyclicAlgorithm100ms */
static StaticTask_t ftsk_taskStructCyclicAlgorithm100ms;

/** @brief Stack of #ftsk_taskHandleCyclicAlgorithm100ms */
static StackType_t ftsk_stackSizeCyclicAlgorithm100ms[FTSK_TSK_CYCLIC_ALGORITHM_100MS_STACKSIZE];

/*========== Static Function Prototypes =====================================*/
static void FTSK_TaskCreatorEngine(void);
static void FTSK_TaskCreatorCyclic1ms(void);
static void FTSK_TaskCreatorCyclic10ms(void);
static void FTSK_TaskCreatorCyclic100ms(void);
static void FTSK_TaskCreatorCyclicAlgorithm100ms(void);

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
void FTSK_CreateQueues(void) {
}

void FTSK_CreateMutexes(void) {
}

void FTSK_CreateEvents(void) {
}

void FTSK_CreateTasks(void) {
    ftsk_taskHandleEngine = xTaskCreateStatic(
        (TaskFunction_t)FTSK_TaskCreatorEngine,
        (const portCHAR *)"FTSK_TaskCreatorEngine",
        ftsk_taskDefinitionEngine.stackSize,
        NULL,
        ftsk_taskDefinitionEngine.priority,
        ftsk_stackSizeEngine,
        &ftsk_taskStructEngine);
    /* Trap if initialization failed */
    FAS_ASSERT(ftsk_taskHandleEngine != NULL);

    /* Cyclic Task 1ms */
    ftsk_taskHandleCyclic1ms = xTaskCreateStatic(
        (TaskFunction_t)FTSK_TaskCreatorCyclic1ms,
        (const portCHAR *)"FTSK_TaskCreatorCyclic1ms",
        ftsk_taskDefinitionCyclic1ms.stackSize,
        NULL,
        ftsk_taskDefinitionCyclic1ms.priority,
        ftsk_stackSizeCyclic1ms,
        &ftsk_taskStructCyclic1ms);
    /* Trap if initialization failed */
    FAS_ASSERT(ftsk_taskHandleCyclic1ms != NULL);

    /* Cyclic Task 10ms */
    ftsk_taskHandleCyclic10ms = xTaskCreateStatic(
        (TaskFunction_t)FTSK_TaskCreatorCyclic10ms,
        (const portCHAR *)"FTSK_TaskCreatorCyclic10ms",
        ftsk_taskDefinitionCyclic10ms.stackSize,
        NULL,
        ftsk_taskDefinitionCyclic10ms.priority,
        ftsk_stackSizeCyclic10ms,
        &ftsk_taskStructCyclic10ms);
    /* Trap if initialization failed */
    FAS_ASSERT(ftsk_taskHandleCyclic10ms != NULL);

    /* Cyclic Task 100ms */
    ftsk_taskHandleCyclic100ms = xTaskCreateStatic(
        (TaskFunction_t)FTSK_TaskCreatorCyclic100ms,
        (const portCHAR *)"FTSK_TaskCreatorCyclic100ms",
        ftsk_taskDefinitionCyclic100ms.stackSize,
        NULL,
        ftsk_taskDefinitionCyclic100ms.priority,
        ftsk_stackSizeCyclic100ms,
        &ftsk_taskStructCyclic100ms);
    /* Trap if initialization failed */
    FAS_ASSERT(ftsk_taskHandleCyclic100ms != NULL);

    /* Cyclic Task 100ms for algorithms */
    ftsk_taskHandleCyclicAlgorithm100ms = xTaskCreateStatic(
        (TaskFunction_t)FTSK_TaskCreatorCyclicAlgorithm100ms,
        (const portCHAR *)"FTSK_TaskCreator_Cyclic_Algorithm_100ms",
        ftsk_taskDefinitionCyclicAlgorithm100ms.stackSize,
        NULL,
        ftsk_taskDefinitionCyclicAlgorithm100ms.priority,
        ftsk_stackSizeCyclicAlgorithm100ms,
        &ftsk_taskStructCyclicAlgorithm100ms);
    /* Trap if initialization failed */
    FAS_ASSERT(ftsk_taskHandleCyclicAlgorithm100ms != NULL);
}

/**
 * @brief   Database-Task
 * @details The task manages the data exchange with the database and must have a
 *          higher task priority than any task using the database.
 * @ingroup API_OS
 */
/* tell compiler this function is a task, context save not necessary */
#pragma TASK(FTSK_TaskCreatorEngine)
void FTSK_TaskCreatorEngine(void) {
    os_boot = OS_SCHEDULER_RUNNING;
    FTSK_UserCodeEngineInit();
    os_boot = OS_ENGINE_RUNNING;

    OS_DelayTaskUntil(&os_schedulerStartTime, ftsk_taskDefinitionEngine.phase);
    while (1) {
        /* notify system monitoring that task will be called */
        SYSM_Notify(SYSM_TASK_ID_ENGINE, SYSM_NOTIFY_ENTER, OS_GetTickCount());
        /* user code implemention */
        FTSK_UserCodeEngine();
        /* notify system monitoring that task has been called */
        SYSM_Notify(SYSM_TASK_ID_ENGINE, SYSM_NOTIFY_EXIT, OS_GetTickCount());
    }
}

/**
 * @brief   Creation of cyclic 1 ms engine task
 * @details The Task calls OsStartUp() in the very beginning, this is the first
 *          active Task. Then the Task is delayed by a phase as defined in
 *          ftsk_tskdef_cyclic_1ms.phase (in milliseconds). After the phase
 *          delay, the cyclic execution starts, the entry time is saved in
 *          current_time. After one cycle, the Task is set to sleep until entry
 *          time + ftsk_tskdef_cyclic_1ms.cycleTime (in milliseconds).
 */
/* tell compiler this function is a task, context save not necessary */
#pragma TASK(FTSK_TaskCreatorCyclic1ms)
void FTSK_TaskCreatorCyclic1ms(void) {
    uint32_t current_time = 0;

    while (os_boot != OS_ENGINE_RUNNING) {
    }

    FTSK_UserCodePreCyclicTasksInitialization();
    os_boot = OS_PRECYCLIC_INIT_HAS_FINISHED;

    OS_DelayTaskUntil(&os_schedulerStartTime, ftsk_taskDefinitionCyclic1ms.phase);
    current_time = OS_GetTickCount();
    while (1) {
        /* notify system monitoring that task will be called */
        SYSM_Notify(SYSM_TASK_ID_CYCLIC_1ms, SYSM_NOTIFY_ENTER, OS_GetTickCount());
        /* user code implementation */
        FTSK_UserCodeCyclic1ms();
        /* notify system monitoring that task has been called */
        SYSM_Notify(SYSM_TASK_ID_CYCLIC_1ms, SYSM_NOTIFY_EXIT, OS_GetTickCount());
        /* task statistics */
        OS_DelayTaskUntil(&current_time, ftsk_taskDefinitionCyclic1ms.cycleTime);
    }
}

/**
 * @brief   Creation of cyclic 10 ms engine task
 * @details Task is delayed by a phase as defined in
 *          ftsk_tskdef_cyclic_10ms.phase (in milliseconds). After the phase
 *          delay, the cyclic execution starts, the entry time is saved in
 *          current_time. After one cycle, the Task is set to sleep until entry
 *          time + ftsk_tskdef_cyclic_10ms.cycleTime (in milliseconds).
 */
/* tell compiler this function is a task, context save not necessary */
#pragma TASK(FTSK_TaskCreatorCyclic10ms)
void FTSK_TaskCreatorCyclic10ms(void) {
    uint32_t current_time = 0;

    while (os_boot != OS_PRECYCLIC_INIT_HAS_FINISHED) {
    }

    OS_DelayTaskUntil(&os_schedulerStartTime, ftsk_taskDefinitionCyclic10ms.phase);
    current_time = OS_GetTickCount();
    while (1) {
        /* notify system monitoring that task will be called */
        SYSM_Notify(SYSM_TASK_ID_CYCLIC_10ms, SYSM_NOTIFY_ENTER, OS_GetTickCount());
        /* user code implementation */
        FTSK_UserCodeCyclic10ms();
        /* notify system monitoring that task has been called */
        SYSM_Notify(SYSM_TASK_ID_CYCLIC_10ms, SYSM_NOTIFY_EXIT, OS_GetTickCount());
        /* task statistics */
        OS_DelayTaskUntil(&current_time, ftsk_taskDefinitionCyclic10ms.cycleTime);
    }
}

/**
 * @brief   Creation of cyclic 100 ms engine task
 * @details Task is delayed by a phase as defined in
 *          ftsk_tskdef_cyclic_100ms.phase (in milliseconds). After the phase
 *          delay, the cyclic execution starts, the entry time is saved in
 *          current_time. After one cycle, the Task is set to sleep until entry
 *          time + ftsk_tskdef_cyclic_100ms.cycleTime (in milliseconds).
 */
/* tell compiler this function is a task, context save not necessary */
#pragma TASK(FTSK_TaskCreatorCyclic100ms)
void FTSK_TaskCreatorCyclic100ms(void) {
    uint32_t current_time = 0;

    while (os_boot != OS_PRECYCLIC_INIT_HAS_FINISHED) {
    }

    OS_DelayTaskUntil(&os_schedulerStartTime, ftsk_taskDefinitionCyclic100ms.phase);
    current_time = OS_GetTickCount();
    while (1) {
        /* notify system monitoring that task will be called */
        SYSM_Notify(SYSM_TASK_ID_CYCLIC_100ms, SYSM_NOTIFY_ENTER, OS_GetTickCount());
        /* user code implementation */
        FTSK_UserCodeCyclic100ms();
        /* notify system monitoring that task has been called */
        SYSM_Notify(SYSM_TASK_ID_CYCLIC_100ms, SYSM_NOTIFY_EXIT, OS_GetTickCount());
        /* task statistics */
        OS_DelayTaskUntil(&current_time, ftsk_taskDefinitionCyclic100ms.cycleTime);
    }
}

/**
 * @brief   Creation of cyclic 100 ms algorithm task
 * @details Task is delayed by a phase as defined in
 *          ftsk_taskDefinitionCyclicAlgorithm100ms.Phase (in milliseconds). After
 *          the phase delay, the cyclic execution starts, the entry time is
 *          saved in current_time. After one cycle, the Task is set to sleep
 *          until entry time + ftsk_taskDefinitionCyclicAlgorithm100ms.CycleTime
 *          (in milliseconds).
 */
/* tell compiler this function is a task, context save not necessary */
#pragma TASK(FTSK_TaskCreatorCyclicAlgorithm100ms)
void FTSK_TaskCreatorCyclicAlgorithm100ms(void) {
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
        FTSK_UserCodeCyclicAlgorithm100ms();
        /* notify system monitoring that task has been called */
        SYSM_Notify(SYSM_TASK_ID_CYCLIC_ALGORITHM_100ms, SYSM_NOTIFY_EXIT, OS_GetTickCount());
        /* task statistics */
        OS_DelayTaskUntil(&current_time, ftsk_taskDefinitionCyclicAlgorithm100ms.cycleTime);
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
