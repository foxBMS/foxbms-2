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
 * @file    sys.c
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2021-10-12 (date of last update)
 * @ingroup ENGINE
 * @prefix  SYS
 *
 * @brief   Sys driver implementation
 */

/*========== Includes =======================================================*/
#include "sys.h"

#include "algorithm.h"
#include "bal.h"
#include "bms.h"
#include "can.h"
#include "contactor.h"
#include "diag.h"
#include "fram.h"
#include "imd.h"
#include "interlock.h"
#include "meas.h"
#include "os.h"
#include "sbc.h"
#include "sof.h"
#include "state_estimation.h"

/*========== Macros and Definitions =========================================*/

/** Saves the last state and the last substate */
#define SYS_SAVELASTSTATES(x)       \
    (x)->lastState    = (x)->state; \
    (x)->lastSubstate = (x)->substate

/** Magic number that is searched by the #SYS_GeneralMacroBist(). */
#define SYS_BIST_GENERAL_MAGIC_NUMBER (42u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/** Symbolic names to check for multiple calls of #SYS_Trigger() */
typedef enum SYS_CHECK_MULTIPLE_CALLS {
    SYS_MULTIPLE_CALLS_NO,  /*!< no multiple calls, OK */
    SYS_MULTIPLE_CALLS_YES, /*!< multiple calls, not OK */
} SYS_CHECK_MULTIPLE_CALLS_e;

/** contains the state of the contactor state machine */
SYS_STATE_s sys_state = {
    .timer                  = 0,
    .triggerEntry           = 0,
    .stateRequest           = SYS_STATE_NO_REQUEST,
    .state                  = SYS_STATEMACH_UNINITIALIZED,
    .substate               = SYS_ENTRY,
    .lastState              = SYS_STATEMACH_UNINITIALIZED,
    .lastSubstate           = SYS_ENTRY,
    .illegalRequestsCounter = 0,
    .initializationTimeout  = 0,
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
 * @param   pSystemState state of the fake state machine
 * @return  #SYS_MULTIPLE_CALLS_YES if there were multiple calls,
 *          #SYS_MULTIPLE_CALLS_NO otherwise
 */
static SYS_CHECK_MULTIPLE_CALLS_e SYS_CheckMultipleCalls(SYS_STATE_s *pSystemState);

/**
 * @brief   Sets the next state, the next substate and the timer value
 *          of the state variable.
 * @param   pSystemState   state of the example state machine
 * @param   nextState      state to be transferred into
 * @param   nextSubstate   substate to be transferred into
 * @param   idleTime       wait time for the state machine
 */
static void SYS_SetState(
    SYS_STATE_s *pSystemState,
    SYS_FSM_STATES_e nextState,
    SYS_FSM_SUBSTATES_e nextSubstate,
    uint16_t idleTime);

/**
 * @brief   Sets the next substate and the timer value
 *          of the state variable.
 * @param   pSystemState   state of the example state machine
 * @param   nextSubstate   substate to be transferred into
 * @param   idleTime       wait time for the state machine
 */
static void SYS_SetSubstate(SYS_STATE_s *pSystemState, SYS_FSM_SUBSTATES_e nextSubstate, uint16_t idleTime);

/**
 * @brief   Defines the state transitions
 * @details This function contains the implementation of the state
 *          machine, i.e., the sequence of states and substates.
 *          It is called by the trigger function every time
 *          the state machine timer has a non-zero value.
 * @param   pSystemState state of the example state machine
 * @return  TODO
 */
static STD_RETURN_TYPE_e SYS_RunStateMachine(SYS_STATE_s *pSystemState);

static SYS_RETURN_TYPE_e SYS_CheckStateRequest(SYS_STATE_REQUEST_e stateRequest);
static SYS_STATE_REQUEST_e SYS_TransferStateRequest(void);

/*========== Static Function Implementations ================================*/
static SYS_CHECK_MULTIPLE_CALLS_e SYS_CheckMultipleCalls(SYS_STATE_s *pSystemState) {
    FAS_ASSERT(pSystemState != NULL_PTR);
    SYS_CHECK_MULTIPLE_CALLS_e multipleCalls = SYS_MULTIPLE_CALLS_NO;
    OS_EnterTaskCritical();
    if (pSystemState->triggerEntry == 0u) {
        pSystemState->triggerEntry++;
    } else {
        multipleCalls = SYS_MULTIPLE_CALLS_YES;
    }
    OS_ExitTaskCritical();
    return multipleCalls;
}

#pragma diag_push
#pragma diag_suppress 179
#pragma WEAK(SYS_SetState)
static void SYS_SetState(
    SYS_STATE_s *pSystemState,
    SYS_FSM_STATES_e nextState,
    SYS_FSM_SUBSTATES_e nextSubstate,
    uint16_t idleTime) {
    FAS_ASSERT(pSystemState != NULL_PTR);

    pSystemState->timer = idleTime;
}
#pragma diag_pop

#pragma diag_push
#pragma diag_suppress 179
#pragma WEAK(SYS_SetSubstate)
static void SYS_SetSubstate(SYS_STATE_s *pSystemState, SYS_FSM_SUBSTATES_e nextSubstate, uint16_t idleTime) {
    FAS_ASSERT(pSystemState != NULL_PTR);
}
#pragma diag_push

static STD_RETURN_TYPE_e SYS_RunStateMachine(SYS_STATE_s *pSystemState) {
    STD_RETURN_TYPE_e ranStateMachine = STD_OK;

    SYS_STATE_REQUEST_e stateRequest               = SYS_STATE_NO_REQUEST;
    SBC_STATEMACHINE_e sbcState                    = SBC_STATEMACHINE_UNDEFINED;
    STD_RETURN_TYPE_e balancingInitializationState = STD_OK;
    BAL_RETURN_TYPE_e balancingGlobalEnableState   = BAL_ERROR;
    STD_RETURN_TYPE_e bmsState                     = STD_NOT_OK;

    switch (pSystemState->state) {
        /****************************UNINITIALIZED***********************************/
        case SYS_STATEMACH_UNINITIALIZED:
            /* waiting for Initialization Request */
            stateRequest = SYS_TransferStateRequest();
            if (stateRequest == SYS_STATE_INIT_REQUEST) {
                SYS_SAVELASTSTATES(pSystemState);
                pSystemState->timer    = SYS_FSM_SHORT_TIME;
                pSystemState->state    = SYS_STATEMACH_INITIALIZATION;
                pSystemState->substate = SYS_ENTRY;
            } else if (stateRequest == SYS_STATE_NO_REQUEST) {
                /* no actual request pending */
            } else {
                pSystemState->illegalRequestsCounter++; /* illegal request pending */
            }
            break;
        /****************************INITIALIZATION**********************************/
        case SYS_STATEMACH_INITIALIZATION:

            SYS_SAVELASTSTATES(pSystemState);
            /* Initializations done here */
            FRAM_Read(FRAM_BLOCK_ID_DEEP_DISCHARGE_FLAG);
            for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
                if (fram_deepDischargeFlags.deepDischargeFlag[stringNumber] == true) {
                    DIAG_Handler(DIAG_ID_DEEP_DISCHARGE_DETECTED, DIAG_EVENT_NOT_OK, DIAG_STRING, stringNumber);
                }
            }

            pSystemState->timer    = SYS_FSM_SHORT_TIME;
            pSystemState->state    = SYS_STATEMACH_INITIALIZE_SBC;
            pSystemState->substate = SYS_ENTRY;
            break;

        /**************************** INITIALIZE SBC *************************************/
        case SYS_STATEMACH_INITIALIZE_SBC:
            SYS_SAVELASTSTATES(pSystemState);

            if (pSystemState->substate == SYS_ENTRY) {
                SBC_SetStateRequest(&sbc_stateMcuSupervisor, SBC_STATE_INIT_REQUEST);
                pSystemState->timer                 = SYS_FSM_SHORT_TIME;
                pSystemState->substate              = SYS_WAIT_INITIALIZATION_SBC;
                pSystemState->initializationTimeout = 0;
                break;
            } else if (pSystemState->substate == SYS_WAIT_INITIALIZATION_SBC) {
                sbcState = SBC_GetState(&sbc_stateMcuSupervisor);
                if (sbcState == SBC_STATEMACHINE_RUNNING) {
                    pSystemState->timer    = SYS_FSM_SHORT_TIME;
                    pSystemState->state    = SYS_STATEMACH_INITIALIZE_CAN;
                    pSystemState->substate = SYS_ENTRY;
                    break;
                } else {
                    if (pSystemState->initializationTimeout >
                        (SYS_STATEMACHINE_SBC_INIT_TIMEOUT_MS / SYS_TASK_CYCLE_CONTEXT_MS)) {
                        pSystemState->timer    = SYS_FSM_SHORT_TIME;
                        pSystemState->state    = SYS_STATEMACH_ERROR;
                        pSystemState->substate = SYS_SBC_INIT_ERROR;
                        break;
                    }
                    pSystemState->timer = SYS_FSM_SHORT_TIME;
                    pSystemState->initializationTimeout++;
                    break;
                }
            }
            break;

        /**************************** INITIALIZE CAN TRANSCEIVER ****************************/
        case SYS_STATEMACH_INITIALIZE_CAN:
            CAN_Initialize();
            pSystemState->timer    = SYS_FSM_SHORT_TIME;
            pSystemState->state    = SYS_STATEMACH_SYSTEM_BIST;
            pSystemState->substate = SYS_ENTRY;
            break;

        /**************************** EXECUTE STARTUP BIST **********************************/
        case SYS_STATEMACH_SYSTEM_BIST:
            SYS_SAVELASTSTATES(pSystemState);
            /* run BIST functions */
            SYS_GeneralMacroBist();
            DATA_ExecuteDataBIST();

            pSystemState->timer    = SYS_FSM_SHORT_TIME;
            pSystemState->state    = SYS_STATEMACH_INITIALIZED;
            pSystemState->substate = SYS_ENTRY;
            break;

        /****************************INITIALIZED*************************************/
        case SYS_STATEMACH_INITIALIZED:
            SYS_SAVELASTSTATES(pSystemState);
            /* Send CAN boot message directly on CAN */
            SYS_SendBootMessage();

            pSystemState->timer    = SYS_FSM_SHORT_TIME;
            pSystemState->state    = SYS_STATEMACH_INITIALIZE_INTERLOCK;
            pSystemState->substate = SYS_ENTRY;
            break;

        /****************************INITIALIZE INTERLOCK*************************************/
        case SYS_STATEMACH_INITIALIZE_INTERLOCK:
            SYS_SAVELASTSTATES(pSystemState);
            ILCK_SetStateRequest(ILCK_STATE_INITIALIZATION_REQUEST);
            pSystemState->timer                 = SYS_FSM_SHORT_TIME;
            pSystemState->state                 = SYS_STATEMACH_INITIALIZE_BALANCING;
            pSystemState->substate              = SYS_ENTRY;
            pSystemState->initializationTimeout = 0;
            break;

        /****************************INITIALIZE CONTACTORS*************************************/
        /* TODO: check if necessary and add */

        /****************************INITIALIZE BALANCING*************************************/
        case SYS_STATEMACH_INITIALIZE_BALANCING:
            SYS_SAVELASTSTATES(pSystemState);
            if (pSystemState->substate == SYS_ENTRY) {
                BAL_SetStateRequest(BAL_STATE_INIT_REQUEST);
                pSystemState->timer                 = SYS_FSM_SHORT_TIME;
                pSystemState->substate              = SYS_WAIT_INITIALIZATION_BAL;
                pSystemState->initializationTimeout = 0;
                break;
            } else if (pSystemState->substate == SYS_WAIT_INITIALIZATION_BAL) {
                balancingInitializationState = BAL_GetInitializationState();
                if (balancingInitializationState == STD_OK) {
                    pSystemState->timer    = SYS_FSM_SHORT_TIME;
                    pSystemState->substate = SYS_WAIT_INITIALIZATION_BAL_GLOBAL_ENABLE;
                    break;
                } else {
                    if (pSystemState->initializationTimeout >
                        (SYS_STATEMACH_BAL_INITIALIZATION_TIMEOUT_MS / SYS_TASK_CYCLE_CONTEXT_MS)) {
                        pSystemState->timer    = SYS_FSM_SHORT_TIME;
                        pSystemState->state    = SYS_STATEMACH_ERROR;
                        pSystemState->substate = SYS_BAL_INIT_ERROR;
                        break;
                    }
                    pSystemState->timer = SYS_FSM_SHORT_TIME;
                    pSystemState->initializationTimeout++;
                    break;
                }
            } else if (pSystemState->substate == SYS_WAIT_INITIALIZATION_BAL_GLOBAL_ENABLE) {
                if (BALANCING_DEFAULT_INACTIVE == true) {
                    balancingGlobalEnableState = BAL_SetStateRequest(BAL_STATE_GLOBAL_DISABLE_REQUEST);
                } else {
                    balancingGlobalEnableState = BAL_SetStateRequest(BAL_STATE_GLOBAL_ENABLE_REQUEST);
                }
                if (balancingGlobalEnableState == BAL_OK) {
                    pSystemState->timer    = SYS_FSM_SHORT_TIME;
                    pSystemState->state    = SYS_STATEMACH_FIRST_MEASUREMENT_CYCLE;
                    pSystemState->substate = SYS_ENTRY;
                    break;
                } else {
                    if (pSystemState->initializationTimeout >
                        (SYS_STATEMACH_INITIALIZATION_TIMEOUT_MS / SYS_TASK_CYCLE_CONTEXT_MS)) {
                        pSystemState->timer    = SYS_FSM_SHORT_TIME;
                        pSystemState->state    = SYS_STATEMACH_ERROR;
                        pSystemState->substate = SYS_BAL_INIT_ERROR;
                        break;
                    }
                    pSystemState->timer = SYS_FSM_SHORT_TIME;
                    pSystemState->initializationTimeout++;
                    break;
                }
            }
            break;

        /****************************START FIRST MEAS CYCLE**************************/
        case SYS_STATEMACH_FIRST_MEASUREMENT_CYCLE:
            SYS_SAVELASTSTATES(pSystemState);
            if (pSystemState->substate == SYS_ENTRY) {
                MEAS_StartMeasurement();
                pSystemState->initializationTimeout = 0;
                pSystemState->substate              = SYS_WAIT_FIRST_MEASUREMENT_CYCLE;
            } else if (pSystemState->substate == SYS_WAIT_FIRST_MEASUREMENT_CYCLE) {
                if (MEAS_IsFirstMeasurementCycleFinished() == true) {
                    /* allow initialization of algorithm module */
                    ALGO_UnlockInitialization();
                    /* MEAS_RequestOpenWireCheck(); */ /*TODO: check with strings */
                    pSystemState->timer = SYS_FSM_SHORT_TIME;
                    if (CURRENT_SENSOR_PRESENT == true) {
                        pSystemState->state = SYS_STATEMACH_CHECK_CURRENT_SENSOR_PRESENCE;
                    } else {
                        pSystemState->state = SYS_STATEMACH_INITIALIZE_MISC;
                    }
                    pSystemState->substate = SYS_ENTRY;
                    break;
                } else {
                    if (pSystemState->initializationTimeout >
                        (SYS_STATEMACH_INITIALIZATION_TIMEOUT_MS / SYS_TASK_CYCLE_CONTEXT_MS)) {
                        pSystemState->timer    = SYS_FSM_SHORT_TIME;
                        pSystemState->state    = SYS_STATEMACH_ERROR;
                        pSystemState->substate = SYS_MEAS_INIT_ERROR;
                        break;
                    } else {
                        pSystemState->timer = SYS_FSM_MEDIUM_TIME;
                        pSystemState->initializationTimeout++;
                        break;
                    }
                }
            }
            break;

        /****************************CHECK CURRENT SENSOR PRESENCE*************************************/
        case SYS_STATEMACH_CHECK_CURRENT_SENSOR_PRESENCE:
            SYS_SAVELASTSTATES(pSystemState);

            if (pSystemState->substate == SYS_ENTRY) {
                pSystemState->initializationTimeout = 0;
                CAN_EnablePeriodic(true);
#if CURRENT_SENSOR_ISABELLENHUETTE_TRIGGERED
                /* If triggered mode is used, CAN trigger message needs to
                     * be transmitted and current sensor response has to be
                     * received afterwards. This may take some time, therefore
                     * delay has to be increased.
                     */
                pSystemState->timer = SYS_FSM_LONG_TIME_MS;
#else  /* CURRENT_SENSOR_ISABELLENHUETTE_TRIGGERED */
                pSystemState->timer = SYS_FSM_LONG_TIME;
#endif /* CURRENT_SENSOR_ISABELLENHUETTE_TRIGGERED */
                pSystemState->substate = SYS_WAIT_CURRENT_SENSOR_PRESENCE;
            } else if (pSystemState->substate == SYS_WAIT_CURRENT_SENSOR_PRESENCE) {
                bool allSensorsPresent = true;
                for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
                    if (CAN_IsCurrentSensorPresent(stringNumber) == true) {
                        if (CAN_IsCurrentSensorCcPresent(stringNumber) == true) {
                            SE_SocInit(true, stringNumber);
                        } else {
                            SE_SocInit(false, stringNumber);
                        }
                        if (CAN_IsCurrentSensorEcPresent(stringNumber) == true) {
                            SE_SoeInit(true, stringNumber);
                        } else {
                            SE_SoeInit(false, stringNumber);
                        }
                    } else {
                        allSensorsPresent = false;
                    }
                }

                if (allSensorsPresent == true) {
                    SOF_Init();

                    pSystemState->timer    = SYS_FSM_SHORT_TIME;
                    pSystemState->state    = SYS_STATEMACH_INITIALIZE_MISC;
                    pSystemState->substate = SYS_ENTRY;
                    break;
                } else {
                    if (pSystemState->initializationTimeout >
                        (SYS_STATEMACH_INITIALIZATION_TIMEOUT_MS / SYS_TASK_CYCLE_CONTEXT_MS)) {
                        pSystemState->timer    = SYS_FSM_SHORT_TIME;
                        pSystemState->state    = SYS_STATEMACH_ERROR;
                        pSystemState->substate = SYS_CURRENT_SENSOR_PRESENCE_ERROR;
                        break;
                    } else {
                        pSystemState->timer = SYS_FSM_MEDIUM_TIME;
                        pSystemState->initializationTimeout++;
                        break;
                    }
                }
            }
            break;

        /****************************INITIALIZED_MISC*************************************/
        case SYS_STATEMACH_INITIALIZE_MISC:
            SYS_SAVELASTSTATES(pSystemState);

            if (CURRENT_SENSOR_PRESENT == false) {
                CAN_EnablePeriodic(true);
                for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
                    SE_SocInit(false, stringNumber);
                    SE_SoeInit(false, stringNumber);
                }
            }

            pSystemState->timer    = SYS_FSM_MEDIUM_TIME;
            pSystemState->state    = SYS_STATEMACH_INITIALIZE_BMS;
            pSystemState->substate = SYS_ENTRY;
            break;

        /****************************INITIALIZE BMS*************************************/
        case SYS_STATEMACH_INITIALIZE_BMS:
            SYS_SAVELASTSTATES(pSystemState);

            if (pSystemState->substate == SYS_ENTRY) {
                BMS_SetStateRequest(BMS_STATE_INIT_REQUEST);
                pSystemState->timer                 = SYS_FSM_SHORT_TIME;
                pSystemState->substate              = SYS_WAIT_INITIALIZATION_BMS;
                pSystemState->initializationTimeout = 0;
                break;
            } else if (pSystemState->substate == SYS_WAIT_INITIALIZATION_BMS) {
                bmsState = BMS_GetInitializationState();
                if (bmsState == STD_OK) {
                    pSystemState->timer    = SYS_FSM_SHORT_TIME;
                    pSystemState->state    = SYS_STATEMACH_RUNNING;
                    pSystemState->substate = SYS_ENTRY;
                    break;
                } else {
                    if (pSystemState->initializationTimeout >
                        (SYS_STATEMACH_INITIALIZATION_TIMEOUT_MS / SYS_TASK_CYCLE_CONTEXT_MS)) {
                        pSystemState->timer    = SYS_FSM_SHORT_TIME;
                        pSystemState->state    = SYS_STATEMACH_ERROR;
                        pSystemState->substate = SYS_BMS_INIT_ERROR;
                        break;
                    }
                    pSystemState->timer = SYS_FSM_SHORT_TIME;
                    pSystemState->initializationTimeout++;
                    break;
                }
            }
            break;

        /****************************RUNNING*************************************/
        case SYS_STATEMACH_RUNNING:
            SYS_SAVELASTSTATES(pSystemState);
            pSystemState->timer = SYS_FSM_LONG_TIME;
            break;

        /****************************ERROR*************************************/
        case SYS_STATEMACH_ERROR:
            SYS_SAVELASTSTATES(pSystemState);
            pSystemState->timer = SYS_FSM_LONG_TIME;
            break;
        /***************************DEFAULT CASE*************************************/
        default:
            /* invalid state */
            FAS_ASSERT(FAS_TRAP);
            break;
    }
    return ranStateMachine;
}

/**
 * @brief   transfers the current state request to the state machine.
 *
 * This function takes the current state request from #sys_state and transfers it to the state machine.
 * It resets the value from #sys_state to #SYS_STATE_NO_REQUEST
 *
 * @return  retVal          current state request, taken from #SYS_STATE_REQUEST_e
 *
 */
static SYS_STATE_REQUEST_e SYS_TransferStateRequest(void) {
    SYS_STATE_REQUEST_e retval = SYS_STATE_NO_REQUEST;

    OS_EnterTaskCritical();
    retval                 = sys_state.stateRequest;
    sys_state.stateRequest = SYS_STATE_NO_REQUEST;
    OS_ExitTaskCritical();

    return (retval);
}

SYS_RETURN_TYPE_e SYS_SetStateRequest(SYS_STATE_REQUEST_e stateRequest) {
    SYS_RETURN_TYPE_e retVal = SYS_ILLEGAL_REQUEST;

    OS_EnterTaskCritical();
    retVal = SYS_CheckStateRequest(stateRequest);

    if (retVal == SYS_OK) {
        sys_state.stateRequest = stateRequest;
    }
    OS_ExitTaskCritical();

    return (retVal);
}

/**
 * @brief   checks the state requests that are made.
 *
 * This function checks the validity of the state requests.
 * The results of the checked is returned immediately.
 *
 * @param   stateRequest    state request to be checked
 *
 * @return              result of the state request that was made, taken from SYS_RETURN_TYPE_e
 */
static SYS_RETURN_TYPE_e SYS_CheckStateRequest(SYS_STATE_REQUEST_e stateRequest) {
    SYS_RETURN_TYPE_e retval = SYS_ILLEGAL_REQUEST;
    if (stateRequest == SYS_STATE_ERROR_REQUEST) {
        retval = SYS_OK;
    } else {
        if (sys_state.stateRequest == SYS_STATE_NO_REQUEST) {
            /* init only allowed from the uninitialized state */
            if (stateRequest == SYS_STATE_INIT_REQUEST) {
                if (sys_state.state == SYS_STATEMACH_UNINITIALIZED) {
                    retval = SYS_OK;
                } else {
                    retval = SYS_ALREADY_INITIALIZED;
                }
            } else {
                retval = SYS_ILLEGAL_REQUEST;
            }
        } else {
            retval = SYS_REQUEST_PENDING;
        }
    }
    return retval;
}

/*========== Extern Function Implementations ================================*/

extern void SYS_GeneralMacroBist(void) {
    const uint8_t dummy[REPEAT_MAXIMUM_REPETITIONS] = {
        REPEAT_U(SYS_BIST_GENERAL_MAGIC_NUMBER, STRIP(REPEAT_MAXIMUM_REPETITIONS))};
    for (uint8_t i = 0u; i < REPEAT_MAXIMUM_REPETITIONS; i++) {
        FAS_ASSERT(SYS_BIST_GENERAL_MAGIC_NUMBER == dummy[i]);
    }
}

extern STD_RETURN_TYPE_e SYS_Trigger(SYS_STATE_s *pSystemState) {
    FAS_ASSERT(pSystemState != NULL_PTR);
    bool earlyExit                = false;
    STD_RETURN_TYPE_e returnValue = STD_OK;

    /* Check multiple calls of function */
    if (SYS_MULTIPLE_CALLS_YES == SYS_CheckMultipleCalls(pSystemState)) {
        returnValue = STD_NOT_OK;
        earlyExit   = true;
    }

    if (earlyExit == false) {
        if (pSystemState->timer > 0u) {
            if ((--pSystemState->timer) > 0u) {
                pSystemState->triggerEntry--;
                returnValue = STD_OK;
                earlyExit   = true;
            }
        }
    }

    if (earlyExit == false) {
        SYS_RunStateMachine(pSystemState);
        pSystemState->triggerEntry--;
    }
    return returnValue;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
