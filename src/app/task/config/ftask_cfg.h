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
 * @file    ftask_cfg.h
 * @author  foxBMS Team
 * @date    2019-08-26 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup TASK_CONFIGURATION
 * @prefix  FTSK
 *
 * @brief   Task configuration header
 * @details TODO
 */

#ifndef FOXBMS__FTASK_CFG_H_
#define FOXBMS__FTASK_CFG_H_

/*========== Includes =======================================================*/

#include "os.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** @brief Stack size of engine task */
#define FTSK_TASK_ENGINE_STACK_SIZE_IN_BYTES (1024u)

/** @brief Phase of engine task */
#define FTSK_TASK_ENGINE_PRIORITY (OS_PRIORITY_REAL_TIME)

/** @brief Phase of engine task */
#define FTSK_TASK_ENGINE_PHASE (0u)

/** @brief Cycle time of engine task */
#define FTSK_TASK_ENGINE_CYCLE_TIME (1u)

/** @brief Maximum allowed jitter of engine task */
#define FTSK_TASK_ENGINE_MAXIMUM_JITTER (1u)

/** @brief pvParameters of the engine task */
#define FTSK_TASK_ENGINE_PV_PARAMETERS (NULL_PTR)

/** @brief Stack size of cyclic 1 ms task */
#define FTSK_TASK_CYCLIC_1MS_STACK_SIZE_IN_BYTES (1024u)

/** @brief Priority of cyclic 1ms task */
#define FTSK_TASK_CYCLIC_1MS_PRIORITY (OS_PRIORITY_VERY_HIGH)

/** @brief Phase of cyclic 1ms task */
#define FTSK_TASK_CYCLIC_1MS_PHASE (0u)

/** @brief Cycle time of 1ms task */
#define FTSK_TASK_CYCLIC_1MS_CYCLE_TIME (1u)

/** @brief Maximum allowed jitter of 1ms task */
#define FTSK_TASK_CYCLIC_1MS_MAXIMUM_JITTER (1u)

/** @brief pvParameters of the 1ms task */
#define FTSK_TASK_CYCLIC_1MS_PV_PARAMETERS (NULL_PTR)

/** @brief Stack size of cyclic 10 ms task */
#define FTSK_TASK_CYCLIC_10MS_STACK_SIZE_IN_BYTES (5120u)

/** @brief Priority of cyclic 10 ms task */
#define FTSK_TASK_CYCLIC_10MS_PRIORITY (OS_PRIORITY_HIGH)

/** @brief Phase of cyclic 10 ms task */
#define FTSK_TASK_CYCLIC_10MS_PHASE (2u)

/** @brief Cycle time of 10 ms task */
#define FTSK_TASK_CYCLIC_10MS_CYCLE_TIME (10u)

/** @brief Maximum allowed jitter of 10ms task */
#define FTSK_TASK_CYCLIC_10MS_MAXIMUM_JITTER (2u)

/** @brief pvParameters of the 10ms task */
#define FTSK_TASK_CYCLIC_10MS_PV_PARAMETERS (NULL_PTR)

/** @brief Stack size of cyclic 100 ms task */
#define FTSK_TASK_CYCLIC_100MS_STACK_SIZE_IN_BYTES (1024u)

/** @brief Priority of cyclic 100 ms task */
#define FTSK_TASK_CYCLIC_100MS_PRIORITY (OS_PRIORITY_ABOVE_NORMAL)

/** @brief Phase of cyclic 100 ms task */
#define FTSK_TASK_CYCLIC_100MS_PHASE (56u)

/** @brief Cycle time of 100ms task */
#define FTSK_TASK_CYCLIC_100MS_CYCLE_TIME (100u)

/** @brief Maximum allowed jitter of 100ms task */
#define FTSK_TASK_CYCLIC_100MS_MAXIMUM_JITTER (5u)

/** @brief pvParameters of the 100ms task */
#define FTSK_TASK_CYCLIC_100MS_PV_PARAMETERS (NULL_PTR)

/** @brief Stack size of cyclic 100 ms task for algorithms */
#define FTSK_TASK_CYCLIC_ALGORITHM_100MS_STACK_SIZE_IN_BYTES (1024u)

/** @brief Priority of cyclic 100 ms task for algorithms */
#define FTSK_TASK_CYCLIC_ALGORITHM_100MS_PRIORITY (OS_PRIORITY_NORMAL)

/** @brief Phase of cyclic 100 ms task for algorithms */
#define FTSK_TASK_CYCLIC_ALGORITHM_100MS_PHASE (64u)

/** @brief Cycle time of 100ms task for algorithms*/
#define FTSK_TASK_CYCLIC_ALGORITHM_100MS_CYCLE_TIME (100u)

/** @brief Maximum allowed jitter of 100ms task for algorithms */
#define FTSK_TASK_CYCLIC_ALGORITHM_100MS_MAXIMUM_JITTER (5u)

/** @brief pvParameters of the 100ms task for algorithms  */
#define FTSK_TASK_CYCLIC_ALGORITHM_100MS_PV_PARAMETERS (NULL_PTR)

/** @brief Stack size of continuously running task for I2C */
#define FTSK_TASK_I2C_STACK_SIZE_IN_BYTES (2048u / 4u)

/** @brief Priority of continuously running task for I2C */
#define FTSK_TASK_I2C_PRIORITY (FTSK_TASK_CYCLIC_10MS_PRIORITY)

/** @brief Phase of continuously running task for I2C */
#define FTSK_TASK_I2C_PHASE (0u)

/** @brief Cycle time of continuously running task for I2C */
#define FTSK_TASK_I2C_CYCLE_TIME (0u)

/** @brief Maximum allowed jitter of continuously running task for I2C */
#define FTSK_TASK_AFE_MAXIMUM_JITTER (5u)

/** @brief pvParameters of the continuously running task for I2C  */
#define FTSK_TASK_I2C_PV_PARAMETERS (NULL_PTR)

/** @brief Stack size of continuously running task for AFEs */
#define FTSK_TASK_AFE_STACK_SIZE_IN_BYTES (4096u / 4u)

/** @brief Priority of continuously running task for AFEs */
#define FTSK_TASK_AFE_PRIORITY (OS_PRIORITY_ABOVE_HIGH)

/** @brief Phase of continuously running task for AFEs */
#define FTSK_TASK_AFE_PHASE (0u)

/** @brief Cycle time of continuously running task for AFEs */
#define FTSK_TASK_AFE_CYCLE_TIME (0u)

/** @brief Maximum allowed jitter of continuously running task for AFEs */
#define FTSK_TASK_AFE_MAXIMUM_JITTER (5u)

/** @brief pvParameters of the continuously running task for AFEs  */
#define FTSK_TASK_AFE_PV_PARAMETERS (NULL_PTR)

/*========== Extern Constant and Variable Declarations ======================*/
/**
 * @brief   Task configuration of the engine task
 * @details Task for database and system monitoring
 */
extern OS_TASK_DEFINITION_s ftsk_taskDefinitionEngine;

/**
 * @brief   Task configuration of the cyclic 1 ms task
 * @details Cyclic 1 ms task
 */
extern OS_TASK_DEFINITION_s ftsk_taskDefinitionCyclic1ms;

/**
 * @brief   Task configuration of the cyclic 10 ms task
 * @details Cyclic 10 ms task
 */
extern OS_TASK_DEFINITION_s ftsk_taskDefinitionCyclic10ms;

/**
 * @brief   Task configuration of the cyclic 100 ms task
 * @details Cyclic 100 ms task
 */
extern OS_TASK_DEFINITION_s ftsk_taskDefinitionCyclic100ms;

/**
 * @brief   Task configuration of the cyclic 100 ms task for algorithms
 * @details Cyclic 100 ms task for algorithms
 */
extern OS_TASK_DEFINITION_s ftsk_taskDefinitionCyclicAlgorithm100ms;

/**
 * @brief   Task configuration of the continuously running task for MCU I2C communication
 * @details Continuously running task for MCU I2C communication
 */
extern OS_TASK_DEFINITION_s ftsk_taskDefinitionI2c;

/**
 * @brief   Task configuration of the continuously running task for AFEs
 * @details Continuously running task for AFEs
 */
extern OS_TASK_DEFINITION_s ftsk_taskDefinitionAfe;

/**
 * @brief Definition of task handles
 */
extern TaskHandle_t ftsk_taskHandleI2c;

/**
 * @brief Definition of task handles
 */
extern OS_TASK_HANDLE ftsk_taskHandleAfe;

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   Initializes the database
 * @details Start up after scheduler starts
 * @warning Do not change the content of this function. This will very likely
 *          break the system. This function is kept in the configuration file
 *          to have a uniform task configuration.
 */
extern void FTSK_InitializeUserCodeEngine(void);

/**
 * @brief   Engine task for the database and the system monitoring module
 * @details Start up after scheduler start. First task to be run, all other
 *          tasks only starts when this task has started
 * @warning Do not change the content of this function. This will very likely
 *          break the system. This function is kept in the configuration file
 *          to have a uniform task configuration.
 */
extern void FTSK_RunUserCodeEngine(void);

/**
 * @brief   Initialization function before all tasks started
 * @details This function is called after the scheduler started but before any
 *          cyclic task runs. Here modules get initialized that are not used
 *          during the startup process.
 */
extern void FTSK_InitializeUserCodePreCyclicTasks(void);

/**
 * @brief   Cyclic 1 ms task
 * @details TODO
 */
extern void FTSK_RunUserCodeCyclic1ms(void);

/**
 * @brief   Cyclic 10 ms task
 * @details TODO
 */
extern void FTSK_RunUserCodeCyclic10ms(void);

/**
 * @brief   Cyclic 100 ms task
 * @details TODO
 */
extern void FTSK_RunUserCodeCyclic100ms(void);

/**
 * @brief   Cyclic 100 ms task for algorithms
 * @details TODO
 */
extern void FTSK_RunUserCodeCyclicAlgorithm100ms(void);

/**
 * @brief   Continuously running task for I2C
 * @details Implements the MCU communication over I2C
 */
extern void FTSK_RunUserCodeI2c(void);

/**
 * @brief   Continuously running task for AFEs
 * @details Implements the communications with AFEs without state machine.
 */
extern void FTSK_RunUserCodeAfe(void);

/**
 * @brief   Idle task
 * @details Called by #vApplicationIdleHook() if configUSE_IDLE_HOOK in
 *          FreeRTOSConfig.h is enabled. If you do not need this hook, you can
 *          disable it in the FreeRTOS configuration.
 */
extern void FTSK_RunUserCodeIdle(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__FTASK_CFG_H_ */
