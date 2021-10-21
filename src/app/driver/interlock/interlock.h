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
 * @file    interlock.h
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2021-10-18 (date of last update)
 * @ingroup DRIVERS
 * @prefix  ILCK
 *
 * @brief   Headers for the driver for the interlock.
 *
 */

#ifndef FOXBMS__INTERLOCK_H_
#define FOXBMS__INTERLOCK_H_

/*========== Includes =======================================================*/
#include "interlock_cfg.h"

/*========== Macros and Definitions =========================================*/

/**
 * States of the ILCK state machine
 */
typedef enum {
    /* Init-Sequence */
    ILCK_STATEMACHINE_UNINITIALIZED, /*!< Uninitialized state */
    ILCK_STATEMACHINE_INITIALIZED,   /*!< statemachine initialized */
    ILCK_STATEMACHINE_UNDEFINED,     /*!< undefined state */
} ILCK_STATEMACH_e;

/**
 * Substates of the ILCK state machine
 */
typedef enum {
    ILCK_ENTRY, /*!< Substate entry state       */
} ILCK_STATEMACH_SUB_e;

/**
 * State requests for the ILCK statemachine
 */
typedef enum {
    ILCK_STATE_INITIALIZATION_REQUEST,
    ILCK_STATE_NO_REQUEST,
} ILCK_STATE_REQUEST_e;

/**
 * Possible return values when state requests are made to the ILCK statemachine
 */
typedef enum {
    ILCK_OK,                  /*!< ILCK --> ok                             */
    ILCK_REQUEST_PENDING,     /*!< requested to be executed                */
    ILCK_ALREADY_INITIALIZED, /*!< Initialization of ilck already finished */
    ILCK_ILLEGAL_REQUEST,     /*!< Request can not be executed             */
} ILCK_RETURN_TYPE_e;

/**
 * This structure contains all the variables relevant for the ILCK state machine.
 * The user can get the current state of the ILCK state machine with this variable
 */
typedef struct {
    uint16_t timer; /*!< time in ms before the state machine processes the next state, e.g. in counts of 1ms    */
    ILCK_STATE_REQUEST_e
        statereq; /*!< current state request made to the state machine                                        */
    ILCK_STATEMACH_e
        state; /*!< state of Driver State Machine                                                          */
    ILCK_STATEMACH_SUB_e
        substate; /*!< current substate of the state machine                                                  */
    ILCK_STATEMACH_e
        laststate; /*!< previous state of the state machine                                                    */
    ILCK_STATEMACH_SUB_e
        lastsubstate; /*!< previous substate of the state machine                                                 */
    uint32_t ErrRequestCounter; /*!< counts the number of illegal requests to the LTC state machine */
    uint8_t triggerentry;       /*!< counter for re-entrance protection (function running flag) */
    uint8_t counter;            /*!< general purpose counter */
} ILCK_STATE_s;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   Checks the configuration of the interlock-module
 * @return  retVal (type: STD_RETURN_TYPE_e)
 */
extern STD_RETURN_TYPE_e ILCK_Init(void);

/**
 * @brief   sets the current state request of the state variable ilck_state.
 *
 * This function is used to make a state request to the state machine,e.g, start voltage measurement,
 * read result of voltage measurement, re-initialization
 * It calls ILCK_CheckStateRequest() to check if the request is valid.
 * The state request is rejected if is not valid.
 * The result of the check is returned immediately, so that the requester can act in case
 * it made a non-valid state request.
 *
 * @param   statereq                state request to set
 *
 * @return  retVal                  current state request, taken from ILCK_STATE_REQUEST_e
 */
extern ILCK_RETURN_TYPE_e ILCK_SetStateRequest(ILCK_STATE_REQUEST_e statereq);

/**
 * @brief   gets the current state.
 *
 * This function is used in the functioning of the ILCK state machine.
 *
 * @return  current state, taken from ILCK_STATEMACH_e
 */
extern ILCK_STATEMACH_e ILCK_GetState(void);

/**
 * @brief   trigger function for the ILCK driver state machine.
 *
 * This function contains the sequence of events in the ILCK state machine.
 * It must be called time-triggered, every 1ms.
 */
extern void ILCK_Trigger(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern void TEST_ILCK_SetStateStruct(ILCK_STATE_s state);
extern ILCK_ELECTRICAL_STATE_TYPE_e TEST_ILCK_GetInterlockFeedback(void);
#endif

#endif /* FOXBMS__INTERLOCK_H_ */
