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
 * @file    sys.h
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2021-10-12 (date of last update)
 * @ingroup ENGINE
 * @prefix  SYS
 *
 * @brief   Sys driver header
 *
 *
 */

#ifndef FOXBMS__SYS_H_
#define FOXBMS__SYS_H_

/*========== Includes =======================================================*/
#include "sys_cfg.h"

/*========== Macros and Definitions =========================================*/

/** Symbolic names for busyness of the system */
typedef enum SYS_CHECK {
    SYS_CHECK_OK,     /*!< system ok      */
    SYS_CHECK_BUSY,   /*!< system busy    */
    SYS_CHECK_NOT_OK, /*!< system not ok  */
} SYS_CHECK_e;

/** States of the state machine */
typedef enum SYS_FSM_STATES {
    SYS_FSM_STATE_DUMMY,          /*!< dummy state - always the first state */
    SYS_FSM_STATE_HAS_NEVER_RUN,  /*!< never run state - always the second state */
    SYS_FSM_STATE_UNINITIALIZED,  /*!< uninitialized state */
    SYS_FSM_STATE_INITIALIZATION, /*!< initializing the state machine */
    SYS_FSM_STATE_RUNNING,        /*!< operational mode of the state machine  */
    SYS_FSM_STATE_ERROR,          /*!< state for error processing  */
} SYS_FSM_STATES_e;

/** Substates of the state machine */
typedef enum SYS_FSM_SUBSTATES {
    SYS_FSM_SUBSTATE_DUMMY,                                  /*!< dummy state - always the first substate */
    SYS_FSM_SUBSTATE_ENTRY,                                  /*!< entry state - always the second substate */
    SYS_FSM_SUBSTATE_INITIALIZATION_SBC,                     /*!< TODO */
    SYS_FSM_SUBSTATE_INITIALIZATION_BOOT_MESSAGE,            /*!< TODO */
    SYS_FSM_SUBSTATE_INITIALIZATION_INTERLOCK,               /*!< TODO */
    SYS_FSM_SUBSTATE_INITIALIZATION_CONTACTORS,              /*!< TODO */
    SYS_FSM_SUBSTATE_INITIALIZATION_BALANCING,               /*!< TODO */
    SYS_FSM_SUBSTATE_INITIALIZATION_INSULATION_GUARD,        /*!< TODO */
    SYS_FSM_SUBSTATE_INITIALIZATION_FIRST_MEASUREMENT_CYCLE, /*!< TODO */
    SYS_FSM_SUBSTATE_INITIALIZATION_CHECK_CURRENT_SENSOR,    /*!< TODO */
    SYS_FSM_SUBSTATE_INITIALIZATION_MISC,                    /*!< TODO */
    SYS_FSM_SUBSTATE_INITIALIZATION_BMS,                     /*!< TODO */
    SYS_FSM_SUBSTATE_RUNNING,                                /*!< fist running substate */
} SYS_FSM_SUBSTATES_e;

/*================== Constant and Variable Definitions ======================*/

/** States of the SYS state machine */
typedef enum SYS_STATEMACH {
    /* Init-Sequence */
    SYS_STATEMACH_UNINITIALIZED,                 /*!<    */
    SYS_STATEMACH_INITIALIZATION,                /*!<    */
    SYS_STATEMACH_SYSTEM_BIST,                   /*!< run a built-in self-test */
    SYS_STATEMACH_INITIALIZED,                   /*!<    */
    SYS_STATEMACH_INITIALIZE_SBC,                /*!<    */
    SYS_STATEMACH_INITIALIZE_CAN,                /*!< initialize CAN module */
    SYS_STATEMACH_INITIALIZE_INTERLOCK,          /*!<    */
    SYS_STATEMACH_INITIALIZE_CONTACTORS,         /*!<    */
    SYS_STATEMACH_INITIALIZE_BALANCING,          /*!<    */
    SYS_STATEMACH_INITIALIZE_BMS,                /*!<    */
    SYS_STATEMACH_RUNNING,                       /*!<    */
    SYS_STATEMACH_FIRST_MEASUREMENT_CYCLE,       /*!<    */
    SYS_STATEMACH_INITIALIZE_MISC,               /*!<    */
    SYS_STATEMACH_CHECK_CURRENT_SENSOR_PRESENCE, /*!<    */
    SYS_STATEMACH_INITIALIZE_ISOGUARD,           /*!<    */
    SYS_STATEMACH_ERROR,                         /*!< Error-State */
} SYS_STATEMACH_e;

/** Substates of the SYS state machine */
typedef enum SYS_STATEMACH_SUB {
    SYS_ENTRY,                         /*!< Substate entry state */
    SYS_CHECK_ERROR_FLAGS,             /*!< Substate check if any error flag set */
    SYS_CHECK_STATE_REQUESTS,          /*!< Substate check if there is a state request */
    SYS_WAIT_INITIALIZATION_SBC,       /*!< Substate to wait for initialization of the sbc state machine */
    SYS_WAIT_INITIALIZATION_INTERLOCK, /*!< Substate to wait for initialization of the interlock state machine */
    SYS_WAIT_INITIALIZATION_CONT,      /*!< Substate to wait for initialization of the contactor state machine */
    SYS_WAIT_INITIALIZATION_BAL,       /*!< Substate to wait for initialization of the balancing state machine */
    SYS_WAIT_INITIALIZATION_BAL_GLOBAL_ENABLE, /*!< Substate to enable/disable balancing globally */
    SYS_WAIT_INITIALIZATION_BMS,               /*!< Substate to wait for initialization of the bms state machine */
    SYS_WAIT_FIRST_MEASUREMENT_CYCLE,          /*!< Substate to wait for first measurement cycle to complete */
    SYS_WAIT_CURRENT_SENSOR_PRESENCE,          /*!< Substate to wait for first measurement cycle to complete */
    SYS_SBC_INIT_ERROR,                        /*!< Substate error of SBC initialization */
    SYS_CONT_INIT_ERROR,                       /*!< Substate error of contactor state machine initialization */
    SYS_BAL_INIT_ERROR,                        /*!< Substate error of balancing state machine initialization */
    SYS_ILCK_INIT_ERROR,                       /*!< Substate error of contactor state machine initialization */
    SYS_BMS_INIT_ERROR,                        /*!< Substate error of bms state machine initialization */
    SYS_MEAS_INIT_ERROR,                       /*!< Substate error if first measurement cycle does not complete */
    SYS_CURRENT_SENSOR_PRESENCE_ERROR,         /*!< Substate error if first measurement cycle does not complete */
} SYS_STATEMACH_SUB_e;

/** State requests for the SYS statemachine */
typedef enum SYS_STATE_REQUEST {
    SYS_STATE_INIT_REQUEST,  /*!< initialization request */
    SYS_STATE_ERROR_REQUEST, /*!< error state requested */
    SYS_STATE_NO_REQUEST,    /*!< no request */
} SYS_STATE_REQUEST_e;

/** Possible return values when state requests are made to the SYS statemachine */
typedef enum SYS_RETURN_TYPE {
    SYS_OK,                  /*!< sys --> ok                             */
    SYS_BUSY_OK,             /*!< sys busy --> ok                        */
    SYS_REQUEST_PENDING,     /*!< requested to be executed               */
    SYS_ILLEGAL_REQUEST,     /*!< Request can not be executed            */
    SYS_ALREADY_INITIALIZED, /*!< Initialization of LTC already finished */
    SYS_ILLEGAL_TASK_TYPE,   /*!< Illegal                                */
} SYS_RETURN_TYPE_e;

/**
 * This structure contains all the variables relevant for the CONT state machine.
 * The user can get the current state of the CONT state machine with this variable
 */
typedef struct SYS_STATE {
    uint16_t timer; /*!< time in ms before the state machine processes the next state, e.g. in counts of 1ms    */
    SYS_STATE_REQUEST_e stateRequest; /*!< current state request made to the state machine                      */
    SYS_STATEMACH_e state;            /*!< state of Driver State Machine                                        */
    SYS_STATEMACH_SUB_e substate;     /*!< current substate of the state machine                                */
    SYS_STATEMACH_e lastState;        /*!< previous state of the state machine                                  */
    SYS_STATEMACH_SUB_e lastSubstate; /*!< previous substate of the state machine                               */
    uint32_t illegalRequestsCounter;  /*!< counts the number of illegal requests to the SYS state machine       */
    uint16_t initializationTimeout;   /*!< Timeout to wait for initialization of state machine state machine    */
    uint8_t triggerEntry;             /*!< counter for re-entrance protection (function running flag)           */
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

/** built-in self-test for the macros in general.h */
extern void SYS_GeneralMacroBist(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__SYS_H_ */
