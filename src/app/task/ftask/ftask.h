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
 * @file    ftask.h
 * @author  foxBMS Team
 * @date    2019-08-27 (date of creation)
 * @updated 2020-01-21 (date of last update)
 * @ingroup TASK
 * @prefix  FTSK
 *
 * @brief   Header of task driver implementation
 * @details Declars the functions that are need to needed to initialize the
 *          operating system. This includes ques, mutexes, events and tasks.
 */

#ifndef FOXBMS__FTASK_H_
#define FOXBMS__FTASK_H_

/*========== Includes =======================================================*/
#include "ftask_cfg.h"

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   Creates all queues
 * @details Creates all queues. Is called after the hardware is initialized
 *          and before the scheduler starts. Queues, Mutexes and Events are
 *          already initialized.
 */
extern void FTSK_CreateQueues(void);

/**
 * @brief   Creates all mutexes
 * @details creates all mutexes needed for engine tasks and is called after
 *          the hardware is initialized and before the scheduler starts.
 *          Queues are already initialized.
 */
extern void FTSK_CreateMutexes(void);

/**
 * @brief   Creates all events of the group
 * @details Creates all events. Is called after the hardware is initialized
 *          and before the scheduler starts. Queues, Mutexes and Events are
 *          already initialized.
 */
extern void FTSK_CreateEvents(void);

/**
 * @brief   Creates all tasks of the group
 * @details Creates all tasks. Is called after the hardware is initialized
 *          and before the scheduler starts. Queues, Mutexes and Events are
 *          already initialized.
 */
extern void FTSK_CreateTasks(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__FTASK_H_ */
