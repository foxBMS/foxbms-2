/**
 *
 * @copyright &copy; 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    imd.c
 * @author  foxBMS Team
 * @date    2021-11-04 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS
 * @prefix  IMD
 *
 * @brief   Main driver state machine for insulation monitoring driver
 *
 * @details  This superimposed state machine initializes, enables and disables
 *           the selected Insulation Monitoring Device. Furthermore, the
 *           measurement results are evaluated and saved in the database.
 *           Requests are used to control the IMD state machine and with that
 *           the behavior of the IMDs.
 */

/*========== Includes =======================================================*/
#include "imd.h"

#include "diag.h"
#include "os.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/**
 * state machine short time definition in #IMD_Trigger calls until next state is
 * processed
 */
#define IMD_FSM_SHORT_TIME (1u)

/** Substates of the state machine */
typedef enum {
    IMD_FSM_SUBSTATE_DUMMY,               /*!< dummy state - always the first substate */
    IMD_FSM_SUBSTATE_ENTRY,               /*!< entry state - always the second substate */
    IMD_FSM_SUBSTATE_INITIALIZATION_0,    /*!< fist initialization substate */
    IMD_FSM_SUBSTATE_INITIALIZATION_1,    /*!< second initialization substate */
    IMD_FSM_SUBSTATE_INITIALIZATION_EXIT, /*!< last initialization substate */
    IMD_FSM_SUBSTATE_RUNNING_0,           /*!< fist running substate */
    IMD_FSM_SUBSTATE_RUNNING_1,           /*!< second running substate */
    IMD_FSM_SUBSTATE_RUNNING_2,           /*!< third running substate */
} IMD_FSM_SUBSTATES_e;

/** Symbolic names to check for multiple calls of #IMD_Trigger */
typedef enum {
    IMD_MULTIPLE_CALLS_NO,  /*!< no multiple calls, OK */
    IMD_MULTIPLE_CALLS_YES, /*!< multiple calls, not OK */
} IMD_CHECK_MULTIPLE_CALLS_e;

/** some struct with some information */
typedef struct {
    bool isStatemachineInitialized; /*!< true if initialized, otherwise false */
    bool switchImdDeviceOn;         /*!< true if enabling process is ongoing */
} IMD_INFORMATION_s;

/** This struct describes the state of the monitoring instance */
typedef struct {
    uint8_t counter;                               /*!< general purpose counter */
    uint16_t timer;                                /*!< timer of the state */
    uint8_t triggerEntry;                          /*!< trigger entry of the state */
    IMD_STATE_REQUEST_e stateRequest;              /*!< current state request made to the state machine */
    IMD_FSM_STATES_e nextState;                    /*!< next state of the FSM */
    IMD_FSM_STATES_e currentState;                 /*!< current state of the FSM */
    IMD_FSM_STATES_e previousState;                /*!< previous state of the FSM */
    IMD_FSM_SUBSTATES_e nextSubstate;              /*!< next substate of the FSM */
    IMD_FSM_SUBSTATES_e currentSubstate;           /*!< current substate of the FSM */
    IMD_FSM_SUBSTATES_e previousSubstate;          /*!< previous substate of the FSM */
    IMD_INFORMATION_s information;                 /*!< Some information to be stored */
    DATA_BLOCK_INSULATION_MONITORING_s *pTableImd; /*!< Pointer to IMD database entry */
} IMD_STATE_s;

/*========== Static Constant and Variable Definitions =======================*/
static DATA_BLOCK_INSULATION_MONITORING_s imd_tableInsulationMonitoring = {
    .header.uniqueId = DATA_BLOCK_ID_INSULATION_MONITORING};

/** global IMD state */
static IMD_STATE_s imd_state = {
    .timer                                 = 0u,
    .triggerEntry                          = 0u,
    .nextState                             = IMD_FSM_STATE_DUMMY,
    .stateRequest                          = IMD_STATE_NO_REQUEST,
    .currentState                          = IMD_FSM_STATE_HAS_NEVER_RUN,
    .previousState                         = IMD_FSM_STATE_DUMMY,
    .nextSubstate                          = IMD_FSM_SUBSTATE_DUMMY,
    .currentSubstate                       = IMD_FSM_SUBSTATE_DUMMY,
    .previousSubstate                      = IMD_FSM_SUBSTATE_DUMMY,
    .information.isStatemachineInitialized = false,
    .information.switchImdDeviceOn         = false,
    .pTableImd                             = &imd_tableInsulationMonitoring,
};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   sets the current state request of the state variable #imd_state.
 * @details This function is used to make a state request to the state machine,
 *          e.g, initialize state machine or shut-down state machine.
 *          It calls #IMD_CheckStateRequest() to check if the request is valid.
 *          The state request is rejected if is not valid. The result of the
 *          check is returned immediately, so that the requester can act in
 *          case it made a non-valid state request.
 * @param[in]     stateRequest   state request to set
 * @param[in,out] pImdState pointer to state variable of IMD state machine
 * @return  OK if request has been accepted, otherwise error reason
 */
static IMD_RETURN_TYPE_e IMD_SetStateRequest(IMD_STATE_s *pImdState, IMD_STATE_REQUEST_e stateRequest);

/**
 * @brief       checks the state requests that are made.
 * @details     This function checks the validity of the state requests. The
 *              results of the checked state request is returned immediately.
 * @param[in]     stateRequest  state request to be checked
 * @param[in,out] pImdState pointer to state variable of IMD state machine
 * @return      result of the state request that was made
 */
static IMD_RETURN_TYPE_e IMD_CheckStateRequest(IMD_STATE_s *pImdState, IMD_STATE_REQUEST_e stateRequest);

/**
 * @brief   transfers the current state request to the state machine.
 * @details This function takes the current state request from #imd_state
 *          transfers it to the state machine. It resets the value from
 *          #imd_state to #IMD_STATE_NO_REQUEST
 * @param[in,out] pImdState pointer to state variable of IMD state machine
 * @return  current state request
 */
static IMD_STATE_REQUEST_e IMD_TransferStateRequest(IMD_STATE_s *pImdState);

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
 * @param[in,out] pImdState pointer to state variable of IMD state machine
 * @return  #IMD_MULTIPLE_CALLS_YES if there were multiple calls,
 *          #IMD_MULTIPLE_CALLS_NO otherwise
 */
static IMD_CHECK_MULTIPLE_CALLS_e IMD_CheckMultipleCalls(IMD_STATE_s *pImdState);

/**
 * @brief   Sets the next state, the next substate and the timer value
 *          of the state variable.
 * @param[in,out] pImdState      state of the example state machine
 * @param[in]     nextState      state to be transferred into
 * @param[in]     nextSubstate   substate to be transferred into
 * @param[in]     idleTime       wait time for the state machine
 */
static void IMD_SetState(
    IMD_STATE_s *pImdState,
    IMD_FSM_STATES_e nextState,
    IMD_FSM_SUBSTATES_e nextSubstate,
    uint16_t idleTime);

/**
 * @brief   Sets the next substate and the timer value
 *          of the state variable.
 * @param[in,out] pImdState       state of the example state machine
 * @param[in]     nextSubstate   substate to be transferred into
 * @param[in]     idleTime       wait time for the state machine
 */
static void IMD_SetSubstate(IMD_STATE_s *pImdState, IMD_FSM_SUBSTATES_e nextSubstate, uint16_t idleTime);

/**
 * @brief   Defines the state transitions
 * @details This function contains the implementation of the state
 *          machine, i.e., the sequence of states and substates.
 *          It is called by the trigger function every time
 *          the state machine timer has a non-zero value.
 * @param[in,out]   pImdState state of the example state machine
 * @return  Always #STD_OK
 */
static STD_RETURN_TYPE_e IMD_RunStateMachine(IMD_STATE_s *pImdState);

/**
 * @brief   Evaluates measurement perform by IMD driver
 * @details This function evaluates the insulation measurement performed by the
 *          selected IMD driver and updates the database entry.
 * @param[in,out] pTableInsulationMonitoring pointer to insulation monitoring
 *                                           database entry
 * @return  Always #IMD_REQUEST_OK
 */
static STD_RETURN_TYPE_e IMD_EvaluateInsulationMeasurement(
    DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring);

/*========== Static Function Implementations ================================*/
static IMD_RETURN_TYPE_e IMD_SetStateRequest(IMD_STATE_s *pImdState, IMD_STATE_REQUEST_e stateRequest) {
    FAS_ASSERT(pImdState != NULL_PTR);
    /* AXIVION Routine Generic-MissingParameterAssert: stateRequest: parameter accepts whole range */
    IMD_RETURN_TYPE_e retVal = IMD_REQUEST_OK;

    OS_EnterTaskCritical();
    retVal = IMD_CheckStateRequest(pImdState, stateRequest);

    if (retVal == IMD_REQUEST_OK) {
        pImdState->stateRequest = stateRequest;
    }
    OS_ExitTaskCritical();

    return retVal;
}

static IMD_RETURN_TYPE_e IMD_CheckStateRequest(IMD_STATE_s *pImdState, IMD_STATE_REQUEST_e stateRequest) {
    FAS_ASSERT(pImdState != NULL_PTR);
    /* AXIVION Routine Generic-MissingParameterAssert: stateRequest: parameter accepts whole range */
    IMD_RETURN_TYPE_e retval = IMD_REQUEST_PENDING;
    if (pImdState->stateRequest == IMD_STATE_NO_REQUEST) {
        /* init only allowed from the uninitialized state */
        if (stateRequest == IMD_STATE_INITIALIZE_REQUEST) {
            if (pImdState->currentState == IMD_FSM_STATE_UNINITIALIZED) {
                retval = IMD_REQUEST_OK;
            } else {
                retval = IMD_ALREADY_INITIALIZED;
            }
        } else if (stateRequest == IMD_STATE_SWITCH_ON_REQUEST) {
            if (pImdState->currentState == IMD_FSM_STATE_SHUTDOWN) {
                retval = IMD_REQUEST_OK;
            } else if (pImdState->currentState == IMD_FSM_STATE_IMD_ENABLE) {
                retval = IMD_REQUEST_OK;
            } else {
                retval = IMD_ILLEGAL_REQUEST;
            }
        } else {
            retval = IMD_ILLEGAL_REQUEST;
        }
    } else {
        /* Request pending */
    }
    return retval;
}

static IMD_STATE_REQUEST_e IMD_TransferStateRequest(IMD_STATE_s *pImdState) {
    FAS_ASSERT(pImdState != NULL_PTR);
    OS_EnterTaskCritical();
    IMD_STATE_REQUEST_e retval = pImdState->stateRequest;
    pImdState->stateRequest    = IMD_STATE_NO_REQUEST;
    OS_ExitTaskCritical();
    return retval;
}

static IMD_CHECK_MULTIPLE_CALLS_e IMD_CheckMultipleCalls(IMD_STATE_s *pImdState) {
    FAS_ASSERT(pImdState != NULL_PTR);
    IMD_CHECK_MULTIPLE_CALLS_e multipleCalls = IMD_MULTIPLE_CALLS_NO;
    OS_EnterTaskCritical();
    if (pImdState->triggerEntry == 0u) {
        pImdState->triggerEntry++;
    } else {
        multipleCalls = IMD_MULTIPLE_CALLS_YES; /* multiple call of function IMD_Trigger for instance pImdState */
    }
    OS_ExitTaskCritical();
    return multipleCalls;
}

static void IMD_SetState(
    IMD_STATE_s *pImdState,
    IMD_FSM_STATES_e nextState,
    IMD_FSM_SUBSTATES_e nextSubstate,
    uint16_t idleTime) {
    FAS_ASSERT(pImdState != NULL_PTR);
    /* AXIVION Routine Generic-MissingParameterAssert: idleTime: parameter accepts whole range */
    /* AXIVION Routine Generic-MissingParameterAssert: nextState: parameter accepts whole range */
    /* AXIVION Routine Generic-MissingParameterAssert: nextSubstate: parameter accepts whole range */
    bool earlyExit = false;

    pImdState->timer = idleTime;

    if ((pImdState->currentState == nextState) && (pImdState->currentSubstate == nextSubstate)) {
        /* Next state and next substate equal to current state and substate: nothing to do */
        pImdState->nextState    = IMD_FSM_STATE_DUMMY;    /* no state transistion required -> reset */
        pImdState->nextSubstate = IMD_FSM_SUBSTATE_DUMMY; /* no substate transistion required -> reset */
        earlyExit               = true;
    }

    if (earlyExit == false) {
        if (pImdState->currentState != nextState) {
            /* Next state is different: switch to it and set substate to entry value */
            pImdState->previousState    = pImdState->currentState;
            pImdState->currentState     = nextState;
            pImdState->previousSubstate = pImdState->currentSubstate;
            pImdState->currentSubstate  = IMD_FSM_SUBSTATE_ENTRY; /* Use entry state after a top level state change */
            pImdState->nextState        = IMD_FSM_STATE_DUMMY;    /* no state transistion required -> reset */
            pImdState->nextSubstate     = IMD_FSM_SUBSTATE_DUMMY; /* no substate transistion required -> reset */
        } else if (pImdState->currentSubstate != nextSubstate) {
            /* Only the next substate is different, switch to it */
            IMD_SetSubstate(pImdState, nextSubstate, idleTime);
        } else {
            ;
        }
    }
}

static void IMD_SetSubstate(IMD_STATE_s *pImdState, IMD_FSM_SUBSTATES_e nextSubstate, uint16_t idleTime) {
    FAS_ASSERT(pImdState != NULL_PTR);
    /* AXIVION Routine Generic-MissingParameterAssert: nextSubstate: parameter accepts whole range */
    /* AXIVION Routine Generic-MissingParameterAssert: idleTime: parameter accepts whole range */
    pImdState->timer            = idleTime;
    pImdState->previousSubstate = pImdState->currentSubstate;
    pImdState->currentSubstate  = nextSubstate;
    pImdState->nextSubstate     = IMD_FSM_SUBSTATE_DUMMY; /* substate has been set, now reset value for nextSubstate */
}

static STD_RETURN_TYPE_e IMD_RunStateMachine(IMD_STATE_s *pImdState) {
    FAS_ASSERT(pImdState != NULL_PTR);
    STD_RETURN_TYPE_e ranStateMachine = STD_OK;
    IMD_FSM_STATES_e nextState        = IMD_FSM_STATE_DUMMY;
    IMD_STATE_REQUEST_e stateRequest  = IMD_STATE_NO_REQUEST;
    switch (pImdState->currentState) {
        /********************************************** STATE: HAS NEVER RUN */
        case IMD_FSM_STATE_HAS_NEVER_RUN:
            /* Nothing to do, just transfer */
            IMD_SetState(pImdState, IMD_FSM_STATE_UNINITIALIZED, IMD_FSM_SUBSTATE_ENTRY, IMD_FSM_SHORT_TIME);
            break;

        /********************************************** STATE: UNINITIALIZED */
        case IMD_FSM_STATE_UNINITIALIZED:
            /* Transition to initialization state, one the state request has been received */
            stateRequest = IMD_TransferStateRequest(pImdState);
            if (stateRequest == IMD_STATE_INITIALIZE_REQUEST) {
                IMD_SetState(pImdState, IMD_FSM_STATE_INITIALIZATION, IMD_FSM_SUBSTATE_ENTRY, IMD_FSM_SHORT_TIME);
            } else {
                /* Do nothing. Stay in state. */
            }
            break;

        /********************************************* STATE: INITIALIZATION */
        case IMD_FSM_STATE_INITIALIZATION:
            nextState = IMD_ProcessInitializationState();
            if (nextState == IMD_FSM_STATE_INITIALIZATION) {
                /* staying in state, processed by state function */
            } else if (nextState == IMD_FSM_STATE_ERROR) {
                IMD_SetState(pImdState, IMD_FSM_STATE_ERROR, IMD_FSM_SUBSTATE_ENTRY, IMD_FSM_SHORT_TIME);
            } else if (nextState == IMD_FSM_STATE_IMD_ENABLE) {
                pImdState->information.isStatemachineInitialized = true;
                IMD_SetState(pImdState, IMD_FSM_STATE_IMD_ENABLE, IMD_FSM_SUBSTATE_ENTRY, IMD_FSM_SHORT_TIME);
            } else {
                FAS_ASSERT(FAS_TRAP); /* Something went wrong */
            }
            break;

        case IMD_FSM_STATE_IMD_ENABLE:
            /* Transition to running state, once the state request has been received */
            stateRequest = IMD_TransferStateRequest(pImdState);
            if (stateRequest == IMD_STATE_SWITCH_ON_REQUEST) {
                pImdState->information.switchImdDeviceOn = true;
            }
            if (pImdState->information.switchImdDeviceOn == true) {
                nextState = IMD_ProcessEnableState();
                if (nextState == IMD_FSM_STATE_RUNNING) {
                    /* Set flag that IMD is running. Do not update the database entry here. The database entry will be
                     * updated after first successful measurement in IMD_FSM_STATE_RUNNING state */
                    pImdState->pTableImd->isImdRunning       = true;
                    pImdState->information.switchImdDeviceOn = false;
                    IMD_SetState(pImdState, IMD_FSM_STATE_RUNNING, IMD_FSM_SUBSTATE_ENTRY, IMD_FSM_SHORT_TIME);
                } else if (nextState == IMD_FSM_STATE_ERROR) {
                    IMD_SetState(pImdState, IMD_FSM_STATE_ERROR, IMD_FSM_SUBSTATE_ENTRY, IMD_FSM_SHORT_TIME);
                } else {
                    /* Do nothing as the process to activate the IMD device is ongoing */
                }
            } else {
                /* Do nothing. Stay in state. */
            }
            break;

        /**************************************************** STATE: RUNNING */
        case IMD_FSM_STATE_RUNNING:
            stateRequest = IMD_TransferStateRequest(pImdState);
            if (stateRequest == IMD_STATE_SHUTDOWN_REQUEST) {
                IMD_SetState(pImdState, IMD_FSM_STATE_SHUTDOWN, IMD_FSM_SUBSTATE_ENTRY, IMD_FSM_SHORT_TIME);
            } else {
                nextState = IMD_ProcessRunningState(pImdState->pTableImd);
                /* Evaluate measurement results */
                IMD_EvaluateInsulationMeasurement(pImdState->pTableImd);
                if (nextState == IMD_FSM_STATE_RUNNING) {
                    /* staying in state, processed by state function */
                } else {
                    IMD_SetState(pImdState, IMD_FSM_STATE_ERROR, IMD_FSM_SUBSTATE_ENTRY, IMD_FSM_SHORT_TIME);
                }
            }
            break;

        case IMD_FSM_STATE_SHUTDOWN:
            nextState = IMD_ProcessShutdownState();
            if (nextState == IMD_FSM_STATE_IMD_ENABLE) {
                /* Set flag, that IMD is deactivated. Update database entry */
                pImdState->pTableImd->isImdRunning = false;
                (void)DATA_WRITE_DATA(&imd_tableInsulationMonitoring);
                IMD_SetState(pImdState, IMD_FSM_STATE_IMD_ENABLE, IMD_FSM_SUBSTATE_ENTRY, IMD_FSM_SHORT_TIME);
            } else if (nextState == IMD_FSM_STATE_ERROR) {
                IMD_SetState(pImdState, IMD_FSM_STATE_ERROR, IMD_FSM_SUBSTATE_ENTRY, IMD_FSM_SHORT_TIME);
            } else {
                /* staying in state, processed by state function */
            }
            break;

        /****************************************************** STATE: ERROR */
        case IMD_FSM_STATE_ERROR:
            /* Issue: 621 */
            FAS_ASSERT(FAS_TRAP);
            break;

        /**************************************************** STATE: DEFAULT */
        default:
            /* all cases must be processed, trap if unknown state arrives */
            FAS_ASSERT(FAS_TRAP);
            break;
    }
    /* Increment general purpose counter */
    pImdState->counter++;
    return ranStateMachine;
}

static STD_RETURN_TYPE_e IMD_EvaluateInsulationMeasurement(
    DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring) {
    FAS_ASSERT(pTableInsulationMonitoring != NULL_PTR);
    /* Assume resistance value as good if no valid measurement values are detected */
    bool lowResistanceDetected = false;

    /* Check if measured resistance value is valid */
    if (pTableInsulationMonitoring->isInsulationMeasurementValid == true) {
        (void)DIAG_Handler(DIAG_ID_INSULATION_MEASUREMENT_VALID, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
        if (pTableInsulationMonitoring->insulationResistance_kOhm < IMD_ERROR_THRESHOLD_INSULATION_RESISTANCE_kOhm) {
            lowResistanceDetected = true;
        }
    } else {
        (void)DIAG_Handler(DIAG_ID_INSULATION_MEASUREMENT_VALID, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
    }

    /* Check if flags of IMD are valid, e.g. status pin */
    if (pTableInsulationMonitoring->areDeviceFlagsValid == true) {
        if (pTableInsulationMonitoring->dfIsCriticalResistanceDetected == true) {
            lowResistanceDetected = true;
        }
        if (pTableInsulationMonitoring->dfIsChassisFaultDetected == true) {
            (void)DIAG_Handler(DIAG_ID_INSULATION_GROUND_ERROR, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
        } else {
            (void)DIAG_Handler(DIAG_ID_INSULATION_GROUND_ERROR, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
        }
    }

    /* Set diagnosis entry depending on measured insulation resistance and critical threshold flag */
    if (lowResistanceDetected == true) {
        (void)DIAG_Handler(DIAG_ID_LOW_INSULATION_RESISTANCE_ERROR, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
    } else {
        (void)DIAG_Handler(DIAG_ID_LOW_INSULATION_RESISTANCE_ERROR, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
    }

    /* Set warning threshold flag depending on flag */
    if (pTableInsulationMonitoring->dfIsWarnableResistanceDetected == true) {
        (void)DIAG_Handler(DIAG_ID_LOW_INSULATION_RESISTANCE_WARNING, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
    } else {
        (void)DIAG_Handler(DIAG_ID_LOW_INSULATION_RESISTANCE_WARNING, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
    }

    /* Write database entry */
    DATA_WRITE_DATA(pTableInsulationMonitoring);

    /* Issue: 621 */

    return STD_OK;
}

/*========== Extern Function Implementations ================================*/
extern IMD_RETURN_TYPE_e IMD_RequestInitialization(void) {
    return IMD_SetStateRequest(&imd_state, IMD_STATE_INITIALIZE_REQUEST);
}

extern IMD_RETURN_TYPE_e IMD_RequestInsulationMeasurement(void) {
    return IMD_SetStateRequest(&imd_state, IMD_STATE_SWITCH_ON_REQUEST);
}

extern IMD_RETURN_TYPE_e IMD_RequestMeasurementStop(void) {
    return IMD_SetStateRequest(&imd_state, IMD_STATE_SHUTDOWN_REQUEST);
}

extern bool IMD_GetInitializationState(void) {
    return imd_state.information.isStatemachineInitialized;
}

extern STD_RETURN_TYPE_e IMD_Trigger(void) {
    bool earlyExit                = false;
    STD_RETURN_TYPE_e returnValue = STD_OK;

    /* Check multiple calls of function */
    if (IMD_MULTIPLE_CALLS_YES == IMD_CheckMultipleCalls(&imd_state)) {
        returnValue = STD_NOT_OK;
        earlyExit   = true;
    }

    if (earlyExit == false) {
        if (imd_state.timer > 0u) {
            if ((--imd_state.timer) > 0u) {
                imd_state.triggerEntry--;
                returnValue = STD_OK;
                earlyExit   = true;
            }
        }
    }

    if (earlyExit == false) {
        IMD_RunStateMachine(&imd_state);
        imd_state.triggerEntry--;
    }
    return returnValue;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
