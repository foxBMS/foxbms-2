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
 * @file    sbc.c
 * @author  foxBMS Team
 * @date    2020-07-14 (date of creation)
 * @updated 2021-03-24 (date of last update)
 * @ingroup DRIVERS
 * @prefix  SBC
 *
 * @brief   Driver for the SBC module
 *
 * @details It must always be used when creating new c source files.
 *
 */

/*========== Includes =======================================================*/
#include "sbc.h"

#include "HL_gio.h"

#include "os.h"

/*========== Macros and Definitions =========================================*/

/** Symbolic names to check re-entrance in #SBC_Trigger */
typedef enum SBC_CHECK_REENTRANCE {
    SBC_REENTRANCE_NO,  /*!< no re-entrance */
    SBC_REENTRANCE_YES, /*!< re-entrance*/
} SBC_CHECK_REENTRANCE_e;

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/
/** state of the SBC module */
SBC_STATE_s sbc_stateMcuSupervisor = {
    .timer = 0u, /*!< time in milliseconds before the state machine processes the next state, in counts of 10ms */
    .stateRequest = SBC_STATE_NO_REQUEST, /*!< current state request made to the state machine                  */
    .state        = SBC_STATEMACHINE_UNINITIALIZED,
    .substate     = SBC_ENTRY,
    .lastState    = SBC_STATEMACHINE_UNINITIALIZED,
    .lastSubstate = SBC_ENTRY,
    .illegalRequestsCounter = 0u, /*!< counts the number of illegal requests to the SBC state machine   */
    .retryCounter           = 0u, /*!< counter to retry subsystem initialization if fails               */
    .requestWatchdogTrigger = 0u, /*!< correct value set during init process                            */
    .triggerEntry           = 0u, /*!< counter for re-entrance protection (function running flag)       */
    .pFs85xxInstance        = &fs85xx_mcuSupervisor, /*!< pointer to FS85xx instance                    */
    .watchdogState          = SBC_PERIODIC_WATCHDOG_DEACTIVATED,
    .watchdogPeriod_10ms    = 10u,
};

/*========== Static Function Prototypes =====================================*/

/**
 * @brief           Saves the last state and last substate
 * @param[in,out]   pInstance
 */
static void SBC_SaveLastStates(SBC_STATE_s *pInstance);

/**
 * @brief           checks the state requests that are made.
 * @details         This function checks the validity of the state requests.
 *                  The results of the checked is returned immediately.
 * @param[in,out]   pInstance
 * @param[in]       stateRequest    state request to be checked
 * @return          result of the state request that was made, taken from
 *                  #SBC_RETURN_TYPE_e
 */
static SBC_RETURN_TYPE_e SBC_CheckStateRequest(SBC_STATE_s *pInstance, SBC_STATE_REQUEST_e stateRequest);

/**
 * @brief   Re-entrance check of SBC state machine trigger function
 * @details This function is not re-entrant and should only be called time- or
 *          event-triggered. It increments the triggerentry counter from the
 *          state variable pInstance->triggerEntry. It should never be called
 *          by two different processes, so if it is the case, triggerEntry
 *          should never be higher than 0 when this function is called.
 * @return  #SBC_REENTRANCE_NO if no further instance of the function is
 *          active, otherwise #SBC_REENTRANCE_YES
 */
static SBC_CHECK_REENTRANCE_e SBC_CheckReEntrance(SBC_STATE_s *pInstance);

/**
 * @brief           Transfers the current state request to the state machine.
 * @details         This function takes the current state request from
 *                  pInstance->stateRequest and transfers it to the state
 *                  machine. It resets the value from stateRequest to
 *                  #SBC_STATE_NO_REQUEST
 * @param[in,out]   pInstance
 * @return          retVal      current state request, taken from
 *                              #SYS_STATE_REQUEST_e
 */
static SBC_STATE_REQUEST_e SBC_TransferStateRequest(SBC_STATE_s *pInstance);

/*========== Static Function Implementations ================================*/
static void SBC_SaveLastStates(SBC_STATE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);

    if (pInstance->lastState != pInstance->state) {
        pInstance->lastState    = pInstance->state;
        pInstance->lastSubstate = pInstance->substate;
    } else if (pInstance->lastSubstate != pInstance->substate) {
        pInstance->lastSubstate = pInstance->substate;
    } else {
        /* Do not set new substate as nothing changed */
        ;
    }
}

static SBC_RETURN_TYPE_e SBC_CheckStateRequest(SBC_STATE_s *pInstance, SBC_STATE_REQUEST_e stateRequest) {
    FAS_ASSERT(pInstance != NULL_PTR);

    SBC_RETURN_TYPE_e retval = SBC_ILLEGAL_REQUEST;
    if (stateRequest == SBC_STATE_ERROR_REQUEST) {
        retval = SBC_OK;
    } else {
        if (pInstance->stateRequest == SBC_STATE_NO_REQUEST) {
            /* init only allowed from the uninitialized state */
            if (stateRequest == SBC_STATE_INIT_REQUEST) {
                if (pInstance->state == SBC_STATEMACHINE_UNINITIALIZED) {
                    retval = SBC_OK;
                } else {
                    retval = SBC_ALREADY_INITIALIZED;
                }
            } else {
                retval = SBC_ILLEGAL_REQUEST;
            }
        } else {
            retval = SBC_REQUEST_PENDING;
        }
    }
    return retval;
}

/**
 * @brief   re-entrance check of SYS state machine trigger function
 * @details This function is not re-entrant and should only be called time- or
 *          event-triggered. It increments the triggerentry counter from the
 *          state variable sys_systemState. It should never be called by two
 *          different processes, so if it is the case, triggerentry should
 *          never be higher than 0 when this function is called.
 * @return  retval  0 if no further instance of the function is active, 0xff
 *          else
 */
static SBC_CHECK_REENTRANCE_e SBC_CheckReEntrance(SBC_STATE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);

    SBC_CHECK_REENTRANCE_e isReEntrance = SBC_REENTRANCE_NO;

    OS_EnterTaskCritical();
    if (!pInstance->triggerEntry) {
        pInstance->triggerEntry++;
    } else {
        isReEntrance = SBC_REENTRANCE_YES;
    }
    OS_ExitTaskCritical();

    return isReEntrance;
}

static SBC_STATE_REQUEST_e SBC_TransferStateRequest(SBC_STATE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);

    SBC_STATE_REQUEST_e retval = SBC_STATE_NO_REQUEST;

    OS_EnterTaskCritical();
    retval                  = pInstance->stateRequest;
    pInstance->stateRequest = SBC_STATE_NO_REQUEST;
    OS_ExitTaskCritical();

    return (retval);
}

/*========== Extern Function Implementations ================================*/
extern SBC_RETURN_TYPE_e SBC_SetStateRequest(SBC_STATE_s *pInstance, SBC_STATE_REQUEST_e stateRequest) {
    FAS_ASSERT(pInstance != NULL_PTR);

    SBC_RETURN_TYPE_e retVal = SBC_ILLEGAL_REQUEST;

    OS_EnterTaskCritical();
    retVal = SBC_CheckStateRequest(pInstance, stateRequest);

    if (retVal == SBC_OK) {
        pInstance->stateRequest = stateRequest;
    }
    OS_ExitTaskCritical();

    return (retVal);
}

extern SBC_STATEMACHINE_e SBC_GetState(SBC_STATE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);

    return pInstance->state;
}

extern void SBC_Trigger(SBC_STATE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);

    SBC_STATE_REQUEST_e stateRequest = SBC_STATE_NO_REQUEST;

    /* Check re-entrance of function */
    if (SBC_CheckReEntrance(pInstance) == SBC_REENTRANCE_YES) {
        return;
    }

    /* Periodic watchdog triggering */
    if (pInstance->watchdogState == SBC_PERIODIC_WATCHDOG_ACTIVATED) {
        if (pInstance->watchdogTrigger > 0u) {
            pInstance->watchdogTrigger--;
            if (pInstance->watchdogTrigger == 0u) {
                if (STD_OK != SBC_TriggerWatchdog(pInstance->pFs85xxInstance)) {
                    /* Do what if triggering of watchdog fails? */
                } else {
                    /* Debug LED Ball V2! :*/
                    /* gioToggleBit(hetPORT1, 1); */
                    /* Reset watchdog counter:
                    * Decremented every SBC_TASK_CYCLE_CONTEXT_MS and checked in next cycle -> Period-1 */
                    pInstance->watchdogTrigger = pInstance->watchdogPeriod_10ms;
                }
            }
        }
    }

    if (pInstance->timer > 0u) {
        if ((--pInstance->timer) > 0u) {
            pInstance->triggerEntry--;
            return; /* handle state machine only if timer has elapsed */
        }
    }

    switch (pInstance->state) {
        /****************************UNINITIALIZED***********************************/
        case SBC_STATEMACHINE_UNINITIALIZED:
            /* waiting for Initialization Request */
            stateRequest = SBC_TransferStateRequest(pInstance);
            if (stateRequest == SBC_STATE_INIT_REQUEST) {
                SBC_SaveLastStates(pInstance);
                pInstance->timer    = SBC_STATEMACHINE_SHORTTIME;
                pInstance->state    = SBC_STATEMACHINE_INITIALIZATION;
                pInstance->substate = SBC_ENTRY;
            } else if (stateRequest == SBC_STATE_NO_REQUEST) {
                /* no actual request pending */
            } else {
                pInstance->illegalRequestsCounter++; /* illegal request pending */
            }
            break;
        /****************************INITIALIZATION**********************************/
        case SBC_STATEMACHINE_INITIALIZATION:
            SBC_SaveLastStates(pInstance);
            /* Initializations done here */

            if (pInstance->substate == SBC_ENTRY) {
                /* Init SBC */
                if (STD_NOT_OK == FS85X_InitFS(pInstance->pFs85xxInstance)) {
                    /* Retry init if it fails */
                    pInstance->retryCounter++;
                    if (pInstance->retryCounter > 3u) {
                        /* Maximum number of retries -> goto error */
                        pInstance->retryCounter = 0u;
                        pInstance->timer        = SBC_STATEMACHINE_SHORTTIME;
                        pInstance->state        = SBC_STATEMACHINE_ERROR;
                        pInstance->substate     = SBC_ENTRY;
                    }
                } else {
                    /* First part of init successful -> start periodic watchdog triggering */
                    pInstance->watchdogState   = SBC_PERIODIC_WATCHDOG_ACTIVATED;
                    pInstance->watchdogTrigger = pInstance->watchdogPeriod_10ms;
                    pInstance->timer           = SBC_STATEMACHINE_SHORTTIME;
                    pInstance->substate        = SBC_INIT_RESET_FAULT_ERROR_COUNTER_PART1;
                    pInstance->retryCounter    = 0u;
                }
            } else if (pInstance->substate == SBC_INIT_RESET_FAULT_ERROR_COUNTER_PART1) {
                /* Fault error counter is reset with valid watchdog refreshes
                 * -> first get required numbers of refreshes */
                uint8_t requiredWatchdogTrigger = 0;
                if (STD_OK != FS85X_Init_ReqWDGRefreshes(pInstance->pFs85xxInstance, &requiredWatchdogTrigger)) {
                    /* Retry init if it fails */
                    pInstance->retryCounter++;
                    if (pInstance->retryCounter > 3u) {
                        /* Maximum number of retries -> goto error */
                        pInstance->retryCounter = 0;
                        pInstance->timer        = SBC_STATEMACHINE_SHORTTIME;
                        pInstance->state        = SBC_STATEMACHINE_ERROR;
                        pInstance->substate     = SBC_ENTRY;
                    }
                } else {
                    /* Wait requiredWatchdogTrigger's */
                    pInstance->retryCounter = 0;
                    pInstance->timer        = (requiredWatchdogTrigger * pInstance->watchdogPeriod_10ms);
                    pInstance->substate     = SBC_INIT_RESET_FAULT_ERROR_COUNTER_PART2;
                }
            } else if (pInstance->substate == SBC_INIT_RESET_FAULT_ERROR_COUNTER_PART2) {
                /* Check if fault error counter is zero */
                if (STD_OK != FS85X_CheckFaultErrorCounter(pInstance->pFs85xxInstance)) {
                    pInstance->retryCounter++;
                    if (pInstance->retryCounter > 3u) {
                        /* Goto error state */
                        pInstance->retryCounter = 0;
                        pInstance->state        = SBC_STATEMACHINE_ERROR;
                        pInstance->substate     = SBC_ENTRY;
                    }
                    pInstance->timer = SBC_STATEMACHINE_SHORTTIME;
                } else {
                    pInstance->substate = SBC_INITIALIZE_SAFETY_PATH_CHECK;
                    pInstance->timer    = SBC_STATEMACHINE_SHORTTIME;
                }
            } else if (pInstance->substate == SBC_INITIALIZE_SAFETY_PATH_CHECK) {
                if (STD_NOT_OK == FS85X_SafetyPathChecks(pInstance->pFs85xxInstance)) {
                    pInstance->retryCounter++;
                    if (pInstance->retryCounter > 3u) {
                        /* Goto error state */
                        pInstance->retryCounter = 0;
                        pInstance->state        = SBC_STATEMACHINE_ERROR;
                        pInstance->substate     = SBC_ENTRY;
                    }
                } else {
                    pInstance->retryCounter = 0;
                    pInstance->state        = SBC_STATEMACHINE_RUNNING;
                    pInstance->substate     = SBC_ENTRY;
                }
                pInstance->timer = SBC_STATEMACHINE_SHORTTIME;
            } else {
                FAS_ASSERT(FAS_TRAP);
            }
            break;

        /****************************RUNNING*************************************/
        case SBC_STATEMACHINE_RUNNING:
            SBC_SaveLastStates(pInstance);
            pInstance->timer = SBC_STATEMACHINE_LONGTIME;
            break;

        /****************************ERROR*************************************/
        case SBC_STATEMACHINE_ERROR:
            SBC_SaveLastStates(pInstance);
            pInstance->timer = SBC_STATEMACHINE_LONGTIME;
            break;
        /***************************DEFAULT CASE*************************************/
        default:
            /* invalid state */
            FAS_ASSERT(FAS_TRAP);
            break;
    } /* end switch (sys_systemState.state) */
    pInstance->triggerEntry--;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
