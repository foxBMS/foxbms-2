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
 * @file    bal.c
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2020-07-31 (date of last update)
 * @ingroup APPLICATION
 * @prefix  BAL
 *
 * @brief   Driver for the Balancing module
 *
 */

/*========== Includes =======================================================*/
#include "bal.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
#pragma WEAK(BAL_SaveLastStates)
extern void BAL_SaveLastStates(BAL_STATE_s *pBalancingState) {
    if (pBalancingState->lastState != pBalancingState->state) {
        pBalancingState->lastState    = pBalancingState->state;
        pBalancingState->lastSubstate = pBalancingState->substate;
    } else if (pBalancingState->lastSubstate != pBalancingState->substate) {
        pBalancingState->lastSubstate = pBalancingState->substate;
    } else {
        /* Do not set new substate as nothing changed */
        ;
    }
}

#pragma WEAK(BAL_CheckReEntrance)
extern uint8_t BAL_CheckReEntrance(BAL_STATE_s *currentState) {
    uint8_t retval = 0;

    OS_EnterTaskCritical();
    if (!currentState->triggerEntry) {
        currentState->triggerEntry++;
    } else {
        retval = 0xFF; /* multiple calls of function */
    }
    OS_ExitTaskCritical();

    return retval;
}

#pragma WEAK(BAL_TransferStateRequest)
extern BAL_STATE_REQUEST_e BAL_TransferStateRequest(BAL_STATE_s *currentState) {
    BAL_STATE_REQUEST_e retval = BAL_STATE_NO_REQUEST;

    OS_EnterTaskCritical();
    retval                     = currentState->stateRequest;
    currentState->stateRequest = BAL_STATE_NO_REQUEST;
    OS_ExitTaskCritical();

    return retval;
}

#pragma WEAK(BAL_CheckStateRequest)
extern BAL_RETURN_TYPE_e BAL_CheckStateRequest(BAL_STATE_s *pCurrentState, BAL_STATE_REQUEST_e stateRequest) {
    if (stateRequest == BAL_STATE_ERROR_REQUEST) {
        return BAL_OK;
    }
    if (stateRequest == BAL_STATE_GLOBAL_ENABLE_REQUEST) {
        pCurrentState->balancingGlobalAllowed = true;
        return BAL_OK;
    }
    if (stateRequest == BAL_STATE_GLOBAL_DISABLE_REQUEST) {
        pCurrentState->balancingGlobalAllowed = false;
        return BAL_OK;
    }
    if ((stateRequest == BAL_STATE_NO_BALANCING_REQUEST) || (stateRequest == BAL_STATE_ALLOWBALANCING_REQUEST)) {
        return BAL_OK;
    }

    if (pCurrentState->stateRequest == BAL_STATE_NO_REQUEST) {
        /* init only allowed from the uninitialized state */
        if (stateRequest == BAL_STATE_INIT_REQUEST) {
            if (pCurrentState->state == BAL_STATEMACH_UNINITIALIZED) {
                return BAL_OK;
            } else {
                return BAL_ALREADY_INITIALIZED;
            }
            /* request to forbid balancing */
        } else {
            return BAL_ILLEGAL_REQUEST;
        }
    } else {
        return BAL_REQUEST_PENDING;
    }
}

#pragma WEAK(BAL_Init)
extern STD_RETURN_TYPE_e BAL_Init(DATA_BLOCK_BALANCING_CONTROL_s *pControl) {
    DATA_READ_DATA(pControl);
    pControl->enableBalancing = 0;
    DATA_WRITE_DATA(pControl);
    return STD_OK;
}

#pragma WEAK(BAL_ProcessStateUninitalized)
extern void BAL_ProcessStateUninitalized(BAL_STATE_s *pCurrentState, BAL_STATE_REQUEST_e stateRequest) {
    if (stateRequest == BAL_STATE_INIT_REQUEST) {
        pCurrentState->timer    = BAL_STATEMACH_SHORTTIME_100ms;
        pCurrentState->state    = BAL_STATEMACH_INITIALIZATION;
        pCurrentState->substate = BAL_ENTRY;
    } else if (stateRequest == BAL_STATE_NO_REQUEST) {
        /* no actual request pending */
    } else {
        pCurrentState->errorRequestCounter++; /* illegal request pending */
    }
}

#pragma WEAK(BAL_ProcessStateInitialization)
extern void BAL_ProcessStateInitialization(BAL_STATE_s *currentState) {
    currentState->timer    = BAL_STATEMACH_SHORTTIME_100ms;
    currentState->state    = BAL_STATEMACH_INITIALIZED;
    currentState->substate = BAL_ENTRY;
}

#pragma WEAK(BAL_ProcessStateInitialized)
extern void BAL_ProcessStateInitialized(BAL_STATE_s *currentState) {
    currentState->initializationFinished = STD_OK;
    currentState->timer                  = BAL_STATEMACH_SHORTTIME_100ms;
    currentState->state                  = BAL_STATEMACH_CHECK_BALANCING;
    currentState->substate               = BAL_ENTRY;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/

/*========== Getter for static Variables (Unit Test) ========================*/
