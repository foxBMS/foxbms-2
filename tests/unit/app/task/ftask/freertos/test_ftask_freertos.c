/**
 *
 * @copyright &copy; 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * - "This product uses parts of foxBMS&reg;"
 * - "This product includes parts of foxBMS&reg;"
 * - "This product is derived from foxBMS&reg;"
 *
 */

/**
 * @file    test_ftask_freertos.c
 * @author  foxBMS Team
 * @date    2021-11-26 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the ftask implementation for FreeRTOS
 * @details Tests Create Task
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockftask_cfg.h"
#include "Mockmpu_prototypes.h"
#include "Mockos.h"
#include "Mocksys_mon.h"

#include "ftask.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("ftask_freertos.c")

TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/fram")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/engine/sys_mon")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/
/** size of storage area for the database queue */
#define FTSK_DATABASE_QUEUE_STORAGE_AREA (FTSK_DATABASE_QUEUE_LENGTH * FTSK_DATABASE_QUEUE_ITEM_SIZE_IN_BYTES)

/** helper macro to translate the stack sizes from bytes into words as FreeRTOS requires words and not bytes */
#define FTSK_BYTES_TO_WORDS(VARIABLE_IN_BYTES) ((VARIABLE_IN_BYTES) / GEN_BYTES_PER_WORD)
/** Stack size of engine task in words */
#define FTSK_TASK_ENGINE_STACK_SIZE_IN_WORDS FTSK_BYTES_TO_WORDS(FTSK_TASK_ENGINE_STACK_SIZE_IN_BYTES)
/** @brief Stack size of cyclic 1 ms task in words */
#define FTSK_TASK_CYCLIC_1MS_STACK_SIZE_IN_WORDS FTSK_BYTES_TO_WORDS(FTSK_TASK_CYCLIC_1MS_STACK_SIZE_IN_BYTES)
/** @brief Stack size of cyclic 10 ms task in words */
#define FTSK_TASK_CYCLIC_10MS_STACK_SIZE_IN_WORDS FTSK_BYTES_TO_WORDS(FTSK_TASK_CYCLIC_10MS_STACK_SIZE_IN_BYTES)
/** @brief Stack size of cyclic 100 ms task in words */
#define FTSK_TASK_CYCLIC_100MS_STACK_SIZE_IN_WORDS FTSK_BYTES_TO_WORDS(FTSK_TASK_CYCLIC_100MS_STACK_SIZE_IN_BYTES)
/** @brief Stack size of cyclic 100 ms task for algorithms in words */
#define FTSK_TASK_CYCLIC_ALGORITHM_100MS_STACK_SIZE_IN_WORDS \
    FTSK_BYTES_TO_WORDS(FTSK_TASK_CYCLIC_ALGORITHM_100MS_STACK_SIZE_IN_BYTES)
/** @brief Stack size of continuously running task for AFEs */
#define FTSK_TASK_AFE_STACK_SIZE_IN_WORDS FTSK_BYTES_TO_WORDS(FTSK_TASK_AFE_STACK_SIZE_IN_BYTES)

OS_TASK_HANDLE ftsk_taskHandleAfe;
OS_TASK_HANDLE ftsk_taskHandleI2c;

OS_TASK_DEFINITION_s ftsk_taskDefinitionEngine = {
    OS_PRIORITY_REAL_TIME,
    FTSK_TASK_ENGINE_PHASE,
    FTSK_TASK_ENGINE_CYCLE_TIME,
    FTSK_TASK_ENGINE_STACK_SIZE_IN_BYTES,
    FTSK_TASK_ENGINE_PV_PARAMETERS};
OS_TASK_DEFINITION_s ftsk_taskDefinitionCyclic1ms = {
    OS_PRIORITY_ABOVE_HIGH,
    FTSK_TASK_CYCLIC_1MS_PHASE,
    FTSK_TASK_CYCLIC_1MS_CYCLE_TIME,
    FTSK_TASK_CYCLIC_1MS_STACK_SIZE_IN_BYTES,
    FTSK_TASK_CYCLIC_1MS_PV_PARAMETERS};
OS_TASK_DEFINITION_s ftsk_taskDefinitionCyclic10ms = {
    OS_PRIORITY_HIGH,
    FTSK_TASK_CYCLIC_10MS_PHASE,
    FTSK_TASK_CYCLIC_10MS_CYCLE_TIME,
    FTSK_TASK_CYCLIC_10MS_STACK_SIZE_IN_BYTES,
    FTSK_TASK_CYCLIC_10MS_PV_PARAMETERS};
OS_TASK_DEFINITION_s ftsk_taskDefinitionCyclic100ms = {
    OS_PRIORITY_ABOVE_NORMAL,
    FTSK_TASK_CYCLIC_100MS_PHASE,
    FTSK_TASK_CYCLIC_100MS_CYCLE_TIME,
    FTSK_TASK_CYCLIC_100MS_STACK_SIZE_IN_BYTES,
    FTSK_TASK_CYCLIC_100MS_PV_PARAMETERS};
OS_TASK_DEFINITION_s ftsk_taskDefinitionCyclicAlgorithm100ms = {
    OS_PRIORITY_NORMAL,
    FTSK_TASK_CYCLIC_ALGORITHM_100MS_PHASE,
    FTSK_TASK_CYCLIC_ALGORITHM_100MS_CYCLE_TIME,
    FTSK_TASK_CYCLIC_ALGORITHM_100MS_STACK_SIZE_IN_BYTES,
    FTSK_TASK_CYCLIC_ALGORITHM_100MS_PV_PARAMETERS};
OS_TASK_DEFINITION_s ftsk_taskDefinitionI2c = {
    FTSK_TASK_I2C_PRIORITY,
    FTSK_TASK_I2C_PHASE,
    FTSK_TASK_I2C_CYCLE_TIME,
    FTSK_TASK_I2C_STACK_SIZE_IN_BYTES,
    FTSK_TASK_I2C_PV_PARAMETERS};
OS_TASK_DEFINITION_s ftsk_taskDefinitionAfe = {
    FTSK_TASK_AFE_PRIORITY,
    FTSK_TASK_AFE_PHASE,
    FTSK_TASK_AFE_CYCLE_TIME,
    FTSK_TASK_AFE_STACK_SIZE_IN_BYTES,
    FTSK_TASK_AFE_PV_PARAMETERS};

/** boot state of the OS */
volatile OS_BOOT_STATE_e os_boot = OS_OFF;
/** timestamp of the scheduler start */
uint32_t os_schedulerStartTime = 0u;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/** test correct queue creation */

/* database queue */
/*static uint8_t ftsk_databaseQueueStorageArea[FTSK_DATABASE_QUEUE_STORAGE_AREA] = {0};
static StaticQueue_t ftsk_databaseQueueStructure                               = {0};
void testFTSK_CreateQueues(void) {
    MPU_xQueueCreateStatic_Expect(
        FTSK_DATABASE_QUEUE_LENGTH,
        FTSK_DATABASE_QUEUE_ITEM_SIZE_IN_BYTES,
        ftsk_databaseQueueStorageArea,
        &ftsk_databaseQueueStructure);
}*/

/** test correct task creation */

/* Engine Task */
static StaticTask_t ftsk_taskEngine                                       = {0};
static StackType_t ftsk_stackEngine[FTSK_TASK_ENGINE_STACK_SIZE_IN_WORDS] = {0};
void testFTSK_CreateTasks(void) {
    OS_TASK_HANDLE dummyHandleSuccess = (OS_TASK_HANDLE)1u;
    MPU_xTaskCreateStatic_ExpectAndReturn(
        (TaskFunction_t)FTSK_CreateTaskEngine,
        "TaskEngine",
        FTSK_BYTES_TO_WORDS(ftsk_taskDefinitionEngine.stackSize_B),
        (void *)ftsk_taskDefinitionEngine.pvParameters,
        (UBaseType_t)ftsk_taskDefinitionEngine.priority,
        ftsk_stackEngine,
        &ftsk_taskEngine,
        dummyHandleSuccess);

    /* Cyclic Task 1ms */
    static StaticTask_t ftsk_taskCyclic1ms                                           = {0};
    static StackType_t ftsk_stackCyclic1ms[FTSK_TASK_CYCLIC_1MS_STACK_SIZE_IN_WORDS] = {0};
    MPU_xTaskCreateStatic_ExpectAndReturn(
        (TaskFunction_t)FTSK_CreateTaskCyclic1ms,
        "TaskCyclic1ms",
        FTSK_BYTES_TO_WORDS(ftsk_taskDefinitionCyclic1ms.stackSize_B),
        (void *)ftsk_taskDefinitionCyclic1ms.pvParameters,
        (UBaseType_t)ftsk_taskDefinitionCyclic1ms.priority,
        ftsk_stackCyclic1ms,
        &ftsk_taskCyclic1ms,
        dummyHandleSuccess);

    /* Cyclic Task 10ms */
    static StaticTask_t ftsk_taskCyclic10ms                                            = {0};
    static StackType_t ftsk_stackCyclic10ms[FTSK_TASK_CYCLIC_10MS_STACK_SIZE_IN_WORDS] = {0};
    MPU_xTaskCreateStatic_ExpectAndReturn(
        (TaskFunction_t)FTSK_CreateTaskCyclic10ms,
        "TaskCyclic10ms",
        FTSK_BYTES_TO_WORDS(ftsk_taskDefinitionCyclic10ms.stackSize_B),
        (void *)ftsk_taskDefinitionCyclic10ms.pvParameters,
        (UBaseType_t)ftsk_taskDefinitionCyclic10ms.priority,
        ftsk_stackCyclic10ms,
        &ftsk_taskCyclic10ms,
        dummyHandleSuccess);

    /* Cyclic Task 100ms */
    static StaticTask_t ftsk_taskCyclic100ms                                             = {0};
    static StackType_t ftsk_stackCyclic100ms[FTSK_TASK_CYCLIC_100MS_STACK_SIZE_IN_WORDS] = {0};
    MPU_xTaskCreateStatic_ExpectAndReturn(
        (TaskFunction_t)FTSK_CreateTaskCyclic100ms,
        "TaskCyclic100ms",
        FTSK_BYTES_TO_WORDS(ftsk_taskDefinitionCyclic100ms.stackSize_B),
        (void *)ftsk_taskDefinitionCyclic100ms.pvParameters,
        (UBaseType_t)ftsk_taskDefinitionCyclic100ms.priority,
        ftsk_stackCyclic100ms,
        &ftsk_taskCyclic100ms,
        dummyHandleSuccess);

    /* Cyclic Task 100ms for algorithms */
    static StaticTask_t ftsk_taskCyclicAlgorithm100ms                                                       = {0};
    static StackType_t ftsk_stackCyclicAlgorithm100ms[FTSK_TASK_CYCLIC_ALGORITHM_100MS_STACK_SIZE_IN_WORDS] = {0};
    MPU_xTaskCreateStatic_ExpectAndReturn(
        (TaskFunction_t)FTSK_CreateTaskCyclicAlgorithm100ms,
        "TaskCyclicAlgorithm100ms",
        FTSK_BYTES_TO_WORDS(ftsk_taskDefinitionCyclicAlgorithm100ms.stackSize_B),
        (void *)ftsk_taskDefinitionCyclicAlgorithm100ms.pvParameters,
        (UBaseType_t)ftsk_taskDefinitionCyclicAlgorithm100ms.priority,
        ftsk_stackCyclicAlgorithm100ms,
        &ftsk_taskCyclicAlgorithm100ms,
        dummyHandleSuccess);

    /* Continuously running Task for I2C */
    static StaticTask_t ftsk_taskI2c                                        = {0};
    static StackType_t ftsk_stackSizeI2c[FTSK_TASK_AFE_STACK_SIZE_IN_WORDS] = {0};
    MPU_xTaskCreateStatic_ExpectAndReturn(
        (TaskFunction_t)FTSK_CreateTaskI2c,
        "TaskI2c",
        FTSK_BYTES_TO_WORDS(ftsk_taskDefinitionI2c.stackSize_B),
        (void *)ftsk_taskDefinitionI2c.pvParameters,
        (UBaseType_t)ftsk_taskDefinitionI2c.priority,
        ftsk_stackSizeI2c,
        &ftsk_taskI2c,
        dummyHandleSuccess);

    /* Continuously running Task for AFE */
    static StaticTask_t ftsk_taskAfe                                        = {0};
    static StackType_t ftsk_stackSizeAfe[FTSK_TASK_AFE_STACK_SIZE_IN_WORDS] = {0};
    MPU_xTaskCreateStatic_ExpectAndReturn(
        (TaskFunction_t)FTSK_CreateTaskAfe,
        "TaskAfe",
        FTSK_BYTES_TO_WORDS(ftsk_taskDefinitionAfe.stackSize_B),
        (void *)ftsk_taskDefinitionAfe.pvParameters,
        (UBaseType_t)ftsk_taskDefinitionAfe.priority,
        ftsk_stackSizeAfe,
        &ftsk_taskAfe,
        dummyHandleSuccess);
    FTSK_CreateTasks();
}
