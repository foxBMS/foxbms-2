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
 * @file    bal.h
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2020-02-24 (date of last update)
 * @ingroup APPLICATION
 * @prefix  BAL
 *
 * @brief   Header for the driver for balancing
 *
 */

#ifndef FOXBMS__BAL_H_
#define FOXBMS__BAL_H_

/*========== Includes =======================================================*/
#include "bal_cfg.h"

#include "database.h"

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/
/**
 * States of the BAL state machine
 */
typedef enum BAL_STATEMACH {
    /* Init-Sequence */
    BAL_STATEMACH_UNINITIALIZED,   /*!<    */
    BAL_STATEMACH_INITIALIZATION,  /*!<    */
    BAL_STATEMACH_INITIALIZED,     /*!<    */
    BAL_STATEMACH_CHECK_BALANCING, /*!<    */
    BAL_STATEMACH_BALANCE,         /*!<    */
    BAL_STATEMACH_NO_BALANCING,    /*!<    */
    BAL_STATEMACH_ALLOWBALANCING,  /*!<    */
    BAL_STATEMACH_GLOBALDISABLE,   /*!<    */
    BAL_STATEMACH_GLOBALENABLE,    /*!<    */
    BAL_STATEMACH_UNDEFINED,       /*!< undefined state */
    BAL_STATEMACH_RESERVED1,       /*!< reserved state */
    BAL_STATEMACH_ERROR,           /*!< Error-State: */
} BAL_STATEMACH_e;

/**
 * Substates of the BAL state machine
 */
typedef enum BAL_STATEMACH_SUB {
    BAL_ENTRY,                /*!< Substate entry state */
    BAL_CHECK_IMBALANCES,     /*!< Check if balancing has been initialized */
    BAL_COMPUTE_IMBALANCES,   /*!< Compute imbalances */
    BAL_ACTIVATE_BALANCING,   /*!< Activated balancing resistors */
    BAL_CHECK_LOWEST_VOLTAGE, /*!< Check if lowest voltage is still  above limit */
    BAL_CHECK_CURRENT,        /*!< Check if current is still  under limit */
} BAL_STATEMACH_SUB_e;

/**
 * State requests for the BAL statemachine
 */
typedef enum BAL_STATE_REQUEST {
    BAL_STATE_INIT_REQUEST,           /*!<    */
    BAL_STATE_ERROR_REQUEST,          /*!<    */
    BAL_STATE_NO_BALANCING_REQUEST,   /*!<    */
    BAL_STATE_ALLOWBALANCING_REQUEST, /*!<    */
    BAL_STATE_GLOBAL_DISABLE_REQUEST, /*!<    */
    BAL_STATE_GLOBAL_ENABLE_REQUEST,  /*!<    */
    BAL_STATE_NO_REQUEST,             /*!< default state: no request to the statemachine */
} BAL_STATE_REQUEST_e;

/**
 * Possible return values when state requests are made to the BAL statemachine
 */
typedef enum BAL_RETURN_TYPE {
    BAL_OK,                  /*!< BAL --> ok */
    BAL_BUSY_OK,             /*!< BAL busy */
    BAL_REQUEST_PENDING,     /*!< requested to be executed */
    BAL_ILLEGAL_REQUEST,     /*!< Request can not be executed */
    BAL_INIT_ERROR,          /*!< Error state: Source: Initialization */
    BAL_OK_FROM_ERROR,       /*!< Return from error --> ok */
    BAL_ERROR,               /*!< General error state */
    BAL_ALREADY_INITIALIZED, /*!< Initialization of BAL already finished */
    BAL_ILLEGAL_TASK_TYPE,   /*!< Illegal */
} BAL_RETURN_TYPE_e;

/**
 * This structure contains all the variables relevant for the BAL state machine.
 * The user can get the current state of the BAL state machine with this variable
 */
typedef struct BAL_STATE {
    uint16_t timer; /*!< time in ms before the state machine processes the next state, e.g. in counts of 1ms */
    BAL_STATE_REQUEST_e stateRequest;         /*!< current state request made to the state machine */
    BAL_STATEMACH_e state;                    /*!< state of Driver State Machine */
    BAL_STATEMACH_SUB_e substate;             /*!< current substate of the state machine */
    BAL_STATEMACH_e lastState;                /*!< previous state of the state machine */
    uint8_t lastSubstate;                     /*!< previous substate of the state machine */
    uint8_t triggerEntry;                     /*!< counter for re-entrance protection (function running flag) */
    uint32_t errorRequestCounter;             /*!< counts the number of illegal requests to the BAL state machine */
    STD_RETURN_TYPE_e initializationFinished; /*!< #STD_OK if statemachine initialized, otherwise #STD_NOT_OK */
    bool active;                              /*!< indicate if balancing active or not */
    int32_t balancingThreshold;               /*!< effective balancing threshold */
    bool balancingAllowed;                    /*!< flag to disable balancing */
    bool balancingGlobalAllowed;              /*!< flag to globally disable balancing */
} BAL_STATE_s;

/*========== Extern Function Prototypes =====================================*/

/** @brief Saves the last state and the last substate */
extern void BAL_SaveLastStates(BAL_STATE_s *pBalancingState);

/**
 * @brief   re-entrance check of BAL state machine trigger function
 * @details This function is not re-entrant and should only be called time- or
 *          event-triggered. It increments the triggerentry counter from the
 *          state variable. It should never be called by two different
 *          processes, so if it is the case, triggerentry should never be
 *          higher than 0 when this function is called.
 * @return  0 if no further instance of the function is active, 0xff otherwise
 */
extern uint8_t BAL_CheckReEntrance(BAL_STATE_s *currentState);

/**
 * @brief   transfers the current state request to the state machine.
 * @details This function takes the current state request from current state
 *          and transfers it to the state machine. It resets the value from
 *          to #BAL_STATE_NO_REQUEST
 * @return  current state request
 */
extern BAL_STATE_REQUEST_e BAL_TransferStateRequest(BAL_STATE_s *currentState);

/**
 * @brief   checks the state requests that are made.
 * @details This function checks the validity of the state requests. The
 *          results of the checked is returned immediately.
 * @param   pCurrentState   pointer to the current state
 * @param   stateRequest    state request to be checked
 * @return  result of the state request that was made
 */
extern BAL_RETURN_TYPE_e BAL_CheckStateRequest(BAL_STATE_s *pCurrentState, BAL_STATE_REQUEST_e stateRequest);

/**
 * @brief   Substate handling function for #BAL_Trigger()
 * @param   pCurrentState   pointer to the current state
 * @param   stateRequest    state request to set
 */
extern void BAL_ProcessStateUninitalized(BAL_STATE_s *pCurrentState, BAL_STATE_REQUEST_e stateRequest);

/**
 * @brief   State machine subfunction to initialize the balancing state machine
 * @details TODO
 */
extern void BAL_ProcessStateInitialization(BAL_STATE_s *currentState);

/**
 * @brief   State machine subfunction to transfer from an initalized state to
 *          "running" states of th state machine
 * @details TODO
 */
extern void BAL_ProcessStateInitialized(BAL_STATE_s *currentState);

/** @brief  Generic initialization function for the balancing module */
extern STD_RETURN_TYPE_e BAL_Init(DATA_BLOCK_BALANCING_CONTROL_s *pControl);

/**
 * @brief   sets the current state request of the state variable bal_state.
 * @details This function is used to make a state request to the state machine,
 *          e.g, start voltage measurement, read result of voltage measurement,
 *          re-initialization. It calls #BAL_CheckStateRequest() to check if
 *          the request is valid. The state request is rejected if is not
 *          valid. The result of the check is returned immediately, so that the
 *          requester can act in case it made a non-valid state request.
 * @param   stateRequest    state request to set
 * @return  current state request
 */
extern BAL_RETURN_TYPE_e BAL_SetStateRequest(BAL_STATE_REQUEST_e stateRequest);

/**
 * @brief   gets the initialization state.
 * @details This function is used for getting the balancing initialization
 *          state
 * @return  #STD_OK if initialized, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e BAL_GetInitializationState(void);

/**
 * @brief   trigger function for the BAL driver state machine.
 * @details This function contains the sequence of events in the BAL state
 *          machine. It must be called time-triggered, every 100 milliseconds.
 */
extern void BAL_Trigger(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern BAL_STATEMACH_e BAL_GetState(void);
#endif
/*========== Getter for static Variables (Unit Test) ========================*/
#ifdef UNITY_UNIT_TEST
extern DATA_BLOCK_BALANCING_CONTROL_s *TEST_BAL_GetBalancingControl(void);
extern BAL_STATE_s *TEST_BAL_GetBalancingState(void);
#endif

#endif /* FOXBMS__BAL_H_ */
