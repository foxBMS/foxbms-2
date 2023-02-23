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
 * @file    ftask.h
 * @author  foxBMS Team
 * @date    2019-08-27 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup TASK
 * @prefix  FTSK
 *
 * @brief   Header of task driver implementation
 * @details Declares the functions that are need to needed to initialize the
 *          operating system. This includes queues, mutexes, events and tasks.
 */

#ifndef FOXBMS__FTASK_H_
#define FOXBMS__FTASK_H_

/*========== Includes =======================================================*/
#include "can_cfg.h"
#include "ftask_cfg.h"

#include "database.h"
#include "os.h"
#include "rtc.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** Length of queue that is used in the database */
#define FTSK_DATABASE_QUEUE_LENGTH (1u)

/** Size of queue item that is used in the database */
#define FTSK_DATABASE_QUEUE_ITEM_SIZE_IN_BYTES (sizeof(DATA_QUEUE_MESSAGE_s))

/** Length of queue that is used in the insulation measurement device (IMD) */
#define FTSK_IMD_QUEUE_LENGTH (5u)
/** Size of queue item that is used in the IMD driver */
#define FTSK_IMD_QUEUE_ITEM_SIZE_IN_BYTES (sizeof(CAN_BUFFER_ELEMENT_s))

/** Length of queue that is used in the can module for receiving messages */
#define FTSK_CAN_RX_QUEUE_LENGTH (50u)
/** Size of queue item that is used in the can driver */
#define FTSK_CAN_RX_QUEUE_ITEM_SIZE_IN_BYTES (sizeof(CAN_BUFFER_ELEMENT_s))

#define FTSK_AFE_REQUEST_QUEUE_LENGTH    (1u)
#define FTSK_AFE_REQUEST_QUEUE_ITEM_SIZE (sizeof(AFE_REQUEST_e))

/** Length of queue that is used in the can module for receiving messages */
#define FTSK_RTC_QUEUE_LENGTH (1u)
/** Size of queue item that is used in the can driver */
#define FTSK_RTC_QUEUE_ITEM_SIZE_IN_BYTES (sizeof(RTC_TIME_DATA_s))

/** Length of queue that is used for I2C transmission over NXP slave */
#define FTSK_AFEI2C_QUEUE_LENGTH (1u)
/** Size of queue item that is used for I2C transmission over NXP slave */
#define FTSK_AFEI2C_QUEUE_ITEM_SIZE_IN_BYTES (sizeof(AFE_I2C_QUEUE_s))

/*========== Extern Constant and Variable Declarations ======================*/
/** database queue */
extern OS_QUEUE ftsk_databaseQueue;

/** queue for CAN based IMD devices */
extern OS_QUEUE ftsk_imdCanDataQueue;

/** CAN driver data queue for RX messages */
extern OS_QUEUE ftsk_canRxQueue;

/** handle of the AFE driver request queue */
extern OS_QUEUE ftsk_afeRequestQueue;

/** handle of the rtc driver data queue */
extern OS_QUEUE ftsk_rtcSetTimeQueue;

/** handle of the I2C transmission over NXP slave queue */
extern OS_QUEUE ftsk_afeToI2cQueue;
extern OS_QUEUE ftsk_afeFromI2cQueue;

/** indicator whether the queues have successfully been initialized to be used
 * in other parts of the software  */
extern volatile bool ftsk_allQueuesCreated;

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   Creates all queues
 * @details Creates all queues. Is called after the hardware is initialized
 *          and before the scheduler starts. Queues, Mutexes and Events are
 *          already initialized.
 */
extern void FTSK_CreateQueues(void);

/**
 * @brief   Creates all tasks of the group
 * @details Creates all tasks. Is called after the hardware is initialized
 *          and before the scheduler starts. Queues, Mutexes and Events are
 *          already initialized.
 */
extern void FTSK_CreateTasks(void);

/**
 * @brief   Database-Task
 * @details The task manages the data exchange with the database and must have
 *          a higher task priority than any task using the database.
 * @param   pvParameters parameter for the to task
 */
extern void FTSK_CreateTaskEngine(void *const pvParameters);

/**
 * @brief   Creation of cyclic 1 ms task
 * @details Then the Task is delayed by a phase as defined in
 *          ftsk_taskDefinitionCyclic1ms.phase (in milliseconds). After the
 *          phase delay, the cyclic execution starts, the entry time is saved
 *          in current_time. After one cycle, the Task is set to sleep until
 *          entry time + ftsk_taskDefinitionCyclic1ms.cycleTime (in
 *          milliseconds).
 * @param   pvParameters parameter for the to task
 */
extern void FTSK_CreateTaskCyclic1ms(void *const pvParameters);

/**
 * @brief   Creation of cyclic 10 ms task
 * @details Task is delayed by a phase as defined in
 *          ftsk_taskDefinitionCyclic10ms.phase (in milliseconds). After
 *          the phase delay, the cyclic execution starts, the entry time is
 *          saved in current_time. After one cycle, the Task is set to sleep
 *          until entry time + ftsk_taskDefinitionCyclic10ms.cycleTime (in
 *          milliseconds).
 * @param   pvParameters parameter for the to task
 */
extern void FTSK_CreateTaskCyclic10ms(void *const pvParameters);

/**
 * @brief   Creation of cyclic 100 ms task
 * @details Task is delayed by a phase as defined in
 *          ftsk_taskDefinitionCyclic100ms.phase (in milliseconds). After the
 *          phase delay, the cyclic execution starts, the entry time is saved
 *          in current_time. After one cycle, the Task is set to sleep until
 *          entry time + ftsk_taskDefinitionCyclic100ms.cycleTime (in
 *          milliseconds).
 * @param   pvParameters parameter for the to task
 */
extern void FTSK_CreateTaskCyclic100ms(void *const pvParameters);

/**
 * @brief   Creation of cyclic 100 ms algorithm task
 * @details Task is delayed by a phase as defined in
 *          ftsk_taskDefinitionCyclicAlgorithm100ms.Phase (in milliseconds).
 *          After the phase delay, the cyclic execution starts, the entry time
 *          is saved in current_time. After one cycle, the Task is set to sleep
 *          until entry
 *          time + ftsk_taskDefinitionCyclicAlgorithm100ms.CycleTime (in
 *          milliseconds).
 * @param   pvParameters parameter for the to task
 */
extern void FTSK_CreateTaskCyclicAlgorithm100ms(void *const pvParameters);

/**
 * @brief   Creation of continuously running task for I2c
 */
extern void FTSK_CreateTaskI2c(void *const pvParameters);

/**
 * @brief   Creation of continuously running task for AFEs
 * @param   pvParameters parameter for the to task
 */
extern void FTSK_CreateTaskAfe(void *const pvParameters);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__FTASK_H_ */
