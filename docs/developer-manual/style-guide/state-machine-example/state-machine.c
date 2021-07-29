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
 * @file    state-machine.c
 * @author  foxBMS Team
 * @date    2020-10-29 (date of creation)
 * @updated 2020-11-09 (date of last update)
 * @ingroup STATE_MACHINE
 * @prefix  EG
 *
 * @brief   Implementation of some driver that needs a state machine
 *
 */

/*========== Includes =======================================================*/
#include "state-machine.h"

/*========== Macros and Definitions =========================================*/
/**
 * statemachine short time definition in #EG_Trigger calls until next state is
 * processed
 */
#define EG_FSM_SHORT_TIME (1u)

/**
 * statemachine medium time definition in #EG_Trigger calls until next
 * state/substate is processed
 */
#define EG_FSM_MEDIUM_TIME (5u)

/**
 * statemachine long time definition in #EG_Trigger calls until next
 * state/substate is processed
 */
#define EG_FSM_LONG_TIME (10u)

/** Symbolic names to check for multiple calls of #EG_Trigger */
typedef enum EG_CHECK_MULTIPLE_CALLS {
    EG_MULTIPLE_CALLS_NO,  /*!< no multiple calls, OK */
    EG_MULTIPLE_CALLS_YES, /*!< multiple calls, not OK */
} EG_CHECK_MULTIPLE_CALLS_e;

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/** local instance of the driver-state */
EG_STATE_s eg_state = {
    .timer            = 0,
    .triggerEntry     = 0,
    .nextState        = EG_FSM_STATE_HAS_NEVER_RUN,
    .currentState     = EG_FSM_STATE_HAS_NEVER_RUN,
    .previousState    = EG_FSM_STATE_HAS_NEVER_RUN,
    .nextSubstate     = EG_FSM_SUBSTATE_DUMMY,
    .currentSubstate  = EG_FSM_SUBSTATE_DUMMY,
    .previousSubstate = EG_FSM_SUBSTATE_DUMMY,
    .information.r0   = 0,
    .information.r1   = 0,
    .information.r2   = 0,
};

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   check for multiple calls of state machine trigger function
 * @details The trigger function is not reentrant, which means it cannot
 *          be called multiple times. This functions increments the
 *          triggerEntry counter once and must be called each time the
 *          trigger function is called. If triggerEntry is greater than
 *          one, there were multiple calls. For this function to work,
 *          triggerEntry must be decremented each time the trigger function
 *          is called, even if no processing do because the timer is
 *          non-zero.
 * @param   pEgState state of the fake state machine
 * @return  #EG_MULTIPLE_CALLS_YES if there were multiple calls,
 *          #EG_MULTIPLE_CALLS_NO otherwise
 */
static EG_CHECK_MULTIPLE_CALLS_e EG_CheckMultipleCalls(EG_STATE_s *pEgState);

/**
 * @brief   Sets the next state, the next substate and the timer value
 *          of the state variable.
 * @param   pEgState       state of the example state machine
 * @param   nextState      state to be transferred into
 * @param   nextSubstate   substate to be transferred into
 * @param   idleTime       wait time for the state machine
 */
static void EG_SetState(
    EG_STATE_s *pEgState,
    EG_FSM_STATES_e nextState,
    EG_FSM_SUBSTATES_e nextSubstate,
    uint16_t idleTime);

/**
 * @brief   Sets the next substate and the timer value
 *          of the state variable.
 * @param   pEgState       state of the example state machine
 * @param   nextSubstate   substate to be transferred into
 * @param   idleTime       wait time for the state machine
 */
static void EG_SetSubstate(EG_STATE_s *pEgState, EG_FSM_SUBSTATES_e nextSubstate, uint16_t idleTime);

/**
 * @brief   dummy function for initialization substate
 *          #EG_FSM_SUBSTATE_INITIALIZATION_0
 * @return  returns always true
 */
static bool EG_SomeInitializationFunction0(void);

/**
 * @brief   dummy function for initialization substate
 *          #EG_FSM_SUBSTATE_INITIALIZATION_1
 * @return  returns always true
 */
static bool EG_SomeInitializationFunction1(void);

/**
 * @brief   dummy function to check if the initialization
 *          step of the state machine was successful
 *          (#EG_FSM_SUBSTATE_INITIALIZATION_1)
 * @return  returns always true
 */
static bool EG_SomeInitializationFunctionExit(void);

/**
 * @brief   dummy function making a test to determine
 *          the outcome of substate #EG_FSM_SUBSTATE_RUNNING_0
 * @return  returns always true
 */
static bool EG_SomeRunningFunction0(void);

/**
 * @brief   dummy function making a test to determine
 *          the outcome of substate EG_FSM_SUBSTATE_RUNNING_1
 * @return  returns always true
 */
static bool EG_SomeRunningFunction1(void);

/**
 * @brief   dummy function making a test to determine
 *          the outcome of substate EG_FSM_SUBSTATE_RUNNING_2
 * @return  returns always true
 */
static bool EG_SomeRunningFunction2(void);

/**
 * @brief   Processes the initialization state
 * @param   pEgState state of the example state machine
 * @return  Always #STD_OK
 */
static EG_FSM_STATES_e EG_ProcessInitializationState(EG_STATE_s *pEgState);

/**
 * @brief   Processes the running state
 * @param   pEgState state of the example state machine
 * @return  Always #STD_OK
 */
static EG_FSM_STATES_e EG_ProcessRunningState(EG_STATE_s *pEgState);

/**
 * @brief   Defines the state transitions
 * @details This function contains the implementation of the state
 *          machine, i.e., the sequence of states and substates.
 *          It is called by the trigger function every time
 *          the state machine timer has a non-zero value.
 * @param   pEgState state of the example state machine
 * @return  Always #STD_OK
 */
static STD_RETURN_TYPE_e EG_RunStateMachine(EG_STATE_s *pEgState);

/*========== Static Function Implementations ================================*/

static EG_CHECK_MULTIPLE_CALLS_e EG_CheckMultipleCalls(EG_STATE_s *pEgState) {
    FAS_ASSERT(pEgState != NULL_PTR);
    EG_CHECK_MULTIPLE_CALLS_e multipleCalls = EG_MULTIPLE_CALLS_NO;
    OS_EnterTaskCritical();
    if (pEgState->triggerEntry == 0u) {
        pEgState->triggerEntry++;
    } else {
        multipleCalls = EG_MULTIPLE_CALLS_YES; /* multiple call of function EG_Trigger for instance pEgState */
    }
    OS_ExitTaskCritical();
    return multipleCalls;
}

static void EG_SetState(
    EG_STATE_s *pEgState,
    EG_FSM_STATES_e nextState,
    EG_FSM_SUBSTATES_e nextSubstate,
    uint16_t idleTime) {
    FAS_ASSERT(pEgState != NULL_PTR);
    bool earlyExit = false;

    pEgState->timer = idleTime;

    if ((pEgState->currentState == nextState) && (pEgState->currentSubstate == nextSubstate)) {
        /* Next state and next substate equal to current state and substate: nothing to do */
        pEgState->nextState    = EG_FSM_STATE_DUMMY;    /* no state transistion required -> reset */
        pEgState->nextSubstate = EG_FSM_SUBSTATE_DUMMY; /* no substate transistion required -> reset */
        earlyExit              = true;
    }

    if (earlyExit == false) {
        if (pEgState->currentState != nextState) {
            /* Next state is different: switch to it and set substate to entry value */
            pEgState->previousState    = pEgState->currentState;
            pEgState->currentState     = nextState;
            pEgState->previousSubstate = pEgState->currentSubstate;
            pEgState->currentSubstate  = EG_FSM_SUBSTATE_ENTRY; /* Use entry state after a top level state change */
            pEgState->nextState        = EG_FSM_STATE_DUMMY;    /* no state transistion required -> reset */
            pEgState->nextSubstate     = EG_FSM_SUBSTATE_DUMMY; /* no substate transistion required -> reset */
        } else if (pEgState->currentSubstate != nextSubstate) {
            /* Only the next substate is different, switch to it */
            EG_SetSubstate(pEgState, nextSubstate, idleTime);
        } else {
            ;
        }
    }
}

static void EG_SetSubstate(EG_STATE_s *pEgState, EG_FSM_SUBSTATES_e nextSubstate, uint16_t idleTime) {
    FAS_ASSERT(pEgState != NULL_PTR);
    pEgState->timer            = idleTime;
    pEgState->previousSubstate = pEgState->currentSubstate;
    pEgState->currentSubstate  = nextSubstate;
    pEgState->nextSubstate     = EG_FSM_SUBSTATE_DUMMY; /* substate has been set, now reset value for nextSubstate */
}

static bool EG_SomeInitializationFunction0(void) {
    return true;
}

static bool EG_SomeInitializationFunction1(void) {
    return true;
}

static bool EG_SomeInitializationFunctionExit(void) {
    return true;
}

static bool EG_SomeRunningFunction0(void) {
    return true;
}

static bool EG_SomeRunningFunction1(void) {
    return true;
}

static bool EG_SomeRunningFunction2(void) {
    return true;
}

static EG_FSM_STATES_e EG_ProcessInitializationState(EG_STATE_s *pEgState) {
    EG_FSM_STATES_e nextState = EG_FSM_STATE_INITIALIZATION; /* default behavior: stay in state */
    switch (pEgState->currentSubstate) {
        case EG_FSM_SUBSTATE_ENTRY:
            /* Nothing to do, just transfer to next substate */
            EG_SetSubstate(pEgState, EG_FSM_SUBSTATE_INITIALIZATION_0, EG_FSM_SHORT_TIME);
            break;

        case EG_FSM_SUBSTATE_INITIALIZATION_0:
            if (true == EG_SomeInitializationFunction0()) {
                EG_SetSubstate(pEgState, EG_FSM_SUBSTATE_INITIALIZATION_1, EG_FSM_SHORT_TIME);
            } else {
                /* Something went wrong, so transition to error state */
                nextState = EG_FSM_STATE_ERROR;
            }
            break;

        case EG_FSM_SUBSTATE_INITIALIZATION_1:
            if (true == EG_SomeInitializationFunction1()) {
                EG_SetSubstate(pEgState, EG_FSM_SUBSTATE_INITIALIZATION_EXIT, EG_FSM_SHORT_TIME);
            } else {
                /* Something went wrong, so transition to error state */
                nextState = EG_FSM_STATE_ERROR;
            }
            break;

        case EG_FSM_SUBSTATE_INITIALIZATION_EXIT:
            if (true == EG_SomeInitializationFunctionExit()) {
                /* Initialization was successful, so transition to running state */
                nextState = EG_FSM_STATE_RUNNING;
            } else {
                /* Something went wrong, so transition to error state */
                nextState = EG_FSM_STATE_ERROR;
            }
            break;

        default:
            FAS_ASSERT(FAS_TRAP);
            break;
    }
    return nextState;
}

static EG_FSM_STATES_e EG_ProcessRunningState(EG_STATE_s *pEgState) {
    EG_FSM_STATES_e nextState = EG_FSM_STATE_RUNNING; /* default behavior: stay in state */
    switch (pEgState->currentSubstate) {
        case EG_FSM_SUBSTATE_ENTRY:
            /* Nothing to do, just transfer to next substate */
            EG_SetSubstate(pEgState, EG_FSM_SUBSTATE_RUNNING_0, EG_FSM_SHORT_TIME);
            break;

        case EG_FSM_SUBSTATE_RUNNING_0:
            if (true == EG_SomeRunningFunction0()) {
                EG_SetSubstate(pEgState, EG_FSM_SUBSTATE_RUNNING_1, EG_FSM_SHORT_TIME);
            } else {
                /* Something went wrong, so transition to error state */
                nextState = EG_FSM_STATE_ERROR;
            }
            break;

        case EG_FSM_SUBSTATE_RUNNING_1:
            if (true == EG_SomeRunningFunction1()) {
                EG_SetSubstate(pEgState, EG_FSM_SUBSTATE_RUNNING_2, EG_FSM_SHORT_TIME);
            } else {
                /* Something went wrong, so transition to error state */
                nextState = EG_FSM_STATE_ERROR;
            }
            break;

        case EG_FSM_SUBSTATE_RUNNING_2:
            if (true == EG_SomeRunningFunction2()) {
                EG_SetSubstate(pEgState, EG_FSM_SUBSTATE_RUNNING_0, EG_FSM_SHORT_TIME);
            } else {
                /* Something went wrong, so transition to error state */
                nextState = EG_FSM_STATE_ERROR;
            }
            break;

        default:
            FAS_ASSERT(FAS_TRAP);
            break;
    }

    return nextState;
}

static STD_RETURN_TYPE_e EG_RunStateMachine(EG_STATE_s *pEgState) {
    STD_RETURN_TYPE_e ranStateMachine = STD_OK;
    EG_FSM_STATES_e nextState         = EG_FSM_STATE_DUMMY;
    switch (pEgState->currentState) {
        /********************************************** STATE: HAS NEVER RUN */
        case EG_FSM_STATE_HAS_NEVER_RUN:
            /* Nothing to do, just transfer */
            EG_SetState(pEgState, EG_FSM_STATE_UNINITIALIZED, EG_FSM_SUBSTATE_ENTRY, EG_FSM_SHORT_TIME);
            break;

        /********************************************** STATE: UNINITIALIZED */
        case EG_FSM_STATE_UNINITIALIZED:
            /* Nothing to do, just transfer */
            EG_SetState(pEgState, EG_FSM_STATE_INITIALIZATION, EG_FSM_SUBSTATE_ENTRY, EG_FSM_SHORT_TIME);
            break;

        /********************************************* STATE: INITIALIZATION */
        case EG_FSM_STATE_INITIALIZATION:
            nextState = EG_ProcessInitializationState(pEgState);
            if (nextState == EG_FSM_STATE_INITIALIZATION) {
                /* staying in state, processed by state function */
            } else if (nextState == EG_FSM_STATE_ERROR) {
                EG_SetState(pEgState, EG_FSM_STATE_ERROR, EG_FSM_SUBSTATE_ENTRY, EG_FSM_SHORT_TIME);
            } else if (nextState == EG_FSM_STATE_RUNNING) {
                EG_SetState(pEgState, EG_FSM_STATE_RUNNING, EG_FSM_SUBSTATE_ENTRY, EG_FSM_SHORT_TIME);
            } else {
                FAS_ASSERT(FAS_TRAP); /* Something went wrong */
            }
            break;

        /**************************************************** STATE: RUNNING */
        case EG_FSM_STATE_RUNNING:
            nextState = EG_ProcessRunningState(pEgState);
            if (nextState == EG_FSM_STATE_RUNNING) {
                /* staying in state, processed by state function */
            } else if (nextState == EG_FSM_STATE_ERROR) {
                EG_SetState(pEgState, EG_FSM_STATE_ERROR, EG_FSM_SUBSTATE_ENTRY, EG_FSM_SHORT_TIME);
            } else {
                FAS_ASSERT(FAS_TRAP); /* Something went wrong */
            }
            break;

        /****************************************************** STATE: ERROR */
        case EG_FSM_STATE_ERROR:
            /* implement error processing here or trap */
            break;

        /**************************************************** STATE: DEFAULT */
        default:
            /* all cases must be processed, trap if unknown state arrives */
            FAS_ASSERT(FAS_TRAP);
            break;
    }

    return ranStateMachine;
}

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e EG_Trigger(EG_STATE_s *pEgState) {
    FAS_ASSERT(pEgState != NULL_PTR);
    bool earlyExit                = false;
    STD_RETURN_TYPE_e returnValue = STD_OK;

    /* Check multiple calls of function */
    if (EG_MULTIPLE_CALLS_YES == EG_CheckMultipleCalls(pEgState)) {
        returnValue = STD_NOT_OK;
        earlyExit   = true;
    }

    if (earlyExit == false) {
        if (pEgState->timer > 0u) {
            if ((--pEgState->timer) > 0u) {
                pEgState->triggerEntry--;
                returnValue = STD_OK;
                earlyExit   = true;
            }
        }
    }

    if (earlyExit == false) {
        EG_RunStateMachine(pEgState);
        pEgState->triggerEntry--;
    }
    return returnValue;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
