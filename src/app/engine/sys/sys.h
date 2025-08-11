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
 * @file    sys.h
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup ENGINE
 * @prefix  SYS
 *
 * @brief   Sys driver header
 * @details TODO
 */

#ifndef FOXBMS__SYS_H_
#define FOXBMS__SYS_H_

/*========== Includes =======================================================*/
#include "sys_cfg.h"

#include "fstd_types.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*================== Constant and Variable Definitions ======================*/
typedef enum {
    SYS_FSM_STATE_DUMMY,          /*!< dummy state - always the first state             */
    SYS_FSM_STATE_HAS_NEVER_RUN,  /*!< never run state - always the second state        */
    SYS_FSM_STATE_UNINITIALIZED,  /*!< uninitialized state                              */
    SYS_FSM_STATE_INITIALIZATION, /*!< initializing the system state machine            */
    SYS_FSM_STATE_PRE_RUNNING,    /*!< state to set everything needed for running state */
    SYS_FSM_STATE_RUNNING,        /*!< operational mode of the state machine            */
    SYS_FSM_STATE_ERROR,          /*!< error processing                                 */
} SYS_FSM_STATES_e;

typedef enum {
    SYS_FSM_SUBSTATE_DUMMY,                    /*!< dummy state - always the first substate */
    SYS_FSM_SUBSTATE_ENTRY,                    /*!< entry state - always the second substate */
    SYS_FSM_CHECK_DEEP_DISCHARGE,              /*!< Substate to read the FRAM */
    SYS_FSM_SUBSTATE_START_INITIALIZATION_SBC, /*!< Substate to start the initialization of the sbc state machine */
    SYS_FSM_SUBSTATE_INITIALIZE_INTERLOCK,     /*!< Substate to initialize the interlock*/
    SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_SBC,  /*!< Substate to wait for initialization of the sbc state machine */
    SYS_FSM_SUBSTATE_INITIALIZATION_CAN,       /*!< Substate to initialize the can system */
    SYS_FSM_SUBSTATE_INITIALIZATION_RTC,       /*!< wait for the RTC module to be initialized */
    SYS_FSM_SUBSTATE_START_UP_BIST,            /*!< Substate to execute the start up built-in self-test */
    SYS_FSM_SUBSTATE_SEND_BOOT_MESSAGE,        /*!< Substate to send the boot message and finish initialization */
    SYS_FSM_SUBSTATE_START_INITIALIZATION_BAL, /*!< Substate to start the initialization of the balancing state machine */
    SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BAL, /*!< Substate to wait for initialization of the balancing state machine */
    SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BAL_GLOBAL_ENABLE, /*!< Substate to enable/disable balancing globally */
    SYS_FSM_SUBSTATE_START_FIRST_MEASUREMENT_CYCLE,         /*!< Substate to start the first measurement cycle */
    SYS_FSM_SUBSTATE_WAIT_FIRST_MEASUREMENT_CYCLE, /*!< Substate to wait for first measurement cycle to complete */
    SYS_FSM_SUBSTATE_START_CURRENT_SENSOR_PRESENCE_CHECK, /*!< Substate to start the current sensor presence check */
    SYS_FSM_SUBSTATE_WAIT_CURRENT_SENSOR_PRESENCE_CHECK, /*!< Substate to wait for the current sensor presence check to complete */
    SYS_FSM_SUBSTATE_INITIALIZATION_MISC,                /*!< Substate to initialize our miscellaneous functions */
    SYS_FSM_SUBSTATE_INITIALIZATION_IMD,                 /*!< Substate to initialize the imd state machine */
    SYS_FSM_SUBSTATE_START_INITIALIZATION_BMS, /*!< Substate to start the initialization of the bms state machine */
    SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BMS,  /*!< Substate to wait for initialization of the bms state machine */
    /* Error states to transmit the error case to the error state */
    SYS_FSM_SUBSTATE_SBC_INITIALIZATION_ERROR, /*!< Substate error of SBC initialization */
    SYS_FSM_SUBSTATE_BAL_INITIALIZATION_ERROR, /*!< Substate error of balancing state machine initialization */
    SYS_FSM_SUBSTATE_BAL_GLOBAL_INITIALIZATION_ERROR, /*!< Substate error of the global balancing state machine initialization */
    SYS_FSM_SUBSTATE_FIRST_MEAS_INITIALIZATION_ERROR, /*!< Substate error if first measurement cycle does not complete */
    SYS_FSM_SUBSTATE_CURRENT_SENSOR_PRESENCE_ERROR,   /*!< Substate error if current sensor can not be found */
    SYS_FSM_SUBSTATE_IMD_INITIALIZATION_ERROR,        /*!< Substate error of bms state machine initialization */
    SYS_FSM_SUBSTATE_BMS_INITIALIZATION_ERROR,        /*!< Substate error of bms state machine initialization */
} SYS_FSM_SUBSTATES_e;

/** State requests for the SYS state machine */
typedef enum {
    SYS_STATE_INITIALIZATION_REQUEST, /*!< initialization request */
    SYS_STATE_ERROR_REQUEST,          /*!< error state requested */
    SYS_STATE_NO_REQUEST,             /*!< no request */
} SYS_STATE_REQUEST_e;

/** Possible return values when state requests are made to the SYS state machine */
typedef enum {
    SYS_OK,                  /*!< sys --> ok                             */
    SYS_BUSY_OK,             /*!< sys busy --> ok                        */
    SYS_REQUEST_PENDING,     /*!< requested to be executed               */
    SYS_ILLEGAL_REQUEST,     /*!< Request can not be executed            */
    SYS_ALREADY_INITIALIZED, /*!< Initialization of LTC already finished */
} SYS_RETURN_TYPE_e;

/**
 * This structure contains all the variables relevant for the CONT state machine.
 * The user can get the current state of the CONT state machine with this variable
 */
typedef struct {
    uint16_t timer; /*!< time in ms before the state machine processes the next state, e.g. in counts of 1ms       */
    SYS_STATE_REQUEST_e stateRequest; /*!< current state request made to the state machine                         */
    SYS_FSM_STATES_e nextState;       /*!< next state of driver state machine                                      */
    SYS_FSM_SUBSTATES_e nextSubstate; /*!< next substate of the state machine                                      */
    SYS_FSM_STATES_e currentState; /*!< current state of driver state machine                                       */
    SYS_FSM_SUBSTATES_e currentSubstate;  /*!< current substate of the state machine                                */
    SYS_FSM_STATES_e previousState;       /*!< previous state of the state machine                                  */
    SYS_FSM_SUBSTATES_e previousSubstate; /*!< previous substate of the state machine                               */
    uint32_t illegalRequestsCounter;      /*!< counts the number of illegal requests to the SYS state machine       */
    uint16_t initializationTimeout;       /*!< Timeout to wait for initialization of state machine state machine    */
    uint8_t triggerEntry;                 /*!< counter for re-entrance protection (function running flag)           */
} SYS_STATE_s;

/*========== Extern Constant and Variable Declarations ======================*/

extern SYS_STATE_s sys_state;

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   sets the current state request of the state variable sys_state.
 * @details This function is used to make a state request to the state machine,
 *          e.g., start voltage measurement, read result of voltage
 *          measurement, re-initialization. It calls #SYS_CheckStateRequest()
 *          to check if the request is valid. The state request is rejected if
 *          is not valid. The result of the check is returned immediately, so
 *          that the requester can act in case it made a non-valid state
 *          request.
 * @param   stateRequest state requested to set
 * @return  If the request was successfully set, it returns the SYS_OK, else
 *          the current state of requests (type #SYS_STATE_REQUEST_e)
 */
extern SYS_RETURN_TYPE_e SYS_SetStateRequest(SYS_STATE_REQUEST_e stateRequest);

/**
 * @brief   tick function, call this to advance the state machine
 * @details This function contains the sequence of events in the SYS state
 *          machine. It must be called time-triggered, every 10ms.
 */
extern STD_RETURN_TYPE_e SYS_Trigger(SYS_STATE_s *pSystemState);

/**
 * @brief   getter function for the current system state
 * @details This function returns the current system state of pSystemState.
 *
 * @param   pSystemState pointer to the system state
 * @return  Returns the current system state
 */
extern SYS_FSM_STATES_e SYS_GetSystemState(SYS_STATE_s *pSystemState);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
STD_RETURN_TYPE_e TEST_SYS_RunStateMachine(SYS_STATE_s *pSystemState);
STD_RETURN_TYPE_e TEST_SYS_CheckStateRequest(SYS_STATE_REQUEST_e stateRequest);
void TEST_SYS_SetState(
    SYS_STATE_s *pSysState,
    SYS_FSM_STATES_e nextState,
    SYS_FSM_SUBSTATES_e nextSubstate,
    uint16_t idleTime);
/** built-in self-test for the macros in general.h */
void TEST_SYS_GeneralMacroBist(void);
#endif

#endif /* FOXBMS__SYS_H_ */
