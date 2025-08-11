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
 * @file    sys.c
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup ENGINE
 * @prefix  SYS
 *
 * @brief   Sys driver implementation
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "general.h"

#include "sys.h"

#include "algorithm.h"
#include "bal.h"
#include "bms.h"
#include "can.h"
#include "contactor.h"
#include "diag.h"
#include "fram.h"
#include "fstd_types.h"
#include "imd.h"
#include "interlock.h"
#include "meas.h"
#include "os.h"
#include "rtc.h"
#include "sbc.h"
#include "sof_trapezoid.h"
#include "state_estimation.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** Magic number that is searched by the #SYS_GeneralMacroBist(). */
#define SYS_BIST_GENERAL_MAGIC_NUMBER (42u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/** Symbolic names to check for multiple calls of #SYS_Trigger() */
typedef enum {
    SYS_MULTIPLE_CALLS_NO,  /*!< no multiple calls, OK */
    SYS_MULTIPLE_CALLS_YES, /*!< multiple calls, not OK */
} SYS_CHECK_MULTIPLE_CALLS_e;

/** contains the current state of the SYS machine */
SYS_STATE_s sys_state = {
    .timer                  = 0,
    .stateRequest           = SYS_STATE_NO_REQUEST,
    .nextState              = SYS_FSM_STATE_HAS_NEVER_RUN,
    .nextSubstate           = SYS_FSM_SUBSTATE_DUMMY,
    .currentState           = SYS_FSM_STATE_UNINITIALIZED,
    .currentSubstate        = SYS_FSM_SUBSTATE_DUMMY,
    .previousState          = SYS_FSM_STATE_HAS_NEVER_RUN,
    .previousSubstate       = SYS_FSM_SUBSTATE_DUMMY,
    .illegalRequestsCounter = 0,
    .initializationTimeout  = 0,
    .triggerEntry           = 0,
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
 * @param   pSystemState state of the state machine
 * @return  #SYS_MULTIPLE_CALLS_YES if there were multiple calls,
 *          #SYS_MULTIPLE_CALLS_NO otherwise
 */
static SYS_CHECK_MULTIPLE_CALLS_e SYS_CheckMultipleCalls(SYS_STATE_s *pSystemState);

/**
 * @brief   Sets the next state, the next substate and the timer value
 *          of the state variable.
 * @param   pSystemState      state of the system state machine
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
 * @param   pSystemState      state of the system state machine
 * @param   nextSubstate   substate to be transferred into
 * @param   idleTime       wait time for the state machine
 */
static void SYS_SetSubstate(SYS_STATE_s *pSystemState, SYS_FSM_SUBSTATES_e nextSubstate, uint16_t idleTime);

/**
 * @brief   Processes the initialization state
 * @param   pSystemState state of the SYS state machine
 * @return  the next state of the SYS state machine
 */
static SYS_FSM_STATES_e SYS_ProcessInitializationState(SYS_STATE_s *pSystemState);

/**
 * @brief   Processes the pre running state
 * @param   pSystemState state of the SYS state machine
 * @return  the next state of the SYS state machine
 */
static SYS_FSM_STATES_e SYS_ProcessPreRunningState(SYS_STATE_s *pSystemState);

/**
 * @brief   Processes the running state
 * @param   pSystemState state of the SYS state machine
 * @return  the next state of the SYS state machine
 */
static SYS_FSM_STATES_e SYS_ProcessRunningState(const SYS_STATE_s *pSystemState);

/**
 * @brief   Processes the error state
 * @param   pSystemState state of the SYS state machine
 * @return  the next state of the SYS state machine
 */
static SYS_FSM_STATES_e SYS_ProcessErrorState(const SYS_STATE_s *pSystemState);

/**
 * @brief   Defines the state transitions
 * @details This function contains the implementation of the state
 *          machine, i.e., the sequence of states and substates.
 *          It is called by the trigger function every time
 *          the state machine timer has a non-zero value.
 * @param   pSystemState state of the system state machine
 * @return  TODO
 */
static STD_RETURN_TYPE_e SYS_RunStateMachine(SYS_STATE_s *pSystemState);

/**
 * @brief   Checks the state requests that are made.
 * @details Checks the validity of the state requests.
 *          The results of the checked is returned immediately.
 * @param   stateRequest    state request to be checked
 * @return  Validity of the state requests.
 */
static SYS_RETURN_TYPE_e SYS_CheckStateRequest(SYS_STATE_REQUEST_e stateRequest);

/**
 * @brief   Transfers the current state request to the state machine.
 * @details Transfers the requested state to the caller and resets the
 *          requested member of #sys_state.
 * @return  Requested state
 */
static SYS_STATE_REQUEST_e SYS_TransferStateRequest(void);

/**
 * @brief   Built-in self-test for the macros in general.h
 * @details Internal built-in self-test for the macros in the file general.h
 */
static void SYS_GeneralMacroBist(void);

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

static void SYS_SetSubstate(SYS_STATE_s *pSystemState, SYS_FSM_SUBSTATES_e nextSubstate, uint16_t idleTime) {
    FAS_ASSERT(pSystemState != NULL_PTR);
    /* AXIVION Routine Generic-MissingParameterAssert: idleTime: parameter accepts whole range */
    pSystemState->timer            = idleTime;
    pSystemState->previousSubstate = pSystemState->currentSubstate;
    pSystemState->currentSubstate  = nextSubstate;
    pSystemState->nextSubstate = SYS_FSM_SUBSTATE_DUMMY; /* substate has been set, now reset value for nextSubstate */
}

static void SYS_SetState(
    SYS_STATE_s *pSystemState,
    SYS_FSM_STATES_e nextState,
    SYS_FSM_SUBSTATES_e nextSubstate,
    uint16_t idleTime) {
    FAS_ASSERT(pSystemState != NULL_PTR);
    /* AXIVION Routine Generic-MissingParameterAssert: idleTime: parameter accepts whole range */
    bool earlyExit = false;

    pSystemState->timer            = idleTime;
    pSystemState->previousState    = pSystemState->currentState;
    pSystemState->previousSubstate = pSystemState->currentSubstate;

    if ((pSystemState->currentState == nextState) && (pSystemState->currentSubstate == nextSubstate)) {
        /* Next state and next substate equal to current state and substate: nothing to do */
        pSystemState->nextState    = SYS_FSM_STATE_DUMMY;    /* no state transition required -> reset */
        pSystemState->nextSubstate = SYS_FSM_SUBSTATE_DUMMY; /* no substate transition required -> reset */
        earlyExit                  = true;
    }

    if (earlyExit == false) {
        if (pSystemState->currentState != nextState) {
            /* distinguish between just a state transfer to the error state and a normal state transfer */
            if (nextState == SYS_FSM_STATE_ERROR) {
                /* Error state gets treated differently since we dont need to enter it through the Entry sub state */
                pSystemState->previousState    = pSystemState->currentState;
                pSystemState->currentState     = nextState;
                pSystemState->previousSubstate = pSystemState->currentSubstate;
                pSystemState->currentSubstate  = nextSubstate;
            } else {
                /* Next state is different than the current one: switch to it and set substate to entry value */
                pSystemState->previousState    = pSystemState->currentState;
                pSystemState->currentState     = nextState;
                pSystemState->previousSubstate = pSystemState->currentSubstate;
                pSystemState->currentSubstate = SYS_FSM_SUBSTATE_ENTRY; /* entry state after a top level state change */
                pSystemState->nextState       = SYS_FSM_STATE_DUMMY;    /* no state transition required -> reset */
                pSystemState->nextSubstate    = SYS_FSM_SUBSTATE_DUMMY; /* no substate transition required -> reset */
            }
        } else if (pSystemState->currentSubstate != nextSubstate) {
            /* Only the next substate is different, switch to it */
            SYS_SetSubstate(pSystemState, nextSubstate, idleTime);
        } else {
            ;
        }
    }
}

static SYS_FSM_STATES_e SYS_ProcessInitializationState(SYS_STATE_s *pSystemState) {
    FAS_ASSERT(pSystemState != NULL_PTR);
    SYS_FSM_STATES_e nextState  = SYS_FSM_STATE_INITIALIZATION; /* default behavior: stay in state */
    SBC_STATEMACHINE_e sbcState = SBC_STATEMACHINE_UNDEFINED;

    switch (pSystemState->currentSubstate) {
        /**************************** ENTRY STATE ****************************************/
        case SYS_FSM_SUBSTATE_ENTRY:
            SYS_SetSubstate(pSystemState, SYS_FSM_CHECK_DEEP_DISCHARGE, SYS_FSM_SHORT_TIME);
            break;

        /**************************** READ FRAM ******************************************/
        case SYS_FSM_CHECK_DEEP_DISCHARGE:
            (void)FRAM_ReadData(FRAM_BLOCK_ID_DEEP_DISCHARGE_FLAG);
            for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
                if (fram_deepDischargeFlags.deepDischargeFlag[s] == true) {
                    (void)DIAG_Handler(DIAG_ID_DEEP_DISCHARGE_DETECTED, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                }
            }
            SYS_SetSubstate(pSystemState, SYS_FSM_SUBSTATE_START_INITIALIZATION_SBC, SYS_FSM_SHORT_TIME);
            break;

        /**************************** INITIALIZE SBC *************************************/
        case SYS_FSM_SUBSTATE_START_INITIALIZATION_SBC:
            if (SBC_SetStateRequest(&sbc_stateMcuSupervisor, SBC_STATE_INIT_REQUEST) == SBC_OK) {
                pSystemState->initializationTimeout = 0;
                SYS_SetSubstate(pSystemState, SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_SBC, SYS_FSM_SHORT_TIME);
            }
            break;

        case SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_SBC:
            sbcState = SBC_GetState(&sbc_stateMcuSupervisor);
            if (sbcState == SBC_STATEMACHINE_RUNNING) {
                SYS_SetSubstate(pSystemState, SYS_FSM_SUBSTATE_INITIALIZATION_CAN, SYS_FSM_SHORT_TIME);
            } else {
                if (pSystemState->initializationTimeout >
                    (SYS_STATE_MACHINE_SBC_INIT_TIMEOUT_MS / SYS_TASK_CYCLE_CONTEXT_MS)) {
                    pSystemState->nextSubstate = SYS_FSM_SUBSTATE_SBC_INITIALIZATION_ERROR;
                    nextState                  = SYS_FSM_STATE_ERROR;
                } else {
                    pSystemState->timer = SYS_FSM_SHORT_TIME;
                    pSystemState->initializationTimeout++;
                }
            }
            break;

        /**************************** INITIALIZE CAN TRANSCEIVER ****************************/
        case SYS_FSM_SUBSTATE_INITIALIZATION_CAN:
            CAN_Initialize();
            SYS_SetSubstate(pSystemState, SYS_FSM_SUBSTATE_INITIALIZATION_RTC, SYS_FSM_SHORT_TIME);
            break;

        /**************************** INITIALIZE RTC ****************************************/
        case SYS_FSM_SUBSTATE_INITIALIZATION_RTC:
            if (RTC_IsRtcModuleInitialized() == true) {
                SYS_SetSubstate(pSystemState, SYS_FSM_SUBSTATE_START_UP_BIST, SYS_FSM_SHORT_TIME);
            }
            break;

        /**************************** EXECUTE STARTUP BIST **********************************/
        case SYS_FSM_SUBSTATE_START_UP_BIST:
            /* run BIST functions: There is no need in this substate to
             * transfer to the error state, as all functions used here are
             * asserting to an infinite loop. */
            SYS_GeneralMacroBist();
            DATA_ExecuteDataBist();
            SYS_SetSubstate(pSystemState, SYS_FSM_SUBSTATE_SEND_BOOT_MESSAGE, SYS_FSM_SHORT_TIME);
            break;

        /**************************** SEND BOOT MESSAGE *************************************/
        case SYS_FSM_SUBSTATE_SEND_BOOT_MESSAGE:
            /* Send CAN boot message directly on CAN */
            SYS_SendBootMessage();
            nextState = SYS_FSM_STATE_PRE_RUNNING;
            break;

        default:                  /* LCOV_EXCL_LINE */
            FAS_ASSERT(FAS_TRAP); /* LCOV_EXCL_LINE */
            break;                /* LCOV_EXCL_LINE */
    }
    return nextState;
}

static SYS_FSM_STATES_e SYS_ProcessPreRunningState(SYS_STATE_s *pSystemState) {
    FAS_ASSERT(pSystemState != NULL_PTR);
    SYS_FSM_STATES_e nextState                     = SYS_FSM_STATE_PRE_RUNNING; /* default behavior: stay in state */
    bool allSensorsPresent                         = true;
    uint16_t nextWaitTime                          = SYS_FSM_SHORT_TIME;
    STD_RETURN_TYPE_e balancingInitializationState = STD_OK;
    BAL_RETURN_TYPE_e balancingGlobalEnableState   = BAL_ERROR;
    STD_RETURN_TYPE_e bmsState                     = STD_NOT_OK;

    switch (pSystemState->currentSubstate) {
        /**************************** ENTRY STATE ****************************************/
        case SYS_FSM_SUBSTATE_ENTRY:
            SYS_SetSubstate(pSystemState, SYS_FSM_SUBSTATE_INITIALIZE_INTERLOCK, SYS_FSM_SHORT_TIME);
            break;

        /****************************INITIALIZE INTERLOCK*************************************/
        case SYS_FSM_SUBSTATE_INITIALIZE_INTERLOCK:
            (void)ILCK_SetStateRequest(ILCK_STATE_INITIALIZATION_REQUEST);
            pSystemState->initializationTimeout = 0;
            SYS_SetSubstate(pSystemState, SYS_FSM_SUBSTATE_START_INITIALIZATION_BAL, SYS_FSM_SHORT_TIME);
            break;

        /****************************INITIALIZE BALANCING*************************************/
        case SYS_FSM_SUBSTATE_START_INITIALIZATION_BAL:
            (void)BAL_SetStateRequest(BAL_STATE_INIT_REQUEST);
            pSystemState->initializationTimeout = 0;
            SYS_SetSubstate(pSystemState, SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BAL, SYS_FSM_SHORT_TIME);
            break;

        case SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BAL:
            balancingInitializationState = BAL_GetInitializationState();
            if (balancingInitializationState == STD_OK) {
                SYS_SetSubstate(
                    pSystemState, SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BAL_GLOBAL_ENABLE, SYS_FSM_SHORT_TIME);
            } else {
                if (pSystemState->initializationTimeout >
                    (SYS_STATE_MACHINE_BAL_INITIALIZATION_TIMEOUT_MS / SYS_TASK_CYCLE_CONTEXT_MS)) {
                    pSystemState->nextSubstate = SYS_FSM_SUBSTATE_BAL_INITIALIZATION_ERROR;
                    nextState                  = SYS_FSM_STATE_ERROR;
                } else {
                    pSystemState->timer = SYS_FSM_SHORT_TIME;
                    pSystemState->initializationTimeout++;
                }
            }
            break;

        case SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BAL_GLOBAL_ENABLE:
#if (BS_BALANCING_DEFAULT_INACTIVE == true)
            balancingGlobalEnableState = BAL_SetStateRequest(BAL_STATE_GLOBAL_DISABLE_REQUEST);
#else  /* BS_BALANCING_DEFAULT_INACTIVE is true */
            balancingGlobalEnableState = BAL_SetStateRequest(BAL_STATE_GLOBAL_ENABLE_REQUEST);
#endif /* BS_BALANCING_DEFAULT_INACTIVE is true */
            if (balancingGlobalEnableState == BAL_OK) {
                SYS_SetSubstate(pSystemState, SYS_FSM_SUBSTATE_START_FIRST_MEASUREMENT_CYCLE, SYS_FSM_SHORT_TIME);
            } else {
                if (pSystemState->initializationTimeout >
                    (SYS_STATE_MACHINE_INITIALIZATION_TIMEOUT_MS / SYS_TASK_CYCLE_CONTEXT_MS)) {
                    pSystemState->nextSubstate = SYS_FSM_SUBSTATE_BAL_GLOBAL_INITIALIZATION_ERROR;
                    nextState                  = SYS_FSM_STATE_ERROR;
                } else {
                    pSystemState->timer = SYS_FSM_SHORT_TIME;
                    pSystemState->initializationTimeout++;
                }
            }
            break;

        /****************************START FIRST MEAS CYCLE**************************/
        case SYS_FSM_SUBSTATE_START_FIRST_MEASUREMENT_CYCLE:
            (void)MEAS_StartMeasurement();
            pSystemState->initializationTimeout = 0;
            SYS_SetSubstate(pSystemState, SYS_FSM_SUBSTATE_WAIT_FIRST_MEASUREMENT_CYCLE, SYS_FSM_SHORT_TIME);
            break;

        case SYS_FSM_SUBSTATE_WAIT_FIRST_MEASUREMENT_CYCLE:
            if (MEAS_IsFirstMeasurementCycleFinished() == true) {
                /* allow initialization of algorithm module */
                ALGO_UnlockInitialization();
                /* MEAS_RequestOpenWireCheck(); */ /*TODO: check with strings */
#if (BS_CURRENT_SENSOR_PRESENT == true)
                SYS_SetSubstate(pSystemState, SYS_FSM_SUBSTATE_START_CURRENT_SENSOR_PRESENCE_CHECK, SYS_FSM_SHORT_TIME);
#else  /* BS_CURRENT_SENSOR_PRESENT is true */
                SYS_SetSubstate(pSystemState, SYS_FSM_SUBSTATE_INITIALIZATION_MISC, SYS_FSM_SHORT_TIME);
#endif /* BS_CURRENT_SENSOR_PRESENT is true */
            } else {
                if (pSystemState->initializationTimeout >
                    (SYS_STATE_MACHINE_INITIALIZATION_TIMEOUT_MS / SYS_TASK_CYCLE_CONTEXT_MS)) {
                    pSystemState->nextSubstate = SYS_FSM_SUBSTATE_FIRST_MEAS_INITIALIZATION_ERROR;
                    nextState                  = SYS_FSM_STATE_ERROR;
                } else {
                    pSystemState->timer = SYS_FSM_MEDIUM_TIME;
                    pSystemState->initializationTimeout++;
                }
            }
            break;

        /****************************CHECK CURRENT SENSOR PRESENCE*************************************/
        case SYS_FSM_SUBSTATE_START_CURRENT_SENSOR_PRESENCE_CHECK:
            pSystemState->initializationTimeout = 0;
            CAN_EnablePeriodic(true);
#if defined(CURRENT_SENSOR_ISABELLENHUETTE_TRIGGERED)
            /* If triggered mode is used, CAN trigger message needs to
             * be transmitted and current sensor response has to be
             * received afterwards. This may take some time, therefore
             * delay has to be increased.
             */
            nextWaitTime = SYS_FSM_LONG_TIME_MS;
#else  /* defined(CURRENT_SENSOR_ISABELLENHUETTE_TRIGGERED) */
            nextWaitTime = SYS_FSM_LONG_TIME;
#endif /* defined(CURRENT_SENSOR_ISABELLENHUETTE_TRIGGERED) */
            SYS_SetSubstate(pSystemState, SYS_FSM_SUBSTATE_WAIT_CURRENT_SENSOR_PRESENCE_CHECK, nextWaitTime);
            break;

        case SYS_FSM_SUBSTATE_WAIT_CURRENT_SENSOR_PRESENCE_CHECK:
            for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
                if (CAN_IsCurrentSensorPresent(s) == true) {
                    if (CAN_IsCurrentSensorCcPresent(s) == true) {
                        SE_InitializeSoc(true, s);
                    } else {
                        SE_InitializeSoc(false, s);
                    }
                    if (CAN_IsCurrentSensorEcPresent(s) == true) {
                        SE_InitializeSoe(true, s);
                    } else {
                        SE_InitializeSoe(false, s);
                    }
                    SE_InitializeSoh(s);
                } else {
                    allSensorsPresent = false;
                }
            }
            if (allSensorsPresent == true) {
                SOF_Init();
                SYS_SetSubstate(pSystemState, SYS_FSM_SUBSTATE_INITIALIZATION_MISC, SYS_FSM_SHORT_TIME);
            } else {
                if (pSystemState->initializationTimeout >
                    (SYS_STATE_MACHINE_INITIALIZATION_TIMEOUT_MS / SYS_TASK_CYCLE_CONTEXT_MS)) {
                    pSystemState->nextSubstate = SYS_FSM_SUBSTATE_CURRENT_SENSOR_PRESENCE_ERROR;
                    nextState                  = SYS_FSM_STATE_ERROR;
                } else {
                    pSystemState->timer = SYS_FSM_MEDIUM_TIME;
                    pSystemState->initializationTimeout++;
                }
            }
            break;

        /****************************INITIALIZED_MISC*************************************/
        case SYS_FSM_SUBSTATE_INITIALIZATION_MISC:
#if (BS_CURRENT_SENSOR_PRESENT == false)
            CAN_EnablePeriodic(true);
            for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
                SE_InitializeSoc(false, s);
                SE_InitializeSoe(false, s);
                SE_InitializeSoh(s);
            }
#endif /* BS_CURRENT_SENSOR_PRESENT is false */

            pSystemState->initializationTimeout = 0u;
            SYS_SetSubstate(pSystemState, SYS_FSM_SUBSTATE_INITIALIZATION_IMD, SYS_FSM_SHORT_TIME);
            break;

        /****************************INITIALIZE CONTACTORS*************************************/
        /* TODO: check if necessary and add */

        /****************************INITIALIZED_IMD*************************************/
        case SYS_FSM_SUBSTATE_INITIALIZATION_IMD:
            if (IMD_REQUEST_OK == IMD_RequestInitialization()) {
                /* Request inquired successfully */
                SYS_SetSubstate(pSystemState, SYS_FSM_SUBSTATE_START_INITIALIZATION_BMS, SYS_FSM_MEDIUM_TIME);
                pSystemState->initializationTimeout = 0u;
            } else {
                /* Request declined -> retry max. 3 times */
                pSystemState->initializationTimeout++;
                pSystemState->timer = SYS_FSM_SHORT_TIME;
                if (pSystemState->initializationTimeout >= SYS_STATE_MACHINE_INITIALIZATION_REQUEST_RETRY_COUNTER) {
                    pSystemState->nextSubstate = SYS_FSM_SUBSTATE_IMD_INITIALIZATION_ERROR;
                    nextState                  = SYS_FSM_STATE_ERROR;
                }
            }
            break;

        /****************************INITIALIZE BMS*************************************/
        case SYS_FSM_SUBSTATE_START_INITIALIZATION_BMS:
            (void)BMS_SetStateRequest(BMS_STATE_INIT_REQUEST);
            pSystemState->initializationTimeout = 0;
            SYS_SetSubstate(pSystemState, SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BMS, SYS_FSM_SHORT_TIME);
            break;

        case SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BMS:
            bmsState = BMS_GetInitializationState();
            if (bmsState == STD_OK) {
                nextState = SYS_FSM_STATE_RUNNING;
            } else {
                if (pSystemState->initializationTimeout >
                    (SYS_STATE_MACHINE_INITIALIZATION_TIMEOUT_MS / SYS_TASK_CYCLE_CONTEXT_MS)) {
                    pSystemState->nextSubstate = SYS_FSM_SUBSTATE_BMS_INITIALIZATION_ERROR;
                    nextState                  = SYS_FSM_STATE_ERROR;
                } else {
                    pSystemState->timer = SYS_FSM_SHORT_TIME;
                    pSystemState->initializationTimeout++;
                }
            }
            break;

        default:                  /* LCOV_EXCL_LINE */
            FAS_ASSERT(FAS_TRAP); /* LCOV_EXCL_LINE */
            break;                /* LCOV_EXCL_LINE */
    }
    return nextState;
}

static SYS_FSM_STATES_e SYS_ProcessRunningState(const SYS_STATE_s *pSystemState) {
    FAS_ASSERT(pSystemState != NULL_PTR);
    SYS_FSM_STATES_e nextState = SYS_FSM_STATE_RUNNING; /* default behavior: stay in state */
    return nextState;
}

static SYS_FSM_STATES_e SYS_ProcessErrorState(const SYS_STATE_s *pSystemState) {
    FAS_ASSERT(pSystemState != NULL_PTR);
    SYS_FSM_STATES_e nextState = SYS_FSM_STATE_ERROR; /* default behavior: stay in state */
    return nextState;
}

static STD_RETURN_TYPE_e SYS_RunStateMachine(SYS_STATE_s *pSystemState) {
    FAS_ASSERT(pSystemState != NULL_PTR);
    STD_RETURN_TYPE_e ranStateMachine = STD_OK;
    SYS_FSM_STATES_e nextState        = SYS_FSM_STATE_DUMMY;

    SYS_STATE_REQUEST_e stateRequest = SYS_STATE_NO_REQUEST;

    switch (pSystemState->currentState) {
        /****************************UNINITIALIZED***********************************/
        case SYS_FSM_STATE_UNINITIALIZED:
            /* waiting for Initialization Request */
            stateRequest = SYS_TransferStateRequest();
            if (stateRequest == SYS_STATE_INITIALIZATION_REQUEST) {
                SYS_SetState(pSystemState, SYS_FSM_STATE_INITIALIZATION, SYS_FSM_SUBSTATE_ENTRY, SYS_FSM_SHORT_TIME);
            } else if (stateRequest == SYS_STATE_NO_REQUEST) {
                /* no actual request pending */
            } else {
                pSystemState->illegalRequestsCounter++; /* illegal request pending */
            }
            break;

        /****************************INITIALIZATION**********************************/
        case SYS_FSM_STATE_INITIALIZATION:
            nextState = SYS_ProcessInitializationState(pSystemState);
            if (nextState == SYS_FSM_STATE_INITIALIZATION) {
                /* staying in state, processed by state function */
            } else if (nextState == SYS_FSM_STATE_ERROR) {
                SYS_SetState(pSystemState, SYS_FSM_STATE_ERROR, pSystemState->nextSubstate, SYS_FSM_SHORT_TIME);
            } else if (nextState == SYS_FSM_STATE_PRE_RUNNING) {
                SYS_SetState(pSystemState, SYS_FSM_STATE_PRE_RUNNING, SYS_FSM_SUBSTATE_ENTRY, SYS_FSM_SHORT_TIME);
            } else {
                FAS_ASSERT(FAS_TRAP); /* Something went wrong */
            }
            break;

        /****************************PRE RUNNING ************************************/
        case SYS_FSM_STATE_PRE_RUNNING:
            nextState = SYS_ProcessPreRunningState(pSystemState);
            if (nextState == SYS_FSM_STATE_PRE_RUNNING) {
                /* staying in state, processed by state function */
            } else if (nextState == SYS_FSM_STATE_ERROR) {
                SYS_SetState(pSystemState, SYS_FSM_STATE_ERROR, pSystemState->nextSubstate, SYS_FSM_SHORT_TIME);
            } else if (nextState == SYS_FSM_STATE_RUNNING) {
                SYS_SetState(pSystemState, SYS_FSM_STATE_RUNNING, SYS_FSM_SUBSTATE_ENTRY, SYS_FSM_SHORT_TIME);
            } else {
                FAS_ASSERT(FAS_TRAP); /* Something went wrong */
            }
            break;

        /****************************RUNNING*****************************************/
        case SYS_FSM_STATE_RUNNING:
            nextState = SYS_ProcessRunningState(pSystemState);
            if (nextState == SYS_FSM_STATE_RUNNING) {
                /* staying in state, processed by state function */
                pSystemState->timer = SYS_FSM_LONG_TIME;
            } else if (nextState == SYS_FSM_STATE_ERROR) {
                SYS_SetState(pSystemState, SYS_FSM_STATE_ERROR, pSystemState->nextSubstate, SYS_FSM_SHORT_TIME);
            } else {
                FAS_ASSERT(FAS_TRAP); /* Something went wrong */
            }
            break;

        /****************************ERROR*******************************************/
        case SYS_FSM_STATE_ERROR:
            nextState = SYS_ProcessErrorState(pSystemState);
            if (nextState == SYS_FSM_STATE_ERROR) {
                /* staying in state, processed by state function */
                pSystemState->timer = SYS_FSM_LONG_TIME;
            } else {
                FAS_ASSERT(FAS_TRAP); /* Something went wrong */
            }
            break;
        /***************************DEFAULT CASE*************************************/
        default:
            FAS_ASSERT(FAS_TRAP); /* invalid state */
            break;
    }
    return ranStateMachine;
}

static SYS_STATE_REQUEST_e SYS_TransferStateRequest(void) {
    SYS_STATE_REQUEST_e requestedState = SYS_STATE_NO_REQUEST;

    OS_EnterTaskCritical();
    requestedState         = sys_state.stateRequest;
    sys_state.stateRequest = SYS_STATE_NO_REQUEST;
    OS_ExitTaskCritical();

    return requestedState;
}

static SYS_RETURN_TYPE_e SYS_CheckStateRequest(SYS_STATE_REQUEST_e stateRequest) {
    SYS_RETURN_TYPE_e retval = SYS_ILLEGAL_REQUEST;
    if (stateRequest == SYS_STATE_ERROR_REQUEST) {
        retval = SYS_OK;
    } else {
        if (sys_state.stateRequest == SYS_STATE_NO_REQUEST) {
            /* initialization is only allowed from the uninitialized state */
            if (stateRequest == SYS_STATE_INITIALIZATION_REQUEST) {
                if (sys_state.currentState == SYS_FSM_STATE_UNINITIALIZED) {
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

static void SYS_GeneralMacroBist(void) {
    const uint8_t dummy[GEN_REPEAT_MAXIMUM_REPETITIONS] = {
        GEN_REPEAT_U(SYS_BIST_GENERAL_MAGIC_NUMBER, GEN_STRIP(GEN_REPEAT_MAXIMUM_REPETITIONS))};
    for (uint8_t i = 0u; i < GEN_REPEAT_MAXIMUM_REPETITIONS; i++) {
        FAS_ASSERT(dummy[i] == SYS_BIST_GENERAL_MAGIC_NUMBER);
    }
}

/*========== Extern Function Implementations ================================*/

SYS_RETURN_TYPE_e SYS_SetStateRequest(SYS_STATE_REQUEST_e stateRequest) {
    SYS_RETURN_TYPE_e stateRequestStatus = SYS_ILLEGAL_REQUEST;

    OS_EnterTaskCritical();
    stateRequestStatus = SYS_CheckStateRequest(stateRequest);

    if (stateRequestStatus == SYS_OK) {
        sys_state.stateRequest = stateRequest;
    }
    OS_ExitTaskCritical();

    return stateRequestStatus;
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
        (void)SYS_RunStateMachine(pSystemState);
        pSystemState->triggerEntry--;
    }
    return returnValue;
}

extern SYS_FSM_STATES_e SYS_GetSystemState(SYS_STATE_s *pSystemState) {
    return pSystemState->currentState;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
STD_RETURN_TYPE_e TEST_SYS_RunStateMachine(SYS_STATE_s *pSystemState) {
    return SYS_RunStateMachine(pSystemState);
}
STD_RETURN_TYPE_e TEST_SYS_CheckStateRequest(SYS_STATE_REQUEST_e stateRequest) {
    return SYS_CheckStateRequest(stateRequest);
}
void TEST_SYS_SetState(
    SYS_STATE_s *pSystemState,
    SYS_FSM_STATES_e nextState,
    SYS_FSM_SUBSTATES_e nextSubstate,
    uint16_t idleTime) {
    SYS_SetState(pSystemState, nextState, nextSubstate, idleTime);
}
void TEST_SYS_GeneralMacroBist(void) {
    SYS_GeneralMacroBist();
}
#endif
