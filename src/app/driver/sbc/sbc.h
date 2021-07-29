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
 * @file    sbc.h
 * @author  foxBMS Team
 * @date    2020-07-14 (date of creation)
 * @updated 2021-07-23 (date of last update)
 * @ingroup DRIVERS
 * @prefix  SBC
 *
 * @brief   Header for the driver for the SBC module
 *
 * @details It must always be used when creating new c header files.
 *
 */

#ifndef FOXBMS__SBC_H_
#define FOXBMS__SBC_H_

/*========== Includes =======================================================*/
#include "general.h"

#include "ftask_cfg.h"

#include "nxpfs85xx.h"

/*========== Macros and Definitions =========================================*/

/**
 * This define MUST represent the cycle time of the task in which context the
 * functions run, e.g., if the #SBC_Trigger() is running in the 10 ms task
 * then the define must be set to 10.
 *
 * This sets the minimum time between two subsequent executed states/substates.
 *
 * Define is only used for compile-time assertion, it has no programatic
 * influence on the actual code.
 */
#define SBC_STATEMACHINE_TASK_CYCLE_CONTEXT_MS (10u)

#if SBC_STATEMACHINE_TASK_CYCLE_CONTEXT_MS != FTSK_TASK_CYCLIC_10MS_CYCLE_TIME
#error "Invalid SBC configuration. Make sure that SBC timing is configured correctly!"
#endif

/**
 * SBC statemachine short time definition in #SBC_Trigger() calls until next
 * state/substate is processed
 */
#define SBC_STATEMACHINE_SHORTTIME (1u)

/**
 * SBC statemachine medium time definition in #SBC_Trigger() calls until next
 * state/substate is processed
 */
#define SBC_STATEMACHINE_MEDIUMTIME (5u)

/**
 * SBC statemachine long time definition in #SBC_Trigger() calls until next
 * state/substate is processed
 */
#define SBC_STATEMACHINE_LONGTIME (10u)

/**
 * trigger period of SBC watchdog
 */
#define SBC_WINDOW_WATCHDOG_PERIOD_MS (100u)

/** State requests for the SYS statemachine */
typedef enum SBC_STATE_REQUEST {
    SBC_STATE_INIT_REQUEST,  /*!< request to begin SBC initialization */
    SBC_STATE_ERROR_REQUEST, /*!< request to switch SBC into error state */
    SBC_STATE_NO_REQUEST,    /*!< enum to clarify that currently no new request needs to be processed */
} SBC_STATE_REQUEST_e;

/** State of watchdog state */
typedef enum SBC_PERIODIC_WATCHDOG_STATE {
    SBC_PERIODIC_WATCHDOG_ACTIVATED,
    SBC_PERIODIC_WATCHDOG_DEACTIVATED,
} SBC_PERIODIC_WATCHDOG_STATE_e;

/** Possible return values when state requests are made to the SYS statemachine */
typedef enum SBC_RETURN_TYPE {
    SBC_OK,                  /*!< sys --> ok                             */
    SBC_BUSY_OK,             /*!< sys busy --> ok                        */
    SBC_REQUEST_PENDING,     /*!< requested to be executed               */
    SBC_ILLEGAL_REQUEST,     /*!< Request can not be executed            */
    SBC_ALREADY_INITIALIZED, /*!< Initialization of SBC already finished */
    SBC_ILLEGAL_TASK_TYPE,   /*!< Illegal                                */
} SBC_RETURN_TYPE_e;

/** States of the SBC state machine */
typedef enum SBC_STATEMACHINE {
    /* Init-Sequence */
    SBC_STATEMACHINE_UNINITIALIZED,  /*!< statemachien start value */
    SBC_STATEMACHINE_INITIALIZATION, /*!< SBC startup initialization process */
    SBC_STATEMACHINE_RUNNING,        /*!< error detected communicating with SBC */
    SBC_STATEMACHINE_ERROR,          /*!< default initialization value */
    SBC_STATEMACHINE_UNDEFINED,
} SBC_STATEMACHINE_e;

/** Substates of the SBC state machine */
typedef enum SBC_STATEMACHINE_SUB {
    SBC_ENTRY,                                /*!< Substate entry state       */
    SBC_INIT_RESET_FAULT_ERROR_COUNTER_PART1, /*!< Substate during initialization to reset fault-error counter */
    SBC_INIT_RESET_FAULT_ERROR_COUNTER_PART2, /*!< Substate during initialization to reset fault-error counter */
    SBC_INITIALIZE_SAFETY_PATH_CHECK,         /*!< Substate to initialize SBC */
    SBC_INITIALIZE_VOLTAGE_SUPERVISOR_PART3,  /*!< Substate to initialize SBC */
    SBC_INITIALIZE_VOLTAGE_SUPERVISOR_PART4,  /*!< Substate to initialize SBC */
} SBC_STATEMACHINE_SUB_e;

/**
 * This structure contains all the variables relevant for the SBC state machine.
 * The user can get the current state of the SBC state machine with this variable
 */
typedef struct SBC_STATE {
    uint16_t timer;           /*!< time in ms before the state machine processes the next state, in counts of 10ms */
    uint16_t watchdogTrigger; /*!< time in ms before the state machine triggers watchdog, in counts of 10ms */
    SBC_STATE_REQUEST_e stateRequest;    /*!< current state request made to the state machine */
    SBC_STATEMACHINE_e state;            /*!< state of Driver State Machine */
    SBC_STATEMACHINE_SUB_e substate;     /*!< current substate of the state machine */
    SBC_STATEMACHINE_e lastState;        /*!< previous state of the state machine */
    SBC_STATEMACHINE_SUB_e lastSubstate; /*!< previous substate of the state machine */
    uint32_t illegalRequestsCounter;     /*!< counts the number of illegal requests to the SBC state machine */
    uint8_t retryCounter;                /*!< counter to retry subsystem initialization if fails */
    uint8_t requestWatchdogTrigger;      /*!< required watchdog triggers during init to correctly initialize SBC */
    uint8_t triggerEntry;                /*!< counter for re-entrance protection (function running flag) */
    SBC_PERIODIC_WATCHDOG_STATE_e watchdogState; /*!< state if periodic watchdog trigger is required or not */
    FS85xx_STATE_s *pFs85xxInstance;             /*!< pointer to FS85xx instance */
    uint16_t watchdogPeriod_10ms;                /*!< watchdog trigger frequency in 10ms */
} SBC_STATE_s;

/*========== Extern Constant and Variable Declarations ======================*/
extern SBC_STATE_s sbc_stateMcuSupervisor;

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   sets the current state request of passed state variable
 * @details This function is used to make a state request to the state machine,
 *          e.g., start initialization. It calls #SBC_CheckStateRequest()
 *          to check if the request is valid. The state request is rejected if
 *          is not valid. The result of the check is returned immediately, so
 *          that the requester can act in case it made a non-valid state
 *          request.
 *
 * @param[in,out] pInstance     SBC instance where different state is requested
 * @param[in]     stateRequest  requested state
 * @return  If the request was successfully set, it returns the #SBC_OK, else
 *          the current state of requests (type #SBC_STATE_REQUEST_e)
 */
extern SBC_RETURN_TYPE_e SBC_SetStateRequest(SBC_STATE_s *pInstance, SBC_STATE_REQUEST_e stateRequest);

/**
 * @brief   gets the current state of passed state variable
 * @details This function is used in the functioning of the SBC state machine.
 *
 * @param[in] pInstance  SBC instance where the current state is requested
 *
 * @return  current state, taken from #SBC_STATEMACHINE_e
 */
extern SBC_STATEMACHINE_e SBC_GetState(SBC_STATE_s *pInstance);

/**
 * @brief   trigger function for the SYS driver state machine.
 * @details This function contains the sequence of events in the SYS state
 *          machine. It must be called time-triggered, every 10ms.
 *
 * @param[in,out] pInstance   SBC instance that is controlled
 */
extern void SBC_Trigger(SBC_STATE_s *pInstance);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__SBC_H_ */
