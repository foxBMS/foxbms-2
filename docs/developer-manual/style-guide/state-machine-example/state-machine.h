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
 * @file    state-machine.h
 * @author  foxBMS Team
 * @date    2020-10-29 (date of creation)
 * @updated 2020-11-09 (date of last update)
 * @ingroup STATE_MACHINE
 * @prefix  EG
 *
 * @brief   Header file of some software
 *
 */

#ifndef FOXBMS__STATE_MACHINE_H_
#define FOXBMS__STATE_MACHINE_H_

/*========== Includes =======================================================*/
#include "general.h"

/*========== Macros and Definitions =========================================*/
/** States of the state machine */
typedef enum EG_FSM_STATES {
    EG_FSM_STATE_DUMMY,          /*!< dummy state - always the first state */
    EG_FSM_STATE_HAS_NEVER_RUN,  /*!< never run state - always the second state */
    EG_FSM_STATE_UNINITIALIZED,  /*!< uninitialized state */
    EG_FSM_STATE_INITIALIZATION, /*!< initializing the state machine */
    EG_FSM_STATE_RUNNING,        /*!< operational mode of the state machine  */
    EG_FSM_STATE_ERROR,          /*!< state for error processing  */
} EG_FSM_STATES_e;

/** Substates of the state machine */
typedef enum EG_FSM_SUBSTATES {
    EG_FSM_SUBSTATE_DUMMY,               /*!< dummy state - always the first substate */
    EG_FSM_SUBSTATE_ENTRY,               /*!< entry state - always the second substate */
    EG_FSM_SUBSTATE_INITIALIZATION_0,    /*!< fist initialization substate */
    EG_FSM_SUBSTATE_INITIALIZATION_1,    /*!< second initialization substate */
    EG_FSM_SUBSTATE_INITIALIZATION_EXIT, /*!< last initialization substate */
    EG_FSM_SUBSTATE_RUNNING_0,           /*!< fist running substate */
    EG_FSM_SUBSTATE_RUNNING_1,           /*!< second running substate */
    EG_FSM_SUBSTATE_RUNNING_2,           /*!< third running substate */
} EG_FSM_SUBSTATES_e;

/** some struct with some information */
typedef struct EG_INFORMATION {
    uint8_t r0; /*!< some info 0 */
    uint8_t r1; /*!< some info 0 */
    uint8_t r2; /*!< some info 0 */
} EG_INFORMATION_s;

/** This struct describes the state of the monitoring instance */
typedef struct EG_STATE {
    uint16_t timer;                      /*!< timer of the state */
    uint8_t triggerEntry;                /*!< trigger entry of the state */
    EG_FSM_STATES_e nextState;           /*!< next state of the FSM */
    EG_FSM_STATES_e currentState;        /*!< current state of the FSM */
    EG_FSM_STATES_e previousState;       /*!< previous state of the FSM */
    EG_FSM_SUBSTATES_e nextSubstate;     /*!< next substate of the FSM */
    EG_FSM_SUBSTATES_e currentSubstate;  /*!< current substate of the FSM */
    EG_FSM_SUBSTATES_e previousSubstate; /*!< previous substate of the FSM */
    EG_INFORMATION_s information;        /*!< Some information to be stored */
} EG_STATE_s;

/*========== Extern Constant and Variable Declarations ======================*/

/** state of the example state machine */
extern EG_STATE_s eg_state;

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   tick function, call this to advance the state machine
 * @param   pEgState current state of the state machine
 * @returns returns always #STD_OK
 */
extern STD_RETURN_TYPE_e EG_Trigger(EG_STATE_s *pEgState);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__STATE_MACHINE_H_ */
