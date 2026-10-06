/**
 *
 * @copyright &copy; 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    bms.c
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup ENGINE
 * @prefix  BMS
 *
 * @brief   Implement the BMS driver
 * @details Implement the state machine that controls the BMS
 *
 */

/*========== Includes =======================================================*/
#include "bms.h"

#include "afe.h"
#include "bal.h"
#include "battery_cell_cfg_types.h"
#include "can_cbs_tx_cyclic.h"
#include "database.h"
#include "diag.h"
#include "foxmath.h"
#include "imd.h"
#include "led.h"
#include "meas.h"
#include "os.h"
#include "soa.h"
#include "sps.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** default value for unset "active delay time" */
#define BMS_NO_ACTIVE_DELAY_TIME_ms (UINT32_MAX)

/** Symbolic names to check for multiple calls of #BMS_Trigger */
typedef enum {
    BMS_MULTIPLE_CALLS_NO,  /*!< no multiple calls, OK */
    BMS_MULTIPLE_CALLS_YES, /*!< multiple calls, not OK */
} BMS_CHECK_MULTIPLE_CALLS_e;

/*========== Static Constant and Variable Definitions =======================*/

/**
 * contains the state of the bms state machine
 */
static BMS_STATE_s bms_state = {
    .information =
        {
            .stringNumber                      = 0u,
            .currentSystick                    = 0u,
            .ErrRequestCounter                 = 0u,
            .initFinished                      = STD_NOT_OK,
            .counter                           = 0u,
            .OscillationTimeout                = 0u,
            .prechargeTryCounter               = 0u,
            .powerPath                         = BMS_POWER_PATH_OPEN,
            .closedStrings                     = {GEN_REPEAT_U(false, GEN_STRIP(BS_NR_OF_STRINGS))},
            .closedPrechargeContactors         = {GEN_REPEAT_U(false, GEN_STRIP(BS_NR_OF_STRINGS))},
            .numberOfClosedStrings             = 0u,
            .deactivatedStrings                = {GEN_REPEAT_U(false, GEN_STRIP(BS_NR_OF_STRINGS))},
            .firstClosedString                 = 0u,
            .stringOpenTimeout                 = 0u,
            .nextStringClosedTimer             = 0u,
            .stringCloseTimeout                = 0u,
            .restTimer_10ms                    = BS_RELAXATION_PERIOD_10ms,
            .currentFlowState                  = BMS_RELAXATION,
            .remainingDelay_ms                 = BMS_NO_ACTIVE_DELAY_TIME_ms,
            .minimumActiveDelay_ms             = BMS_NO_ACTIVE_DELAY_TIME_ms,
            .startOfPrecharging                = 0u,
            .transitionToErrorState            = false,
            .timeAboveContactorBreakCurrent_ms = 0u,
            .stringToBeOpened                  = 0u,
            .contactorToBeOpened               = CONT_UNDEFINED,
            .nextStringNumber                  = 0u,
        },
    .timer            = 0u,
    .stateRequest     = BMS_STATE_NO_REQUEST,
    .triggerEntry     = 0u,
    .currentState     = BMS_FSM_STATE_HAS_NEVER_RUN,
    .currentSubstate  = BMS_FSM_SUBSTATE_DUMMY,
    .previousState    = BMS_FSM_STATE_HAS_NEVER_RUN,
    .previousSubstate = BMS_FSM_SUBSTATE_DUMMY,
    .nextState        = BMS_FSM_STATE_HAS_NEVER_RUN,
    .nextSubstate     = BMS_FSM_SUBSTATE_DUMMY,
};

/** local copies of database tables */
/**@{*/
static DATA_BLOCK_MIN_MAX_s bms_tableMinMax         = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
static DATA_BLOCK_OPEN_WIRE_s bms_tableOpenWire     = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};
static DATA_BLOCK_PACK_VALUES_s bms_tablePackValues = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
/**@}*/

/*========== Extern Constant and Variable Definitions =======================*/

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
 * @param   pBmsState state of the BMS state machine
 * @return  #BMS_MULTIPLE_CALLS_YES if there were multiple calls,
 *          #BMS_MULTIPLE_CALLS_NO otherwise
 */
static BMS_CHECK_MULTIPLE_CALLS_e BMS_CheckMultipleCalls(BMS_STATE_s *pBmsState);

/**
 * @brief   Sets the next state, the next substate and the timer value
 *          of the state variable.
 * @param   pBmsState       state of the BMS state machine
 * @param   nextState      state to be transferred into
 * @param   nextSubstate   substate to be transferred into
 * @param   idleTime       wait time for the state machine
 */
static void BMS_SetState(
    BMS_STATE_s *pBmsState,
    BMS_FSM_STATES_e nextState,
    BMS_FSM_SUBSTATES_e nextSubstate,
    uint16_t idleTime);

/**
 * @brief   Sets the next substate and the timer value
 *          of the state variable.
 * @param   pBmsState       state of the BMS state machine
 * @param   nextSubstate   substate to be transferred into
 * @param   idleTime       wait time for the state machine
 */
static void BMS_SetSubstate(BMS_STATE_s *pBmsState, BMS_FSM_SUBSTATES_e nextSubstate, uint16_t idleTime);

/**
 * @brief       checks the state requests that are made.
 * @details     Check the validity of the state requests. The
 *              results of the checked is returned immediately.
 * @param[in]   statereq    state request to be checked
 * @return      result of the state request that was made
 */
static BMS_RETURN_TYPE_e BMS_CheckStateRequest(BMS_STATE_REQUEST_e statereq);

/**
 * @brief   transfers the current state request to the state machine.
 * @details Transfer the current state request from #bms_state,
 *          transfers it to the state machine. It resets the value from
 *          #bms_state to #BMS_STATE_NO_REQUEST
 * @return  current state request
 */
static BMS_STATE_REQUEST_e BMS_TransferStateRequest(void);

/**
 * @brief   Checks the state requests made to the BMS state machine.
 * @details Checks of the state request in the database and sets this value as
 *          return value.
 * @return  requested state
 */
static uint8_t BMS_CheckCanRequests(void);

/**
 * @brief   Checks all the error flags from diagnosis module with a severity of
 *          #DIAG_FATAL_ERROR
 * @details Checks all the error flags from diagnosis module with a severity of
 *          #DIAG_FATAL_ERROR. Furthermore, sets parameter minimumActiveDelay_ms
 *          of bms_state variable.
 * @return  true if error flag is set, otherwise false
 */
static bool BMS_IsAnyFatalErrorFlagSet(void);

/**
 * @brief   Checks if any error flag is set and handles delay until contactors
 *          need to be opened.
 * @details Checks all the diagnosis entries with severity of #DIAG_FATAL_ERROR
 *          and handles the configured delay until the contactors need to be
 *          opened. The shortest delay is used, if multiple errors are active at
 *          once.
 * @return  #STD_NOT_OK if error detected and delay time elapsed, otherwise #STD_OK
 */
static STD_RETURN_TYPE_e BMS_IsBatterySystemStateOkay(BMS_STATE_s *pBmsState);

/**
 * @brief   Checks if the contactor feedback for a specific contactor is valid
 *          need to be opened.
 * @details Reads error flag database entry and checks if the feedback for this
 *          specific contactor is valid or not.
 * @return  true if no error detected feedback is valid, otherwise false
 */
static bool BMS_IsContactorFeedbackValid(uint8_t stringNumber, CONT_TYPE_e contactorType);

/** Get latest database entries for static module variables */
static void BMS_GetMeasurementValues(void);

/**
 * @brief   Check for any open voltage sense wire
 */
static void BMS_CheckOpenSenseWire(void);

/**
 * @brief       Checks if the current limitations are violated
 * @param[in]   pBmsState             pointer to BMS state machine variable
 * @param[in]   stringNumber          string addressed
 * @param[in]   pPackValues           pointer to pack values database entry
 * @param[in]   monitoringParameters
 * @param[in]   timeout_ms
 * @return      BMS_PRECHARGING_SUCCESSFUL if precharging succeeded
 *              BMS_PRECHARGING_ONGOING if precharging is ongoing
 *              BMS_PRECHARGING_FAILED if timeout reached and precharge
 *              process was not successful (type: #BMS_RESULT_PRECHARGE_PROCESS_e)
 */
static BMS_RESULT_PRECHARGE_PROCESS_e BMS_MonitorPrechargeProcess(
    BMS_STATE_s *pBmsState,
    uint8_t stringNumber,
    const DATA_BLOCK_PACK_VALUES_s *pPackValues,
    BS_PRECHARGE_MONITORING_e monitoringParameters,
    uint32_t timeout_ms);

/**
 * @brief       Checks if passed battery current is below limit
 * @param[in]   stringNumber string addressed
 * @param[in]   pPackValues  pointer to pack values database entry
 * @return      #STD_OK if battery current is below limit, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e BMS_IsPrechargeCurrentBelowLimit(
    uint8_t stringNumber,
    const DATA_BLOCK_PACK_VALUES_s *pPackValues);

/**
 * @brief       Checks if the current limitations are violated
 * @param[in]   stringNumber string addressed
 * @param[in]   pPackValues  pointer to pack values database entry
 * @return      true if voltage difference between battery and DC link voltage is below limit, otherwise false
 */
static STD_RETURN_TYPE_e BMS_IsPrechargeVoltageBelowLimit(
    uint8_t stringNumber,
    const DATA_BLOCK_PACK_VALUES_s *pPackValues);

/**
 * @brief   Returns ID of string with highest total voltage
 * @details Use this helper to select the first string when drive-off is requested.
 * @param[in]   precharge   If #BMS_DO_NOT_TAKE_PRECHARGE_INTO_ACCOUNT,
 *                          precharge availability for string is ignored.
 *                          if #BMS_TAKE_PRECHARGE_INTO_ACCOUNT, only select
 *                          string that has precharge available.
 * @param[in]   pPackValues pointer to pack values database entry
 * @return  index of string with highest voltage If no string is available,
 *          returns #BMS_NO_STRING_AVAILABLE.
 */
static uint8_t BMS_GetHighestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues);

/**
 * @brief   Returns ID of string with voltage closest to first closed string voltage
 * @details Use this helper to close further strings in drive.
 * @param[in]   precharge   If #BMS_DO_NOT_TAKE_PRECHARGE_INTO_ACCOUNT,
 *                          precharge availability for string is ignored.
 *                          if #BMS_TAKE_PRECHARGE_INTO_ACCOUNT, only select
 *                          string that has precharge available.
 * @param[in]   pPackValues pointer to pack values database entry
 * @return  index of string with voltage closest to the first closed string voltage.
 *          If no string is available, returns #BMS_NO_STRING_AVAILABLE.
 */
static uint8_t BMS_GetClosestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues);

/**
 * @brief   Returns ID of string with lowest total voltage
 * @details Use this helper to close the first string when charge-off is requested.
 *
 * @param[in]   precharge   If 0, precharge availability for string is ignored.
 *                          If 1, only selects a string that has precharge
 *                          available.
 * @param[in]   pPackValues pointer to pack values database entry
 * @return  index of string with lowest voltage. If no string is available,
 *          returns #BMS_NO_STRING_AVAILABLE.
 */
static uint8_t BMS_GetLowestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues);

/**
 * @brief   Returns voltage difference between first closed string and
 *          string ID
 * @details Check voltage when trying to close further
 *          strings.
 * @param[in]   string  ID of string that must be compared with first closed
 *                      string
 * @param[in]   pPackValues pointer to pack values database entry
 * @return voltage difference in mV, will return INT32_MAX if voltages are
 *         invalid and difference can not be calculated
 */
static int32_t BMS_GetStringVoltageDifference(uint8_t string, const DATA_BLOCK_PACK_VALUES_s *pPackValues);

/**
 * @brief   Returns the average current flowing through all strings.
 * @details Use this helper when closing strings.
 * @param[in]   pPackValues pointer to pack values database entry
 * @return  average current taking all strings into account in mA. INT32_MAX if there is no valid current measurement
 */
static int32_t BMS_GetAverageStringCurrent(DATA_BLOCK_PACK_VALUES_s *pPackValues);

/**
 * @brief   Updates battery state variable depending on measured/recent
 *          current values
 * @param[in]   pPackValues  recent measured values from current sensor
 */
static void BMS_UpdateBatterySystemState(DATA_BLOCK_PACK_VALUES_s *pPackValues);

/**
 * @brief   Get first string contactor that should be opened depending on the
 *          actual current flow direction
 * @details Check the mounting direction of the contactors and open the
 *          contactor that is mounted in the preferred current flow direction.
 *          Open the plus contactor first if, there is no contactor in
 *          preferred direction to the current flow to open available. This may
 *          be either because both contactors are installed in the same
 *          direction or because the contactors are bidirectional.
 * @param stringNumber         string that will be opened
 * @param flowDirection        current flow direction (charging or discharging)
 * @return #CONT_TYPE_e contactor that should be opened
 */
static CONT_TYPE_e BMS_GetFirstContactorToBeOpened(uint8_t stringNumber, BMS_CURRENT_FLOW_STATE_e flowDirection);

/**
 * @brief   Get second string contactor that should be opened
 * @details Mounting direction of the contactor does not need to be checked
 *          for the second contactor as the current has already been
 *          interrupted opening the first contactor.
 * @param stringNumber             string that will be opened
 * @param firstOpenedContactorType type of first contactor that has been opened
 * @return #CONT_TYPE_e contactor that should be opened
 */
static CONT_TYPE_e BMS_GetSecondContactorToBeOpened(uint8_t stringNumber, CONT_TYPE_e firstOpenedContactorType);

/*========== Static Function Implementations ================================*/

static BMS_CHECK_MULTIPLE_CALLS_e BMS_CheckMultipleCalls(BMS_STATE_s *pBmsState) {
    FAS_ASSERT(pBmsState != NULL_PTR);
    BMS_CHECK_MULTIPLE_CALLS_e multipleCalls = BMS_MULTIPLE_CALLS_NO;
    OS_EnterTaskCritical();
    if (pBmsState->triggerEntry == 0u) {
        pBmsState->triggerEntry++;
    } else {
        multipleCalls = BMS_MULTIPLE_CALLS_YES; /* multiple call of function BMS_Trigger for instance pBmsState */
    }
    OS_ExitTaskCritical();
    return multipleCalls;
}

static void BMS_SetState(
    BMS_STATE_s *pBmsState,
    BMS_FSM_STATES_e nextState,
    BMS_FSM_SUBSTATES_e nextSubstate,
    uint16_t idleTime) {
    FAS_ASSERT(pBmsState != NULL_PTR);
    bool earlyExit = false;

    pBmsState->timer            = idleTime;
    pBmsState->previousState    = pBmsState->currentState;
    pBmsState->previousSubstate = pBmsState->currentSubstate;

    if ((pBmsState->currentState == nextState) && (pBmsState->currentSubstate == nextSubstate)) {
        /* Next currentState and next currentSubstate equal to current currentState and currentSubstate: nothing to do */
        pBmsState->nextState    = BMS_FSM_STATE_DUMMY;    /* no currentState transition required -> reset */
        pBmsState->nextSubstate = BMS_FSM_SUBSTATE_DUMMY; /* no currentSubstate transition required -> reset */
        earlyExit               = true;
    }

    if (earlyExit == false) {
        if (pBmsState->currentState != nextState) {
            /* distinguish between just a currentState transfer to the error currentState and a normal currentState transfer */
            if (nextState == BMS_FSM_STATE_ERROR) {
                /* Error currentState gets treated differently since we dont need to enter it through the entry currentSubstate */
                pBmsState->currentState    = nextState;
                pBmsState->currentSubstate = nextSubstate;
            } else {
                /* Next currentState is different than the current one: switch to it and set currentSubstate to entry value */
                pBmsState->previousState    = pBmsState->currentState;
                pBmsState->currentState     = nextState;
                pBmsState->previousSubstate = pBmsState->currentSubstate;
                pBmsState->currentSubstate =
                    BMS_FSM_SUBSTATE_ENTRY; /* entry currentState after a top level currentState change */
                pBmsState->nextState    = BMS_FSM_STATE_DUMMY;    /* no currentState transition required -> reset */
                pBmsState->nextSubstate = BMS_FSM_SUBSTATE_DUMMY; /* no currentSubstate transition required -> reset */
            }
        } else if (pBmsState->currentSubstate != nextSubstate) {
            /* Only the next currentSubstate is different, switch to it */
            BMS_SetSubstate(pBmsState, nextSubstate, idleTime);
        } else {
            ;
        }
    }
}

static void BMS_SetSubstate(BMS_STATE_s *pBmsState, BMS_FSM_SUBSTATES_e nextSubstate, uint16_t idleTime) {
    FAS_ASSERT(pBmsState != NULL_PTR);
    pBmsState->timer            = idleTime;
    pBmsState->previousSubstate = pBmsState->currentSubstate;
    pBmsState->currentSubstate  = nextSubstate;
    pBmsState->nextSubstate =
        BMS_FSM_SUBSTATE_DUMMY; /* currentSubstate has been set, now reset value for nextSubstate */
}

static BMS_RETURN_TYPE_e BMS_CheckStateRequest(BMS_STATE_REQUEST_e statereq) {
    if (statereq == BMS_STATE_ERROR_REQUEST) {
        return BMS_OK;
    }

    if (bms_state.stateRequest == BMS_STATE_NO_REQUEST) {
        /* init only allowed from the uninitialized currentState */
        if (statereq == BMS_STATE_INITIALIZATION_REQUEST) {
            if (bms_state.currentState == BMS_FSM_STATE_UNINITIALIZED) {
                return BMS_OK;
            } else {
                return BMS_ALREADY_INITIALIZED;
            }
        } else {
            return BMS_ILLEGAL_REQUEST;
        }
    } else {
        return BMS_REQUEST_PENDING;
    }
}

static BMS_STATE_REQUEST_e BMS_TransferStateRequest(void) {
    BMS_STATE_REQUEST_e retval = BMS_STATE_NO_REQUEST;

    OS_EnterTaskCritical();
    retval                 = bms_state.stateRequest;
    bms_state.stateRequest = BMS_STATE_NO_REQUEST;
    OS_ExitTaskCritical();
    return retval;
}

static void BMS_GetMeasurementValues(void) {
    DATA_READ_DATA(&bms_tablePackValues, &bms_tableOpenWire, &bms_tableMinMax);
}

static uint8_t BMS_CheckCanRequests(void) {
    uint8_t retVal                     = BMS_REQ_ID_NOREQ;
    DATA_BLOCK_STATE_REQUEST_s request = {.header.uniqueId = DATA_BLOCK_ID_STATE_REQUEST};

    DATA_READ_DATA(&request);

    if (request.stateRequestViaCan == BMS_REQ_ID_STANDBY) {
        retVal = BMS_REQ_ID_STANDBY;
    } else if (request.stateRequestViaCan == BMS_REQ_ID_NORMAL) {
        retVal = BMS_REQ_ID_NORMAL;
    } else if (request.stateRequestViaCan == BMS_REQ_ID_CHARGE) {
        retVal = BMS_REQ_ID_CHARGE;
    } else if (request.stateRequestViaCan == BMS_REQ_ID_NOREQ) {
        retVal = BMS_REQ_ID_NOREQ;
    } else {
        /* invalid or no request, default to BMS_REQ_ID_NOREQ (already set) */
    }

    return retVal;
}

static void BMS_CheckOpenSenseWire(void) {
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        const bool openWireDetected = bms_tableOpenWire.nrOpenWires[s] > 0u;
        if (openWireDetected == false) {
            DIAG_Handler(DIAG_ID_AFE_OPEN_WIRE, DIAG_EVENT_OK, DIAG_STRING, s);
        } else {
            DIAG_Handler(DIAG_ID_AFE_OPEN_WIRE, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
        }
    }
}

static BMS_RESULT_PRECHARGE_PROCESS_e BMS_MonitorPrechargeProcess(
    BMS_STATE_s *pBmsState,
    uint8_t stringNumber,
    const DATA_BLOCK_PACK_VALUES_s *pPackValues,
    BS_PRECHARGE_MONITORING_e monitoringParameters,
    uint32_t timeout_ms) {
    /* make sure that we do not access the arrays in the database
       tables out of bounds */
    FAS_ASSERT(pBmsState != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    FAS_ASSERT(pPackValues != NULL_PTR);
    FAS_ASSERT(
        (monitoringParameters == BS_PRECHARGE_MONITOR_CURRENT) ||
        (monitoringParameters == BS_PRECHARGE_MONITOR_VOLTAGE) ||
        (monitoringParameters == BS_PRECHARGE_MONITOR_CURRENT_AND_VOLTAGE));
    /* AXIVION Routine Generic-MissingParameterAssert: timeout_ms: parameter accepts whole range */
    /* Indicate precharging ongoing until it has been finished or precharging failed */
    BMS_RESULT_PRECHARGE_PROCESS_e prechargingState = BMS_PRECHARGING_ONGOING;
    STD_RETURN_TYPE_e currentPrecharged             = STD_NOT_OK;
    STD_RETURN_TYPE_e voltagePrecharged             = STD_NOT_OK;
    if (monitoringParameters == BS_PRECHARGE_MONITOR_CURRENT) {
        currentPrecharged = BMS_IsPrechargeCurrentBelowLimit(stringNumber, pPackValues);
        voltagePrecharged = STD_OK;
    } else if (monitoringParameters == BS_PRECHARGE_MONITOR_VOLTAGE) {
        voltagePrecharged = BMS_IsPrechargeVoltageBelowLimit(stringNumber, pPackValues);
        currentPrecharged = STD_OK;
    } else {
        /* monitoringParameters == BS_PRECHARGE_MONITOR_CURRENT_AND_VOLTAGE */
        currentPrecharged = BMS_IsPrechargeCurrentBelowLimit(stringNumber, pPackValues);
        voltagePrecharged = BMS_IsPrechargeVoltageBelowLimit(stringNumber, pPackValues);
    }

    if ((currentPrecharged == STD_OK) && (voltagePrecharged == STD_OK)) {
        prechargingState = BMS_PRECHARGING_FINISHED;
        (void)DIAG_Handler(DIAG_ID_PRECHARGE_ABORT_REASON_VOLTAGE, DIAG_EVENT_OK, DIAG_STRING, stringNumber);
        (void)DIAG_Handler(DIAG_ID_PRECHARGE_ABORT_REASON_CURRENT, DIAG_EVENT_OK, DIAG_STRING, stringNumber);
    } else {
        /* Check if precharging timeout has reached to indicate a failure or not */
        if (pBmsState->information.currentSystick - pBmsState->information.startOfPrecharging > timeout_ms) {
            prechargingState = BMS_PRECHARGING_FAILED;
            DIAG_ReportResultToHandler(
                currentPrecharged, DIAG_ID_PRECHARGE_ABORT_REASON_CURRENT, DIAG_STRING, stringNumber);
            DIAG_ReportResultToHandler(
                voltagePrecharged, DIAG_ID_PRECHARGE_ABORT_REASON_VOLTAGE, DIAG_STRING, stringNumber);
        }
    }
    return prechargingState;
}

static STD_RETURN_TYPE_e BMS_IsPrechargeCurrentBelowLimit(
    uint8_t stringNumber,
    const DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    /* AXIVION Routine Generic-MissingParameterAssert: stringNumber: function parameters are checked by caller */
    /* AXIVION Routine Generic-MissingParameterAssert: pPackValues: function parameters are checked by caller */
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    /* Only current, not the current direction is checked */
    if ((pPackValues->invalidStringCurrent[stringNumber] == 0u) &&
        ((MATH_AbsInt32_t(pPackValues->stringCurrent_mA[stringNumber]) < BMS_PRECHARGE_CURRENT_THRESHOLD_mA))) {
        retval = STD_OK;
    }
    return retval;
}

static STD_RETURN_TYPE_e BMS_IsPrechargeVoltageBelowLimit(
    uint8_t stringNumber,
    const DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    /* AXIVION Routine Generic-MissingParameterAssert: stringNumber: function parameters are checked by caller */
    /* AXIVION Routine Generic-MissingParameterAssert: pPackValues: function parameters are checked by caller */
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    if ((pPackValues->invalidStringVoltage[stringNumber] == 0u) && (pPackValues->invalidHvBusVoltage == 0u)) {
        const int64_t cont_prechargeVoltDiff_mV = MATH_AbsInt64_t(
            (int64_t)pPackValues->stringVoltage_mV[stringNumber] - (int64_t)pPackValues->highVoltageBusVoltage_mV);
        if (cont_prechargeVoltDiff_mV < BMS_PRECHARGE_VOLTAGE_THRESHOLD_mV) {
            retval = STD_OK;
        }
    }
    return retval;
}

static bool BMS_IsAnyFatalErrorFlagSet(void) {
    bool fatalErrorActive = false;

    for (uint16_t entry = 0u; entry < diag_device.numberOfFatalErrors; entry++) {
        const STD_RETURN_TYPE_e diagnosisState =
            DIAG_GetDiagnosisEntryState(diag_device.pFatalErrorLinkTable[entry]->id);
        if (STD_NOT_OK == diagnosisState) {
            /* Fatal error detected -> get delay of this error until contactors shall be opened */
            const uint32_t kDelay_ms = DIAG_GetDelay(diag_device.pFatalErrorLinkTable[entry]->id);
            /* Check if delay of detected failure is smaller than the delay of a previously detected failure */
            if (bms_state.information.minimumActiveDelay_ms > kDelay_ms) {
                bms_state.information.minimumActiveDelay_ms = kDelay_ms;
            }
            fatalErrorActive = true;
        }
    }
    return fatalErrorActive;
}

static STD_RETURN_TYPE_e BMS_IsBatterySystemStateOkay(BMS_STATE_s *pBmsState) {
    STD_RETURN_TYPE_e retVal          = STD_OK; /* is set to STD_NOT_OK if error detected */
    static uint32_t previousTimestamp = 0u;
    uint32_t timestamp                = OS_GetTickCount();

    /* Check if any fatal error is detected */
    const bool isErrorActive = BMS_IsAnyFatalErrorFlagSet();

    /** Check if a fatal error has been detected previously. If yes, check delay */
    if (pBmsState->information.transitionToErrorState == true) {
        /* Decrease active delay since last call */
        const uint32_t timeSinceLastCall_ms = timestamp - previousTimestamp;
        if (timeSinceLastCall_ms <= pBmsState->information.remainingDelay_ms) {
            pBmsState->information.remainingDelay_ms -= timeSinceLastCall_ms;
        } else {
            pBmsState->information.remainingDelay_ms = 0u;
        }

        /* Check if delay from a new error is shorter then active delay from
         * previously detected error in BMS state machine */
        if (pBmsState->information.remainingDelay_ms >= pBmsState->information.minimumActiveDelay_ms) {
            pBmsState->information.remainingDelay_ms = pBmsState->information.minimumActiveDelay_ms;
        }
    } else {
        /* Delay is not active, check if it should be activated */
        if (isErrorActive == true) {
            pBmsState->information.transitionToErrorState = true;
            pBmsState->information.remainingDelay_ms      = pBmsState->information.minimumActiveDelay_ms;
        }
    }

    /** Set previous timestamp for next call */
    previousTimestamp = timestamp;

    /* Check if bms state machine should switch to error state. This is the case
     * if the delay is activated and the remaining delay is down to 0 */
    if ((pBmsState->information.transitionToErrorState == true) && (pBmsState->information.remainingDelay_ms == 0u)) {
        retVal = STD_NOT_OK;
    }

    return retVal;
}

static bool BMS_IsContactorFeedbackValid(uint8_t stringNumber, CONT_TYPE_e contactorType) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    FAS_ASSERT(contactorType != CONT_UNDEFINED);
    bool feedbackValid = false;
    /* Read latest error flags from database */
    DATA_BLOCK_ERROR_STATE_s tableErrorFlags = {.header.uniqueId = DATA_BLOCK_ID_ERROR_STATE};
    DATA_READ_DATA(&tableErrorFlags);
    /* Check if contactor feedback is valid */
    switch (contactorType) {
        case CONT_PLUS:
            if (tableErrorFlags.contactorInPositivePathOfStringFeedbackError[stringNumber] == false) {
                feedbackValid = true;
            }
            break;
        case CONT_MINUS:
            if (tableErrorFlags.contactorInNegativePathOfStringFeedbackError[stringNumber] == false) {
                feedbackValid = true;
            }
            break;
        case CONT_PRECHARGE:
            if (tableErrorFlags.prechargeContactorFeedbackError[stringNumber] == false) {
                feedbackValid = true;
            }
            break;
        default:
            /* CONT_UNDEFINED already prevent via assert */
            break;
    }
    return feedbackValid;
}

static uint8_t BMS_GetHighestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    FAS_ASSERT(pPackValues != NULL_PTR);
    uint8_t highest_string_index = BMS_NO_STRING_AVAILABLE;
    int32_t max_stringVoltage_mV = INT32_MIN;

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        if ((pPackValues->stringVoltage_mV[s] >= max_stringVoltage_mV) &&
            (pPackValues->invalidStringVoltage[s] == 0u)) {
            if (bms_state.information.deactivatedStrings[s] == false) {
                if (precharge == BMS_DO_NOT_TAKE_PRECHARGE_INTO_ACCOUNT) {
                    max_stringVoltage_mV = pPackValues->stringVoltage_mV[s];
                    highest_string_index = s;
                } else {
                    if (bs_stringsWithPrecharge[s] == BS_STRING_WITH_PRECHARGE) {
                        max_stringVoltage_mV = pPackValues->stringVoltage_mV[s];
                        highest_string_index = s;
                    }
                }
            }
        }
    }

    return highest_string_index;
}

static uint8_t BMS_GetClosestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    FAS_ASSERT(pPackValues != NULL_PTR);
    uint8_t closestStringIndex     = BMS_NO_STRING_AVAILABLE;
    int32_t closedStringVoltage_mV = 0;
    bool searchString              = false;

    /* Get voltage of first closed string */
    if (pPackValues->invalidStringVoltage[bms_state.information.firstClosedString] == 0u) {
        closedStringVoltage_mV = pPackValues->stringVoltage_mV[bms_state.information.firstClosedString];
        searchString           = true;
    } else if (pPackValues->invalidHvBusVoltage == 0u) {
        /* Use high voltage bus voltage if measured string voltage is invalid */
        closedStringVoltage_mV = pPackValues->highVoltageBusVoltage_mV;
        searchString           = true;
    } else {
        /* Do not search for next string  if no valid voltages could be measured */
        searchString = false;
    }

    if (searchString == true) {
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            const bool isStringClosed          = BMS_IsStringClosed(s);
            const uint8_t isStringVoltageValid = pPackValues->invalidStringVoltage[s];
            if ((isStringClosed == false) && (isStringVoltageValid == 0u)) {
                /* Only check open strings with valid voltages */
                int32_t minimumVoltageDifference_mV = INT32_MAX;
                int32_t voltageDifference_mV        = labs(closedStringVoltage_mV - pPackValues->stringVoltage_mV[s]);
                if (voltageDifference_mV <= minimumVoltageDifference_mV) {
                    if (bms_state.information.deactivatedStrings[s] == false) {
                        if (precharge == BMS_TAKE_PRECHARGE_INTO_ACCOUNT) {
                            if (bs_stringsWithPrecharge[s] == BS_STRING_WITH_PRECHARGE) {
                                minimumVoltageDifference_mV = voltageDifference_mV;
                                closestStringIndex          = s;
                            }
                        } else {
                            minimumVoltageDifference_mV = voltageDifference_mV;
                            closestStringIndex          = s;
                        }
                    }
                }
            }
        }
    }
    return closestStringIndex;
}

static uint8_t BMS_GetLowestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    FAS_ASSERT(pPackValues != NULL_PTR);
    uint8_t lowest_string_index  = BMS_NO_STRING_AVAILABLE;
    int32_t min_stringVoltage_mV = INT32_MAX;

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        if ((pPackValues->stringVoltage_mV[s] <= min_stringVoltage_mV) &&
            (pPackValues->invalidStringVoltage[s] == 0u)) {
            if (bms_state.information.deactivatedStrings[s] == false) {
                if (precharge == BMS_DO_NOT_TAKE_PRECHARGE_INTO_ACCOUNT) {
                    min_stringVoltage_mV = pPackValues->stringVoltage_mV[s];
                    lowest_string_index  = s;
                } else {
                    if (bs_stringsWithPrecharge[s] == BS_STRING_WITH_PRECHARGE) {
                        min_stringVoltage_mV = pPackValues->stringVoltage_mV[s];
                        lowest_string_index  = s;
                    }
                }
            }
        }
    }
    return lowest_string_index;
}

static int32_t BMS_GetStringVoltageDifference(uint8_t string, const DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    FAS_ASSERT(string < BS_NR_OF_STRINGS);
    FAS_ASSERT(pPackValues != NULL_PTR);
    int32_t voltageDifference_mV = INT32_MAX;
    if ((pPackValues->invalidStringVoltage[string] == 0u) &&
        (pPackValues->invalidStringVoltage[bms_state.information.firstClosedString] == 0u)) {
        /* Calculate difference between string voltages */
        voltageDifference_mV = MATH_AbsInt32_t(
            pPackValues->stringVoltage_mV[string] -
            pPackValues->stringVoltage_mV[bms_state.information.firstClosedString]);
    } else if ((pPackValues->invalidStringVoltage[string] == 0u) && (pPackValues->invalidHvBusVoltage == 0u)) {
        /* Calculate difference between string and high voltage bus voltage */
        voltageDifference_mV =
            MATH_AbsInt32_t(pPackValues->stringVoltage_mV[string] - pPackValues->highVoltageBusVoltage_mV);
    } else {
        /* No valid voltages for comparison -> do not calculate difference and return INT32_MAX */
        voltageDifference_mV = INT32_MAX;
    }
    return voltageDifference_mV;
}

static int32_t BMS_GetAverageStringCurrent(DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    FAS_ASSERT(pPackValues != NULL_PTR);
    int32_t average_current = pPackValues->packCurrent_mA / (int32_t)BS_NR_OF_STRINGS;
    if (pPackValues->invalidPackCurrent == 1u) {
        average_current = INT32_MAX;
    }
    return average_current;
}

static void BMS_UpdateBatterySystemState(DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    FAS_ASSERT(pPackValues != NULL_PTR);

    /* Only update system state if current value is valid */
    if (pPackValues->invalidPackCurrent == 0u) {
        if (BS_POSITIVE_DISCHARGE_CURRENT == true) {
            /* Positive current values equal a discharge of the battery system */
            if (pPackValues->packCurrent_mA >= BS_REST_CURRENT_mA) { /* TODO: string use pack current */
                bms_state.information.currentFlowState = BMS_DISCHARGING;
                bms_state.information.restTimer_10ms   = BS_RELAXATION_PERIOD_10ms;
            } else if (pPackValues->packCurrent_mA <= -BS_REST_CURRENT_mA) {
                bms_state.information.currentFlowState = BMS_CHARGING;
                bms_state.information.restTimer_10ms   = BS_RELAXATION_PERIOD_10ms;
            } else {
                /* Current below rest current: either battery system is at rest
                 * or the relaxation process is still ongoing */
                if (bms_state.information.restTimer_10ms == 0u) {
                    /* Rest timer elapsed -> battery system at rest */
                    bms_state.information.currentFlowState = BMS_AT_REST;
                } else {
                    bms_state.information.restTimer_10ms--;
                    bms_state.information.currentFlowState = BMS_RELAXATION;
                }
            }
        } else {
            /* Negative current values equal a discharge of the battery system */
            if (pPackValues->packCurrent_mA <= -BS_REST_CURRENT_mA) {
                bms_state.information.currentFlowState = BMS_DISCHARGING;
                bms_state.information.restTimer_10ms   = BS_RELAXATION_PERIOD_10ms;
            } else if (pPackValues->packCurrent_mA >= BS_REST_CURRENT_mA) {
                bms_state.information.currentFlowState = BMS_CHARGING;
                bms_state.information.restTimer_10ms   = BS_RELAXATION_PERIOD_10ms;
            } else {
                /* Current below rest current: either battery system is at rest
                 * or the relaxation process is still ongoing */
                if (bms_state.information.restTimer_10ms == 0u) {
                    /* Rest timer elapsed -> battery system at rest */
                    bms_state.information.currentFlowState = BMS_AT_REST;
                } else {
                    bms_state.information.restTimer_10ms--;
                    bms_state.information.currentFlowState = BMS_RELAXATION;
                }
            }
        }
    }
}

static CONT_TYPE_e BMS_GetFirstContactorToBeOpened(uint8_t stringNumber, BMS_CURRENT_FLOW_STATE_e flowDirection) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    /* AXIVION Routine Generic-MissingParameterAssert: flowDirection: parameter accepts all defined enums */
    CONT_TYPE_e contactorToBeOpened                     = CONT_UNDEFINED;
    CONT_CURRENT_BREAKING_DIRECTION_e breakingDirection = CONT_BIDIRECTIONAL;
    /* Required preferred opening direction dependent on the current direction */
    if (flowDirection == BMS_CHARGING) {
        breakingDirection = CONT_CHARGING_DIRECTION;
    } else {
        breakingDirection = CONT_DISCHARGING_DIRECTION;
    }
    /* Iterate over contactor array and search for wanted contactor */
    uint8_t contactor = 0u;
    for (; contactor < BS_NR_OF_CONTACTORS; contactor++) {
        /* Search for:
         * 1. contactor from requested string
         * 2. contactor mounted in preferred opening direction or is bidirectional
         * 3. is no precharge contactor */
        bool correctString           = (bool)(stringNumber == cont_contactorStates[contactor].stringIndex);
        bool inPreferredDirection    = (bool)(breakingDirection == cont_contactorStates[contactor].breakingDirection);
        bool hasNoPreferredDirection = (bool)(cont_contactorStates[contactor].breakingDirection == CONT_BIDIRECTIONAL);
        bool noPrechargeContactor    = (bool)(cont_contactorStates[contactor].type != CONT_PRECHARGE);
        if (correctString && noPrechargeContactor && (inPreferredDirection || hasNoPreferredDirection)) {
            contactorToBeOpened = cont_contactorStates[contactor].type;
            break;
        }
    }
    if (contactor == BS_NR_OF_CONTACTORS) {
        /* No contactor mounted in preferred current direction found. Select
         * the PLUS contactor found in array cont_contactorStates from the
         * passed string */
        for (contactor = 0u; contactor < BS_NR_OF_CONTACTORS; contactor++) {
            /* Search for:
             * 1. contactor from requested string
             * 2. is PLUS contactor */
            if ((stringNumber == cont_contactorStates[contactor].stringIndex) &&
                (cont_contactorStates[contactor].type == CONT_PLUS)) {
                contactorToBeOpened = cont_contactorStates[contactor].type;
                break;
            }
        }
    }
    if (contactor == BS_NR_OF_CONTACTORS) {
        /* No PLUS contactor found. Select MINUS contactor found in array
         * cont_contactorStates from the passed string */
        for (contactor = 0u; contactor < BS_NR_OF_CONTACTORS; contactor++) {
            /* Search for:
             * 1. contactor from requested string
             * 2. is PLUS contactor */
            if ((stringNumber == cont_contactorStates[contactor].stringIndex) &&
                (cont_contactorStates[contactor].type == CONT_MINUS)) {
                contactorToBeOpened = cont_contactorStates[contactor].type;
                break;
            }
        }
    }
    if (contactor == BS_NR_OF_CONTACTORS) {
        /* No PLUS or MAIN_MINUS contactor found in requested string. */
        FAS_ASSERT(FAS_TRAP);
    }
    return contactorToBeOpened;
}

static CONT_TYPE_e BMS_GetSecondContactorToBeOpened(uint8_t stringNumber, CONT_TYPE_e firstOpenedContactorType) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    FAS_ASSERT((firstOpenedContactorType != CONT_UNDEFINED) && (firstOpenedContactorType != CONT_PRECHARGE));
    CONT_TYPE_e contactorToBeOpened = CONT_UNDEFINED;
    /* Check what contactor has already been opened and select the other one */
    if (firstOpenedContactorType == CONT_PLUS) {
        contactorToBeOpened = CONT_MINUS;
    } else {
        contactorToBeOpened = CONT_PLUS;
    }
    /* Iterate over contactor array and search for wanted contactor */
    uint8_t contactor = 0u;
    for (; contactor < BS_NR_OF_CONTACTORS; contactor++) {
        /* Search for specific contactor from requested string */
        if ((stringNumber == cont_contactorStates[contactor].stringIndex) &&
            (contactorToBeOpened == cont_contactorStates[contactor].type)) {
            contactorToBeOpened = cont_contactorStates[contactor].type;
            break;
        }
    }
    if (contactor == BS_NR_OF_CONTACTORS) {
        /* No PLUS or MAIN_MINUS contactor found in requested string.
         * Apparently, only one contactor has been defined for this string */
        FAS_ASSERT(FAS_TRAP);
    }
    return contactorToBeOpened;
}

static BMS_FSM_STATES_e BMS_ProcessInitializedState(BMS_STATE_s *pBmsState) {
    FAS_ASSERT(pBmsState != NULL_PTR);

    BMS_FSM_STATES_e nextState = BMS_FSM_STATE_INITIALIZED; /* default behavior: stay in state */

    switch (pBmsState->currentSubstate) {
        case BMS_FSM_SUBSTATE_ENTRY:
            /* Nothing to do, just transfer to next substate */
            BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_INITIALIZATION_IMD, BMS_FSM_SHORTTIME);
            break;

        case BMS_FSM_SUBSTATE_INITIALIZATION_IMD:
            if (IMD_RequestInsulationMeasurement() == IMD_ILLEGAL_REQUEST) {
                /* Initialization of IMD device not finished yet -> wait until this is finished before moving on */
                BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_INITIALIZATION_IMD, BMS_FSM_LONGTIME);
            } else {
                /* Initialization of IMD device finished -> move on to exit */
                BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_INITIALIZATION_EXIT, BMS_FSM_SHORTTIME);
            }
            break;

        case BMS_FSM_SUBSTATE_INITIALIZATION_EXIT:
            /* Nothing to do, just transfer to next state */
            nextState = BMS_FSM_STATE_IDLE;
            break;

        default:                  /* LCOV_EXCL_LINE */
            FAS_ASSERT(FAS_TRAP); /* LCOV_EXCL_LINE */
            break;                /* LCOV_EXCL_LINE */
    }

    return nextState;
}

static BMS_FSM_STATES_e BMS_ProcessIdleState(BMS_STATE_s *pBmsState) {
    FAS_ASSERT(pBmsState != NULL_PTR);

    BMS_FSM_STATES_e nextState            = BMS_FSM_STATE_IDLE; /* default behavior: stay in state */
    DATA_BLOCK_SYSTEM_STATE_s systemState = {.header.uniqueId = DATA_BLOCK_ID_SYSTEM_STATE};

    switch (pBmsState->currentSubstate) {
        case BMS_FSM_SUBSTATE_ENTRY:
            /* TODO: function for setting CAN state */
            DATA_READ_DATA(&systemState);
            systemState.bmsCanState = BMS_CAN_STATE_IDLE;
            DATA_WRITE_DATA(&systemState);

            BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS, BMS_FSM_SHORTTIME);
            break;

        case BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS:
            if (BMS_IsBatterySystemStateOkay(pBmsState) == STD_NOT_OK) {
                nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR;
            } else {
                BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS, BMS_FSM_SHORTTIME);
            }
            break;

        case BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS:
            if (BMS_CheckCanRequests() == BMS_REQ_ID_STANDBY) {
                nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY;
            } else {
                BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS, BMS_FSM_SHORTTIME);
            }
            break;

        default:                  /* LCOV_EXCL_LINE */
            FAS_ASSERT(FAS_TRAP); /* LCOV_EXCL_LINE */
            break;                /* LCOV_EXCL_LINE */
    }
    return nextState;
}

static BMS_FSM_STATES_e BMS_ProcessOpenContactorsToErrorState(BMS_STATE_s *pBmsState) {
    FAS_ASSERT(pBmsState != NULL_PTR);

    BMS_FSM_STATES_e nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR; /* default behavior: stay in state */
    CONT_ELECTRICAL_STATE_TYPE_e contactorState = CONT_SWITCH_UNDEFINED;
    bool contactorFeedbackValid                 = false;

    switch (pBmsState->currentSubstate) {
        case BMS_FSM_SUBSTATE_ENTRY:
            BAL_SetStateRequest(BAL_STATE_NO_BALANCING_REQUEST);
            /* Check if the error reason is the loss of supply voltage clamp 30C */
            if (DIAG_GetDiagnosisEntryState(DIAG_ID_SUPPLY_VOLTAGE_CLAMP_30C_LOST) == STD_NOT_OK) {
                BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_HANDLE_SUPPLY_VOLTAGE_30C_LOSS, BMS_FSM_SHORTTIME);
            } else {
                BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_OPEN_ALL_PRECHARGE_CONTACTORS, BMS_FSM_SHORTTIME);
            }
            break;

        case BMS_FSM_SUBSTATE_OPEN_ALL_PRECHARGE_CONTACTORS:
            /* Precharge contactors can always be opened as the precharge
                 * resistor limits the maximum current */
            CONT_OpenAllPrechargeContactors();

            /* Regular string opening - Open one string after another,
                      * starting with highest string index */
            pBmsState->information.stringNumber = BS_NR_OF_STRINGS - 1u;
            BMS_SetSubstate(
                pBmsState, BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR, BMS_TIME_WAIT_AFTER_OPENING_PRECHARGE);
            break;

        case BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR:
            /* Precharge contactors have been opened -> start opening first string contactor */
            /* TODO: Check if precharge contactors have been opened? */
            if ((bms_tablePackValues.invalidStringCurrent[pBmsState->information.stringNumber] == 0u) &&
                (MATH_AbsInt32_t(bms_tablePackValues.stringCurrent_mA[pBmsState->information.stringNumber]) <
                 BS_MAIN_CONTACTORS_MAXIMUM_BREAK_CURRENT_mA)) {
                /* Current is below maximum break current -> open first contactor
                     * Check the mounting direction of the contactors and open the contactor that is mounted in the
                     * preferred current flow direction. Open the plus contactor first if, there is no contactor
                     * in preferred direction to the current flow to open available. This may be either because both
                     * contactors are installed in the same direction or because the contactors are bidirectional.
                     */
                const BMS_CURRENT_FLOW_STATE_e flowDirection = BMS_GetCurrentFlowDirection(
                    bms_tablePackValues.stringCurrent_mA[pBmsState->information.stringNumber]);
                pBmsState->information.contactorToBeOpened =
                    BMS_GetFirstContactorToBeOpened(pBmsState->information.stringNumber, flowDirection);
                pBmsState->information.stringToBeOpened = pBmsState->information.stringNumber;
                /* Open first contactor */
                CONT_OpenContactor(pBmsState->information.stringNumber, pBmsState->information.contactorToBeOpened);

                BMS_SetSubstate(
                    pBmsState,
                    BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR,
                    BMS_WAIT_TIME_AFTER_OPENING_STRING_CONTACTOR);
                pBmsState->information.stringOpenTimeout = BMS_STRING_OPEN_TIMEOUT;
            } else {
                /* Current is above maximum contactor break current -> contactor can not be opened */
                pBmsState->information.timeAboveContactorBreakCurrent_ms += BMS_STATEMACHINE_TASK_CYCLE_CONTEXT_MS;
                if (pBmsState->information.timeAboveContactorBreakCurrent_ms >
                    BS_MAIN_FUSE_MAXIMUM_TRIGGER_DURATION_ms) {
                    /* Fuse should have been triggered by now but apparently has not yet. Do not wait any
                         * longer. Activate ALERT mode and nevertheless start opening the contactors */
                    DIAG_Handler(DIAG_ID_ALERT_MODE, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
                    const BMS_CURRENT_FLOW_STATE_e flowDirection = BMS_GetCurrentFlowDirection(
                        bms_tablePackValues.stringCurrent_mA[pBmsState->information.stringNumber]);
                    pBmsState->information.contactorToBeOpened =
                        BMS_GetFirstContactorToBeOpened(pBmsState->information.stringNumber, flowDirection);
                    pBmsState->information.stringToBeOpened = pBmsState->information.stringNumber;
                    /* Open first contactor */
                    CONT_OpenContactor(
                        pBmsState->information.stringToBeOpened, pBmsState->information.contactorToBeOpened);
                    BMS_SetSubstate(
                        pBmsState,
                        BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR,
                        BMS_WAIT_TIME_AFTER_OPENING_STRING_CONTACTOR);
                }
            }
            break;

        case BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR:
            /* Check if first contactor has been opened correctly */
            contactorState = CONT_GetContactorState(
                pBmsState->information.stringToBeOpened, pBmsState->information.contactorToBeOpened);
            contactorFeedbackValid = BMS_IsContactorFeedbackValid(
                pBmsState->information.stringToBeOpened, pBmsState->information.contactorToBeOpened);
            /* If we want to open the contactors because of a feedback
                 * error for this contactor, the statement will never be true.
                 * Thus, also continue if a feedback error for this contactor
                 * is detected as we are not able to get a valid feedback
                 * information at this point */
            if ((contactorState == CONT_SWITCH_OFF) || (contactorFeedbackValid == false)) {
                /* First contactor opened correctly.
                     * Open second contactor. Pass first opened contactor into function */
                pBmsState->information.contactorToBeOpened = BMS_GetSecondContactorToBeOpened(
                    pBmsState->information.stringNumber, pBmsState->information.contactorToBeOpened);
                /* Open second contactor */
                CONT_OpenContactor(pBmsState->information.stringToBeOpened, pBmsState->information.contactorToBeOpened);
                BMS_SetSubstate(
                    pBmsState,
                    BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR,
                    BMS_WAIT_TIME_AFTER_OPENING_STRING_CONTACTOR);
            } else {
                /* String not opened, re-issue closing request */
                CONT_OpenContactor(pBmsState->information.stringToBeOpened, pBmsState->information.contactorToBeOpened);
                BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR, BMS_FSM_SHORTTIME);
                /* TODO: add timeout */
            }
            break;

        case BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR:
            /* Check if second contactor has been opened correctly */
            contactorState = CONT_GetContactorState(
                pBmsState->information.stringToBeOpened, pBmsState->information.contactorToBeOpened);
            contactorFeedbackValid = BMS_IsContactorFeedbackValid(
                pBmsState->information.stringToBeOpened, pBmsState->information.contactorToBeOpened);
            /* If we want to open the contactors because of a feedback
                 * error for this contactor, the statement will never be true.
                 * Thus, also continue if a feedback error for this contactor
                 * is detected as we are not able to get a valid feedback
                 * information at this point */
            if ((contactorState == CONT_SWITCH_OFF) || (contactorFeedbackValid == false)) {
                /* Opening for this string finished. Reset currentState variables used for opening */
                pBmsState->information.contactorToBeOpened = CONT_UNDEFINED;
                pBmsState->information.stringToBeOpened    = 0u;
                /* String opened. Decrement string counter */
                if (pBmsState->information.numberOfClosedStrings > 0u) {
                    pBmsState->information.numberOfClosedStrings--;
                }
                pBmsState->information.closedStrings[pBmsState->information.stringNumber] = false;
                if (pBmsState->information.stringNumber > 0u) {
                    /* Not all strings opened yet -> open next string */
                    pBmsState->information.stringNumber--;
                    BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR, BMS_FSM_SHORTTIME);
                } else {
                    /* All strings opened -> prepare to leave currentState BMS_FSM_OPEN_CONTACTORS */
                    BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_OPEN_STRINGS_EXIT, BMS_FSM_SHORTTIME);
                }
            } else if (pBmsState->information.stringOpenTimeout == 0u) {
                /* String takes too long to open, go to next string */
                BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR, BMS_FSM_SHORTTIME);
            } else {
                /* String not opened, re-issue closing request */
                CONT_OpenContactor(pBmsState->information.stringToBeOpened, pBmsState->information.contactorToBeOpened);
                BMS_SetSubstate(pBmsState, pBmsState->currentSubstate, BMS_FSM_SHORTTIME);
            }
            break;

        case BMS_FSM_SUBSTATE_HANDLE_SUPPLY_VOLTAGE_30C_LOSS:
            CONT_OpenAllContactors();
            SPS_SwitchOffAllGeneralIoChannels();
            BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_OPEN_STRINGS_EXIT, BMS_FSM_SHORTTIME);
            break;
        case BMS_FSM_SUBSTATE_OPEN_STRINGS_EXIT:
            /* Opening due to detected error -> switch to BMS_FSM_STATE_ERROR */
            nextState = BMS_FSM_STATE_ERROR;
            break;

        default:                  /* LCOV_EXCL_LINE */
            FAS_ASSERT(FAS_TRAP); /* LCOV_EXCL_LINE */
            break;                /* LCOV_EXCL_LINE */
    }

    return nextState;
}

static BMS_FSM_STATES_e BMS_ProcessOpenContactorsToStandbyState(BMS_STATE_s *pBmsState) {
    FAS_ASSERT(pBmsState != NULL_PTR);

    BMS_FSM_STATES_e nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY; /* default behavior: stay in state */
    CONT_ELECTRICAL_STATE_TYPE_e contactorState = CONT_SWITCH_UNDEFINED;
    bool contactorFeedbackValid                 = false;

    switch (pBmsState->currentSubstate) {
        case BMS_FSM_SUBSTATE_ENTRY:
            BAL_SetStateRequest(BAL_STATE_NO_BALANCING_REQUEST);
            BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_OPEN_ALL_PRECHARGE_CONTACTORS, BMS_FSM_SHORTTIME);
            break;

        case BMS_FSM_SUBSTATE_OPEN_ALL_PRECHARGE_CONTACTORS:
            /* Precharge contactors can always be opened as the precharge
                 * resistor limits the maximum current */
            CONT_OpenAllPrechargeContactors();

            /* Regular string opening - Open one string after another,
                      * starting with highest string index */
            pBmsState->information.stringNumber = BS_NR_OF_STRINGS - 1u;
            BMS_SetSubstate(
                pBmsState, BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR, BMS_TIME_WAIT_AFTER_OPENING_PRECHARGE);

            break;

        case BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR:
            /* Precharge contactors have been opened -> start opening first string contactor */
            /* TODO: Check if precharge contactors have been opened? */
            if ((bms_tablePackValues.invalidStringCurrent[pBmsState->information.stringNumber] == 0u) &&
                (MATH_AbsInt32_t(bms_tablePackValues.stringCurrent_mA[pBmsState->information.stringNumber]) <
                 BS_MAIN_CONTACTORS_MAXIMUM_BREAK_CURRENT_mA)) {
                /* Current is below maximum break current -> open first contactor
                     * Check the mounting direction of the contactors and open the contactor that is mounted in the
                     * preferred current flow direction. Open the plus contactor first if, there is no contactor
                     * in preferred direction to the current flow to open available. This may be either because both
                     * contactors are installed in the same direction or because the contactors are bidirectional.
                     */
                const BMS_CURRENT_FLOW_STATE_e flowDirection = BMS_GetCurrentFlowDirection(
                    bms_tablePackValues.stringCurrent_mA[pBmsState->information.stringNumber]);
                pBmsState->information.contactorToBeOpened =
                    BMS_GetFirstContactorToBeOpened(pBmsState->information.stringNumber, flowDirection);
                pBmsState->information.stringToBeOpened = pBmsState->information.stringNumber;
                /* Open first contactor */
                CONT_OpenContactor(pBmsState->information.stringNumber, pBmsState->information.contactorToBeOpened);
                BMS_SetSubstate(
                    pBmsState,
                    BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR,
                    BMS_WAIT_TIME_AFTER_OPENING_STRING_CONTACTOR);
                pBmsState->information.stringOpenTimeout = BMS_STRING_OPEN_TIMEOUT;

            } else {
                /* Current is above maximum contactor break current -> contactor can not be opened */
                pBmsState->information.timeAboveContactorBreakCurrent_ms += BMS_STATEMACHINE_TASK_CYCLE_CONTEXT_MS;
                if (pBmsState->information.timeAboveContactorBreakCurrent_ms >
                    BS_MAIN_FUSE_MAXIMUM_TRIGGER_DURATION_ms) {
                    /* Fuse should have been triggered by now but apparently has not yet. Do not wait any
                         * longer. Activate ALERT mode and nevertheless start opening the contactors */
                    DIAG_Handler(DIAG_ID_ALERT_MODE, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
                    const BMS_CURRENT_FLOW_STATE_e flowDirection = BMS_GetCurrentFlowDirection(
                        bms_tablePackValues.stringCurrent_mA[pBmsState->information.stringNumber]);
                    pBmsState->information.contactorToBeOpened =
                        BMS_GetFirstContactorToBeOpened(pBmsState->information.stringNumber, flowDirection);
                    pBmsState->information.stringToBeOpened = pBmsState->information.stringNumber;
                    /* Open first contactor */
                    CONT_OpenContactor(
                        pBmsState->information.stringToBeOpened, pBmsState->information.contactorToBeOpened);
                    BMS_SetSubstate(
                        pBmsState,
                        BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR,
                        BMS_WAIT_TIME_AFTER_OPENING_STRING_CONTACTOR);
                }
            }
            break;

        case BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR:
            /* Check if first contactor has been opened correctly */
            contactorState = CONT_GetContactorState(
                pBmsState->information.stringToBeOpened, pBmsState->information.contactorToBeOpened);
            contactorFeedbackValid = BMS_IsContactorFeedbackValid(
                pBmsState->information.stringToBeOpened, pBmsState->information.contactorToBeOpened);
            /* If we want to open the contactors because of a feedback
                 * error for this contactor, the statement will never be true.
                 * Thus, also continue if a feedback error for this contactor
                 * is detected as we are not able to get a valid feedback
                 * information at this point */
            if ((contactorState == CONT_SWITCH_OFF) || (contactorFeedbackValid == false)) {
                /* First contactor opened correctly.
                     * Open second contactor. Pass first opened contactor into function */
                pBmsState->information.contactorToBeOpened = BMS_GetSecondContactorToBeOpened(
                    pBmsState->information.stringNumber, pBmsState->information.contactorToBeOpened);
                /* Open second contactor */
                CONT_OpenContactor(pBmsState->information.stringToBeOpened, pBmsState->information.contactorToBeOpened);
                BMS_SetSubstate(
                    pBmsState,
                    BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR,
                    BMS_WAIT_TIME_AFTER_OPENING_STRING_CONTACTOR);
            } else {
                /* String not opened, re-issue closing request */
                CONT_OpenContactor(pBmsState->information.stringToBeOpened, pBmsState->information.contactorToBeOpened);
                BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR, BMS_FSM_SHORTTIME);
                /* TODO: add timeout */
            }
            break;

        case BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR:
            /* Check if second contactor has been opened correctly */
            contactorState = CONT_GetContactorState(
                pBmsState->information.stringToBeOpened, pBmsState->information.contactorToBeOpened);
            contactorFeedbackValid = BMS_IsContactorFeedbackValid(
                pBmsState->information.stringToBeOpened, pBmsState->information.contactorToBeOpened);
            /* If we want to open the contactors because of a feedback
                 * error for this contactor, the statement will never be true.
                 * Thus, also continue if a feedback error for this contactor
                 * is detected as we are not able to get a valid feedback
                 * information at this point */
            if ((contactorState == CONT_SWITCH_OFF) || (contactorFeedbackValid == false)) {
                /* Opening for this string finished. Reset currentState variables used for opening */
                pBmsState->information.contactorToBeOpened = CONT_UNDEFINED;
                pBmsState->information.stringToBeOpened    = 0u;
                /* String opened. Decrement string counter */
                if (pBmsState->information.numberOfClosedStrings > 0u) {
                    pBmsState->information.numberOfClosedStrings--;
                }
                pBmsState->information.closedStrings[pBmsState->information.stringNumber] = false;
                if (pBmsState->information.stringNumber > 0u) {
                    /* Not all strings opened yet -> open next string */
                    pBmsState->information.stringNumber--;
                    BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR, BMS_FSM_SHORTTIME);
                } else {
                    /* All strings opened -> prepare to leave currentState BMS_FSM_OPEN_CONTACTORS */
                    BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_OPEN_STRINGS_EXIT, BMS_FSM_SHORTTIME);
                }
            } else if (pBmsState->information.stringOpenTimeout == 0u) {
                /* String takes too long to open, go to next string */
                BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR, BMS_FSM_SHORTTIME);
            } else {
                /* String not opened, re-issue closing request */
                CONT_OpenContactor(pBmsState->information.stringToBeOpened, pBmsState->information.contactorToBeOpened);
                BMS_SetSubstate(pBmsState, pBmsState->currentSubstate, BMS_FSM_SHORTTIME);
            }
            break;

        case BMS_FSM_SUBSTATE_OPEN_STRINGS_EXIT:
            /* Opening due to standby request -> switch to BMS_FSM_STATE_STANDBY */
            nextState = BMS_FSM_STATE_STANDBY;
            break;

        default:                  /* LCOV_EXCL_LINE */
            FAS_ASSERT(FAS_TRAP); /* LCOV_EXCL_LINE */
            break;                /* LCOV_EXCL_LINE */
    }

    return nextState;
}

static BMS_FSM_STATES_e BMS_ProcessStandbyState(BMS_STATE_s *pBmsState) {
    FAS_ASSERT(pBmsState != NULL_PTR);

    BMS_FSM_STATES_e nextState            = BMS_FSM_STATE_STANDBY; /* default behavior: stay in state */
    DATA_BLOCK_SYSTEM_STATE_s systemState = {.header.uniqueId = DATA_BLOCK_ID_SYSTEM_STATE};

    switch (pBmsState->currentSubstate) {
        case BMS_FSM_SUBSTATE_ENTRY:
            BAL_SetStateRequest(BAL_STATE_ALLOW_BALANCING_REQUEST);
            BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_INTERLOCK, BMS_FSM_MEDIUM_TIME);
            DATA_READ_DATA(&systemState);
            systemState.bmsCanState = BMS_CAN_STATE_STANDBY;
            DATA_WRITE_DATA(&systemState);
            break;

        case BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_INTERLOCK:
            if (BMS_IsBatterySystemStateOkay(pBmsState) == STD_NOT_OK) {
                nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR;
            } else {
                BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_INTERLOCK_CHECKED, BMS_FSM_SHORTTIME);
            }
            break;

        case BMS_FSM_SUBSTATE_INTERLOCK_CHECKED:
            BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS, BMS_FSM_SHORTTIME);
            break;

        case BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS:
            if (BMS_IsBatterySystemStateOkay(pBmsState) == STD_NOT_OK) {
                nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR;
            } else {
                BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS, BMS_FSM_SHORTTIME);
            }
            break;

        case BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS:
            if (BMS_CheckCanRequests() == BMS_REQ_ID_NORMAL) {
                pBmsState->information.powerPath = BMS_POWER_PATH_0; /* Discharging */
                nextState                        = BMS_FSM_STATE_PRECHARGE;
            } else if (BMS_CheckCanRequests() == BMS_REQ_ID_CHARGE) {
                pBmsState->information.powerPath = BMS_POWER_PATH_1;
                nextState                        = BMS_FSM_STATE_PRECHARGE;
            } else {
                BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS, BMS_FSM_SHORTTIME);
            }
            break;

        default:                  /* LCOV_EXCL_LINE */
            FAS_ASSERT(FAS_TRAP); /* LCOV_EXCL_LINE */
            break;                /* LCOV_EXCL_LINE */
    }

    return nextState;
}

static BMS_FSM_STATES_e BMS_ProcessPrechargeState(BMS_STATE_s *pBmsState) {
    FAS_ASSERT(pBmsState != NULL_PTR);

    BMS_FSM_STATES_e nextState                     = BMS_FSM_STATE_PRECHARGE; /* default behavior: stay in state */
    CONT_ELECTRICAL_STATE_TYPE_e contactorState    = CONT_SWITCH_UNDEFINED;
    DATA_BLOCK_SYSTEM_STATE_s systemState          = {.header.uniqueId = DATA_BLOCK_ID_SYSTEM_STATE};
    BMS_RESULT_PRECHARGE_PROCESS_e prechargeRetval = BMS_PRECHARING_HAS_NOT_STARTED;
    STD_RETURN_TYPE_e contRetVal                   = STD_NOT_OK;

    switch (pBmsState->currentSubstate) {
        case BMS_FSM_SUBSTATE_ENTRY:
            DATA_READ_DATA(&systemState);
            systemState.bmsCanState = BMS_CAN_STATE_PRECHARGE;
            DATA_WRITE_DATA(&systemState);
            /*BMS_POWER_PATH_1 is set on charge*/
            if (pBmsState->information.powerPath == BMS_POWER_PATH_1) {
                pBmsState->information.stringNumber =
                    BMS_GetLowestString(BMS_TAKE_PRECHARGE_INTO_ACCOUNT, &bms_tablePackValues);
            } else {
                pBmsState->information.stringNumber =
                    BMS_GetHighestString(BMS_TAKE_PRECHARGE_INTO_ACCOUNT, &bms_tablePackValues);
            }
            if (pBmsState->information.stringNumber == BMS_NO_STRING_AVAILABLE) {
                nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR;
            } else {
                pBmsState->information.prechargeTryCounter = 0u;
                pBmsState->information.firstClosedString   = pBmsState->information.stringNumber;
                if (pBmsState->information.OscillationTimeout == 0u) {
                    /* Close MINUS string contactor */
                    if (CONT_CloseContactor(pBmsState->information.firstClosedString, CONT_MINUS) == STD_OK) {
                        pBmsState->information.stringCloseTimeout = BMS_STRING_CLOSE_TIMEOUT;
                        BMS_SetSubstate(
                            pBmsState,
                            BMS_FSM_SUBSTATE_PRECHARGE_CLOSE_PRECHARGE,
                            BMS_WAIT_TIME_AFTER_CLOSING_STRING_CONTACTOR);
                    } else {
                        /* Invalid contactor requested */
                        nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR;
                    }
                } else if (BMS_IsBatterySystemStateOkay(pBmsState) == STD_NOT_OK) {
                    /* If precharge re-enter timeout not elapsed, wait (and check errors while waiting) */
                    nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR;
                } else {
                    ;
                }
            }
            break;

        case BMS_FSM_SUBSTATE_PRECHARGE_CLOSE_PRECHARGE:
            /* Check if MINUS contactor has been successfully closed */
            contactorState = CONT_GetContactorState(pBmsState->information.firstClosedString, CONT_MINUS);
            if (contactorState == CONT_SWITCH_ON) {
                pBmsState->information.OscillationTimeout = BMS_OSCILLATION_TIMEOUT;
                contRetVal = CONT_ClosePrecharge(pBmsState->information.firstClosedString);
                pBmsState->information.closedPrechargeContactors[pBmsState->information.stringNumber] = true;
                if (contRetVal == STD_OK) {
                    /* Minus Contactor closed successfully and request to close precharge contactor sent
                             * -> save timestamp and monitor precharging process */
                    pBmsState->information.startOfPrecharging = pBmsState->information.currentSystick;
                    BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_PRECHARGE_CHECK_PRECHARGE_PROCESS, BMS_FSM_SHORTTIME);
                } else {
                    nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR;
                }
            } else if (pBmsState->information.stringCloseTimeout == 0u) {
                /* String takes too long to close */
                nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR;
            } else {
                /* String not closed, re-issue closing request */
                CONT_CloseContactor(pBmsState->information.firstClosedString, CONT_MINUS);
                BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_PRECHARGE_CLOSE_PRECHARGE, BMS_FSM_SHORTTIME);
            }
            break;

        case BMS_FSM_SUBSTATE_PRECHARGE_CHECK_PRECHARGE_PROCESS:
            if (BMS_IsBatterySystemStateOkay(pBmsState) == STD_NOT_OK) {
                /* Error detected: abort and do no proceed with monitoring precharge process */
                nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR;
            } else if (BMS_CheckCanRequests() == BMS_REQ_ID_STANDBY) {
                /* Contactor open request received: abort here and do no
                         * proceed with monitoring precharge process */
                nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY;
            } else {
                contactorState = CONT_GetContactorState(pBmsState->information.firstClosedString, CONT_PRECHARGE);
                /* Monitor precharging */
                prechargeRetval = BMS_MonitorPrechargeProcess(
                    pBmsState,
                    pBmsState->information.firstClosedString,
                    &bms_tablePackValues,
                    BMS_PRECHARGE_MONITORING_PARAMETERS,
                    BMS_MAXIMUM_PRECHARGE_DURATION_ms);
                /* Check if precharge contactor is closed and precharge is finished */
                if ((contactorState == CONT_SWITCH_ON) && (prechargeRetval == BMS_PRECHARGING_FINISHED)) {
                    /* Successfully precharged. Close string PLUS contactor */
                    CONT_CloseContactor(pBmsState->information.firstClosedString, CONT_PLUS);
                    pBmsState->information.stringCloseTimeout = BMS_STRING_CLOSE_TIMEOUT;
                    BMS_SetSubstate(
                        pBmsState,
                        BMS_FSM_SUBSTATE_CHECK_CLOSE_SECOND_STRING_CONTACTOR_PRECHARGE_STATE,
                        BMS_WAIT_TIME_AFTER_CLOSING_STRING_CONTACTOR);
                } else if (prechargeRetval == BMS_PRECHARGING_ONGOING) {
                    /* Stay in this currentState until precharging is successful
                             * or timeout reached */
                    BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_PRECHARGE_CHECK_PRECHARGE_PROCESS, BMS_FSM_SHORTTIME);
                } else {
                    /* Precharging failed. Timeout reached. Open precharge contactor. */
                    contRetVal = CONT_OpenPrecharge(pBmsState->information.firstClosedString);
                    /* Check if retry limit has been reached */
                    if (pBmsState->information.prechargeTryCounter < (BMS_PRECHARGE_TRIES - 1u)) {
                        pBmsState->information.closedPrechargeContactors[pBmsState->information.stringNumber] = false;
                        if (contRetVal == STD_OK) {
                            BMS_SetSubstate(
                                pBmsState,
                                BMS_FSM_SUBSTATE_PRECHARGE_CLOSE_PRECHARGE,
                                BMS_TIME_WAIT_AFTER_PRECHARGE_FAIL);
                            pBmsState->information.prechargeTryCounter++;
                        } else {
                            nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR;
                        }
                    } else {
                        pBmsState->information.closedPrechargeContactors[pBmsState->information.stringNumber] = false;
                        nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR;
                    }
                }
            }
            break;

        case BMS_FSM_SUBSTATE_CHECK_CLOSE_SECOND_STRING_CONTACTOR_PRECHARGE_STATE:
            contactorState = CONT_GetContactorState(pBmsState->information.firstClosedString, CONT_PLUS);
            if (contactorState == CONT_SWITCH_ON) {
                pBmsState->information.closedStrings[pBmsState->information.firstClosedString] = true;
                pBmsState->information.numberOfClosedStrings++;
                BMS_SetSubstate(
                    pBmsState,
                    BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_PRECHARGE_CLOSING_STRINGS,
                    BMS_WAIT_TIME_AFTER_CLOSING_STRING_CONTACTOR);
            } else if (pBmsState->information.stringCloseTimeout == 0u) {
                /* String takes too long to close */
                nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR;
            } else {
                /* String not closed, re-issue closing request */
                CONT_CloseContactor(pBmsState->information.firstClosedString, CONT_PLUS);
                BMS_SetSubstate(
                    pBmsState, BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_PRECHARGE_FIRST_STRING, BMS_FSM_SHORTTIME);
            }
            break;

        case BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_PRECHARGE_FIRST_STRING:
            if (BMS_IsBatterySystemStateOkay(pBmsState) == STD_NOT_OK) {
                nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR;
            } else {
                BMS_SetSubstate(
                    pBmsState, BMS_FSM_SUBSTATE_CHECK_CLOSE_SECOND_STRING_CONTACTOR_PRECHARGE_STATE, BMS_FSM_SHORTTIME);
            }
            break;

        case BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_PRECHARGE_CLOSING_STRINGS:
            /* Always make one error check after the first string was closed successfully */
            if (BMS_IsBatterySystemStateOkay(pBmsState) == STD_NOT_OK) {
                nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR;
            } else {
                BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_PRECHARGE_OPEN_PRECHARGE, BMS_FSM_SHORTTIME);
            }
            break;

        case BMS_FSM_SUBSTATE_PRECHARGE_OPEN_PRECHARGE:
            contRetVal = CONT_OpenPrecharge(pBmsState->information.firstClosedString);
            if (contRetVal == STD_OK) {
                pBmsState->information.closedPrechargeContactors[pBmsState->information.stringNumber] = false;
                BMS_SetSubstate(
                    pBmsState, BMS_FSM_SUBSTATE_PRECHARGE_CHECK_OPEN_PRECHARGE, BMS_TIME_WAIT_AFTER_OPENING_PRECHARGE);
            } else {
                nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR;
            }
            break;

        case BMS_FSM_SUBSTATE_PRECHARGE_CHECK_OPEN_PRECHARGE:
            contactorState = CONT_GetContactorState(pBmsState->information.firstClosedString, CONT_PRECHARGE);
            if (contactorState == CONT_SWITCH_OFF) {
                nextState = BMS_FSM_STATE_NORMAL;
            } else if (pBmsState->information.stringCloseTimeout == 0u) {
                /* Precharge contactor takes too long to open */
                nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR;
            } else {
                /* Precharge contactor not opened, re-issue open request */
                CONT_OpenPrecharge(pBmsState->information.firstClosedString);
                BMS_SetSubstate(
                    pBmsState, BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_PRECHARGE_FIRST_STRING, BMS_FSM_SHORTTIME);
            }
            break;

        default:                  /* LCOV_EXCL_LINE */
            FAS_ASSERT(FAS_TRAP); /* LCOV_EXCL_LINE */
            break;                /* LCOV_EXCL_LINE */
    }

    return nextState;
}

static BMS_FSM_STATES_e BMS_ProcessNormalState(BMS_STATE_s *pBmsState) {
    FAS_ASSERT(pBmsState != NULL_PTR);

    BMS_FSM_STATES_e nextState                  = BMS_FSM_STATE_NORMAL; /* default behavior: stay in state */
    DATA_BLOCK_SYSTEM_STATE_s systemState       = {.header.uniqueId = DATA_BLOCK_ID_SYSTEM_STATE};
    CONT_ELECTRICAL_STATE_TYPE_e contactorState = CONT_SWITCH_UNDEFINED;

    switch (pBmsState->currentSubstate) {
        case BMS_FSM_SUBSTATE_ENTRY:
            DATA_READ_DATA(&systemState);
            if (pBmsState->information.powerPath == BMS_POWER_PATH_0) {
                /* TODO: Now the CAN state is dependent on the power path. However,
            this is not explicitly linked to charging or discharging. Needs to be clarified. */
                systemState.bmsCanState = BMS_CAN_STATE_NORMAL;
            } else {
                systemState.bmsCanState = BMS_CAN_STATE_CHARGE;
            }
            DATA_WRITE_DATA(&systemState);
            BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS, BMS_FSM_SHORTTIME);
            pBmsState->information.nextStringClosedTimer = 0u;
            break;

        case BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS:
            if (BMS_IsBatterySystemStateOkay(pBmsState) == STD_NOT_OK) {
                nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR;
            } else {
                BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS, BMS_FSM_SHORTTIME);
            }
            break;

        case BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS:
            if (BMS_CheckCanRequests() == BMS_REQ_ID_STANDBY) {
                nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY;
            } else {
                BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_NORMAL_CLOSE_NEXT_STRING, BMS_FSM_SHORTTIME);
            }
            break;

        case BMS_FSM_SUBSTATE_NORMAL_CLOSE_NEXT_STRING:
            if (pBmsState->information.nextStringClosedTimer == 0u) {
                /* Time to close the next string */
                pBmsState->information.nextStringNumber =
                    BMS_GetClosestString(BMS_DO_NOT_TAKE_PRECHARGE_INTO_ACCOUNT, &bms_tablePackValues);
                if (pBmsState->information.nextStringNumber == BMS_NO_STRING_AVAILABLE) {
                    BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS, BMS_FSM_SHORTTIME);
                } else if ((BMS_GetStringVoltageDifference(
                                pBmsState->information.nextStringNumber, &bms_tablePackValues) <=
                            BMS_NEXT_STRING_VOLTAGE_LIMIT_MV) &&
                           (BMS_GetAverageStringCurrent(&bms_tablePackValues) <= BMS_AVERAGE_STRING_CURRENT_LIMIT_MA)) {
                    /* Voltage/current conditions suitable to close a further string. Close first string contactor
                             */
                    CONT_CloseContactor(pBmsState->information.nextStringNumber, CONT_MINUS);
                    pBmsState->information.stringCloseTimeout = BMS_STRING_CLOSE_TIMEOUT;
                    BMS_SetSubstate(
                        pBmsState,
                        BMS_FSM_SUBSTATE_NORMAL_CLOSE_SECOND_STRING_CONTACTOR,
                        BMS_WAIT_TIME_AFTER_CLOSING_STRING_CONTACTOR);
                }
            } else {
                BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS, BMS_FSM_SHORTTIME);
            }
            break;

        case BMS_FSM_SUBSTATE_NORMAL_CLOSE_SECOND_STRING_CONTACTOR:
            contactorState = CONT_GetContactorState(pBmsState->information.nextStringNumber, CONT_MINUS);
            if (contactorState == CONT_SWITCH_ON) {
                /* First string contactor closed. Close second string contactor */
                CONT_CloseContactor(pBmsState->information.nextStringNumber, CONT_PLUS);
                BMS_SetSubstate(
                    pBmsState,
                    BMS_FSM_SUBSTATE_NORMAL_CHECK_STRING_CLOSED,
                    BMS_WAIT_TIME_AFTER_CLOSING_STRING_CONTACTOR);
            } else if (pBmsState->information.stringCloseTimeout == 0u) {
                /* String takes too long to close */
                nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR;
            } else {
                /* String minus contactor has not been closed successfully. Re-trigger closing */
                CONT_CloseContactor(pBmsState->information.nextStringNumber, CONT_MINUS);
                BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_NORMAL_CLOSE_SECOND_STRING_CONTACTOR, BMS_FSM_SHORTTIME);
            }
            break;

        case BMS_FSM_SUBSTATE_NORMAL_CHECK_STRING_CLOSED:
            contactorState = CONT_GetContactorState(pBmsState->information.nextStringNumber, CONT_PLUS);
            if (contactorState == CONT_SWITCH_ON) {
                pBmsState->information.numberOfClosedStrings++;
                pBmsState->information.closedStrings[pBmsState->information.nextStringNumber] = true;
                pBmsState->information.nextStringClosedTimer = BMS_WAIT_TIME_BETWEEN_CLOSING_STRINGS;
                /* Go to begin of NORMAL case to redo the full procedure with error check and request check */
                BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS, BMS_FSM_SHORTTIME);
            } else if (pBmsState->information.stringCloseTimeout == 0u) {
                /* String takes too long to close */
                nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR;
            } else if (BMS_IsBatterySystemStateOkay(pBmsState) == STD_NOT_OK) {
                nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR;
            } else if (BMS_CheckCanRequests() == BMS_REQ_ID_STANDBY) {
                nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY;
            } else {
                /* String not closed, re-issue closing request */
                CONT_CloseContactor(pBmsState->information.nextStringNumber, CONT_PLUS);
                BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_NORMAL_CHECK_STRING_CLOSED, BMS_FSM_SHORTTIME);
            }
            break;

        default:                  /* LCOV_EXCL_LINE */
            FAS_ASSERT(FAS_TRAP); /* LCOV_EXCL_LINE */
            break;                /* LCOV_EXCL_LINE */
    }

    return nextState;
}

static BMS_FSM_STATES_e BMS_ProcessErrorState(BMS_STATE_s *pBmsState) {
    FAS_ASSERT(pBmsState != NULL_PTR);

    BMS_FSM_STATES_e nextState            = BMS_FSM_STATE_ERROR; /* default behavior: stay in state */
    DATA_BLOCK_SYSTEM_STATE_s systemState = {.header.uniqueId = DATA_BLOCK_ID_SYSTEM_STATE};

    switch (pBmsState->currentSubstate) {
        case BMS_FSM_SUBSTATE_ENTRY:
            /* Set BMS System currentState to error */
            DATA_READ_DATA(&systemState);
            systemState.bmsCanState = BMS_CAN_STATE_ERROR;
            DATA_WRITE_DATA(&systemState);
            /* Deactivate balancing */
            BAL_SetStateRequest(BAL_STATE_NO_BALANCING_REQUEST);
            /* Change LED toggle frequency to indicate an error */
            LED_SetToggleTime(LED_ERROR_OPERATION_ON_OFF_TIME_ms);
            /* Switch to next currentSubstate */
            BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS, BMS_FSM_SHORTTIME);
            break;

        case BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS:
            if (DIAG_IsAnyFatalErrorSet() == false) {
                /* No error detected anymore - reset fatal error related variables */
                pBmsState->information.minimumActiveDelay_ms  = BMS_NO_ACTIVE_DELAY_TIME_ms;
                pBmsState->information.transitionToErrorState = false;
                /* Check for STANDBY request */
                BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS, BMS_FSM_SHORTTIME);
            }
            break;

        case BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS:
            if (BMS_CheckCanRequests() == BMS_REQ_ID_STANDBY) {
                /* Activate balancing again */
                BAL_SetStateRequest(BAL_STATE_ALLOW_BALANCING_REQUEST);
                /* Set LED frequency to normal operation as we leave error
                       currentState subsequently */
                LED_SetToggleTime(LED_NORMAL_OPERATION_ON_OFF_TIME_ms);

                /* Verify that all contactors are opened and switch to
                     * STANDBY currentState afterwards */
                nextState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY;
            } else {
                BMS_SetSubstate(pBmsState, BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS, BMS_FSM_SHORTTIME);
            }
            break;

        default:                  /* LCOV_EXCL_LINE */
            FAS_ASSERT(FAS_TRAP); /* LCOV_EXCL_LINE */
            break;                /* LCOV_EXCL_LINE */
    }
    return nextState;
}

static STD_RETURN_TYPE_e BMS_RunStateMachine(BMS_STATE_s *pBmsState) {
    BMS_STATE_REQUEST_e statereq      = BMS_STATE_NO_REQUEST;
    STD_RETURN_TYPE_e ranStateMachine = STD_OK;
    BMS_FSM_STATES_e nextState        = BMS_FSM_STATE_DUMMY;

    /****Happens every time the state machine is triggered**************/
    switch (pBmsState->currentState) {
            /********************************************** STATE: HAS NEVER RUN */
        case BMS_FSM_STATE_HAS_NEVER_RUN:
            /* Options:
             * (1) Initial value, just transfer into the entry state
             */
            BMS_SetState(pBmsState, BMS_FSM_STATE_UNINITIALIZED, BMS_FSM_SUBSTATE_ENTRY, BMS_FSM_SHORTTIME);
            break;
        /****************************UNINITIALIZED****************************/
        case BMS_FSM_STATE_UNINITIALIZED:
            /* Options:
             * (1) Stay in this main state
             * (2) transition to next allowed/defined state(s):
             *     - BMS_FSM_STATE_INITIALIZATION
             * (3) invalid main state requested to transition from this state
             *     to --> assert
             */

            /* waiting for Initialization Request */
            nextState = BMS_FSM_STATE_UNINITIALIZED;
            statereq  = BMS_TransferStateRequest();

            /* Determine next state based on state request */
            if (statereq == BMS_STATE_INITIALIZATION_REQUEST) {
                nextState = BMS_FSM_STATE_INITIALIZATION;
            } else if (statereq == BMS_STATE_NO_REQUEST) {
                /* no actual request pending */
            } else {
                pBmsState->information.ErrRequestCounter++; /* illegal request pending */
            }

            if (nextState == BMS_FSM_STATE_UNINITIALIZED) {
                /* staying in state, processed by state function */
            } else if (nextState == BMS_FSM_STATE_INITIALIZATION) {
                BMS_SetState(pBmsState, BMS_FSM_STATE_INITIALIZATION, BMS_FSM_SUBSTATE_ENTRY, BMS_FSM_SHORTTIME);
            } else {
                FAS_ASSERT(FAS_TRAP); /* invalid state transition requested */
            }
            break;

        /****************************INITIALIZATION***************************/
        case BMS_FSM_STATE_INITIALIZATION:
            /* TODO: Align the roles of `BMS_FSM_STATE_INITIALIZATION` and `BMS_FSM_STATE_INITIALIZED` with the
            documented naming semantics. In BMS, `BMS_FSM_STATE_INITIALIZATION` immediately sets initialization
            complete and transitions, while `BMS_FSM_STATE_INITIALIZED` still runs initialization substates such as
            `BMS_FSM_SUBSTATE_INITIALIZATION_IMD` and `BMS_FSM_SUBSTATE_INITIALIZATION_EXIT`.*/
            /* Options:
             * (1) transition to next allowed/defined state(s):
             *     - BMS_FSM_STATE_INITIALIZED
             */

            /* Reset ALERT mode flag */
            DIAG_Handler(DIAG_ID_ALERT_MODE, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
            pBmsState->information.initFinished = STD_OK;

            BMS_SetState(pBmsState, BMS_FSM_STATE_INITIALIZED, BMS_FSM_SUBSTATE_ENTRY, BMS_FSM_SHORTTIME);
            break;

        /****************************INITIALIZED******************************/
        case BMS_FSM_STATE_INITIALIZED:
            /* Options:
             * (1) stay in this main state,
             * (2) transition to next allowed/defined state(s):
             *     - BMS_FSM_STATE_IDLE
             * (3) invalid main state requested to transition from this state
             *     to --> assert
             */
            nextState = BMS_ProcessInitializedState(pBmsState);

            if (nextState == BMS_FSM_STATE_INITIALIZED) {
                /* staying in state, processed by state function */
            } else if (nextState == BMS_FSM_STATE_IDLE) {
                BMS_SetState(pBmsState, BMS_FSM_STATE_IDLE, BMS_FSM_SUBSTATE_ENTRY, BMS_FSM_SHORTTIME);
            } else {
                FAS_ASSERT(FAS_TRAP); /* invalid state transition requested */
            }
            break;

        /****************************IDLE*************************************/
        case BMS_FSM_STATE_IDLE:
            /* Options:
             * (1) stay in this main state,
             * (2) transition to error state via open contactors to error state
             * (3) transition to next allowed/defined state(s):
             *     - BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY
             * (4) invalid main state requested to transition from this state
             *     to --> assert
             */
            nextState = BMS_ProcessIdleState(pBmsState);
            if (nextState == BMS_FSM_STATE_IDLE) {
                /* staying in state, processed by state function */
            } else if (nextState == BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR) {
                BMS_SetState(
                    pBmsState, BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, BMS_FSM_SUBSTATE_ENTRY, BMS_FSM_SHORTTIME);
            } else if (nextState == BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY) {
                BMS_SetState(
                    pBmsState, BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY, BMS_FSM_SUBSTATE_ENTRY, BMS_FSM_SHORTTIME);
            } else {
                FAS_ASSERT(FAS_TRAP); /* invalid state transition requested */
            }

            break;

        /****************************OPEN CONTACTORS TO STANDBY***************/
        case BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY:
            /* Options:
             * (1) stay in this main state,
             * (3) transition to next allowed/defined state(s):
             *     - BMS_FSM_STATE_STANDBY
             * (4) invalid main state requested to transition from this state
             *     to --> assert
             */

            nextState = BMS_ProcessOpenContactorsToStandbyState(pBmsState);
            if (nextState == BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY) {
                /* staying in state, processed by state function */
            } else if (nextState == BMS_FSM_STATE_STANDBY) {
                BMS_SetState(pBmsState, BMS_FSM_STATE_STANDBY, BMS_FSM_SUBSTATE_ENTRY, BMS_FSM_SHORTTIME);
            } else {
                FAS_ASSERT(FAS_TRAP); /* invalid state transition requested */
            }
            break;

        /****************************STANDBY**********************************/
        case BMS_FSM_STATE_STANDBY:
            /* Options:
             * (1) stay in this main state,
             * (2) transition to error state via open contactors to error state
             * (3) transition to next allowed/defined state(s):
             *     - BMS_FSM_STATE_PRECHARGE
             * (4) invalid main state requested to transition from this state
             *     to --> assert
             */
            nextState = BMS_ProcessStandbyState(pBmsState);
            if (nextState == BMS_FSM_STATE_STANDBY) {
                /* staying in state, processed by state function */
            } else if (nextState == BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR) {
                BMS_SetState(
                    pBmsState, BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, BMS_FSM_SUBSTATE_ENTRY, BMS_FSM_SHORTTIME);
            } else if (nextState == BMS_FSM_STATE_PRECHARGE) {
                BMS_SetState(pBmsState, BMS_FSM_STATE_PRECHARGE, BMS_FSM_SUBSTATE_ENTRY, BMS_FSM_SHORTTIME);
            } else {
                FAS_ASSERT(FAS_TRAP); /* invalid state transition requested */
            }
            break;

        /****************************PRECHARGE********************************/
        case BMS_FSM_STATE_PRECHARGE:
            /* Options:
             * (1) stay in this main state,
             * (2) transition to error state via open contactors to error state
             * (3) transition to standby state via open contactors to standby state
             * (4) transition to next allowed/defined state(s):
             *     - BMS_FSM_STATE_NORMAL
             * (5) invalid main state requested to transition from this state
             *     to --> assert
             */
            nextState = BMS_ProcessPrechargeState(pBmsState);
            if (nextState == BMS_FSM_STATE_PRECHARGE) {
                /* staying in state, processed by state function */
            } else if (nextState == BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR) {
                BMS_SetState(
                    pBmsState, BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, BMS_FSM_SUBSTATE_ENTRY, BMS_FSM_SHORTTIME);
            } else if (nextState == BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY) {
                BMS_SetState(
                    pBmsState, BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY, BMS_FSM_SUBSTATE_ENTRY, BMS_FSM_SHORTTIME);
            } else if (nextState == BMS_FSM_STATE_NORMAL) {
                BMS_SetState(pBmsState, BMS_FSM_STATE_NORMAL, BMS_FSM_SUBSTATE_ENTRY, BMS_FSM_SHORTTIME);
            } else {
                FAS_ASSERT(FAS_TRAP); /* invalid state transition requested */
            }
            break;

        /****************************NORMAL***********************************/
        case BMS_FSM_STATE_NORMAL:
            /* Options:
             * (1) stay in this main state,
             * (2) transition to error state via open contactors to error state
             * (3) transition to next allowed/defined state(s):
             *     - BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY
             * (4) invalid main state requested to transition from this state
             *     to --> assert
             */

            nextState = BMS_ProcessNormalState(pBmsState);
            if (nextState == BMS_FSM_STATE_NORMAL) {
                /* staying in state, processed by state function */
            } else if (nextState == BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR) {
                BMS_SetState(
                    pBmsState, BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, BMS_FSM_SUBSTATE_ENTRY, BMS_FSM_SHORTTIME);
            } else if (nextState == BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY) {
                BMS_SetState(
                    pBmsState, BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY, BMS_FSM_SUBSTATE_ENTRY, BMS_FSM_SHORTTIME);
            } else {
                FAS_ASSERT(FAS_TRAP); /* invalid state transition requested */
            }
            break;

        /****************************OPEN CONTACTORS TO ERROR***************/
        case BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR:
            /* Options:
             * (1) stay in this main state,
             * (2) transition to next allowed/defined state(s):
             *     - BMS_FSM_STATE_ERROR
             * (3) invalid main state requested to transition from this state
             *     to --> assert
             */
            nextState = BMS_ProcessOpenContactorsToErrorState(pBmsState);
            if (nextState == BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR) {
                /* staying in state, processed by state function */
            } else if (nextState == BMS_FSM_STATE_ERROR) {
                BMS_SetState(pBmsState, BMS_FSM_STATE_ERROR, BMS_FSM_SUBSTATE_ENTRY, BMS_FSM_SHORTTIME);
            } else {
                FAS_ASSERT(FAS_TRAP); /* invalid state transition requested */
            }
            break;
        /****************************ERROR*************************************/
        case BMS_FSM_STATE_ERROR:
            /* Options:
             * (1) stay in this main state,
             * (2) transition to next allowed/defined state(s):
             *     - BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY
             * (3) invalid main state requested to transition from this state
             *     to --> assert
             */
            nextState = BMS_ProcessErrorState(pBmsState);
            if (nextState == BMS_FSM_STATE_ERROR) {
                /* staying in state, processed by state function */
            } else if (nextState == BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY) {
                BMS_SetState(
                    pBmsState, BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY, BMS_FSM_SUBSTATE_ENTRY, BMS_FSM_SHORTTIME);
            } else {
                FAS_ASSERT(FAS_TRAP); /* invalid state transition requested */
            }
            break;

        default:
            /* invalid currentState */
            FAS_ASSERT(FAS_TRAP);
            break;
    } /* end switch (pBmsState->currentState) */

    return ranStateMachine;
}

/*========== Extern Function Implementations ================================*/

extern STD_RETURN_TYPE_e BMS_GetInitializationState(void) {
    return bms_state.information.initFinished;
}

extern BMS_FSM_STATES_e BMS_GetState(void) {
    return bms_state.currentState;
}

extern BMS_FSM_SUBSTATES_e BMS_GetSubstate(void) {
    return bms_state.currentSubstate;
}

BMS_RETURN_TYPE_e BMS_SetStateRequest(BMS_STATE_REQUEST_e statereq) {
    BMS_RETURN_TYPE_e retVal = BMS_OK;

    OS_EnterTaskCritical();
    retVal = BMS_CheckStateRequest(statereq);

    if (retVal == BMS_OK) {
        bms_state.stateRequest = statereq;
    }
    OS_ExitTaskCritical();

    return retVal;
}

extern STD_RETURN_TYPE_e BMS_Trigger(void) {
    bms_state.information.currentSystick = OS_GetTickCount();
    STD_RETURN_TYPE_e returnValue        = STD_OK;
    bool earlyExit                       = false;

    /* Check re-entrance of function */
    if (BMS_MULTIPLE_CALLS_YES == BMS_CheckMultipleCalls(&bms_state)) {
        returnValue = STD_NOT_OK;
        earlyExit   = true;
    }

    if (earlyExit == false) {
        if (bms_state.information.nextStringClosedTimer > 0u) {
            bms_state.information.nextStringClosedTimer--;
        }
        if (bms_state.information.stringOpenTimeout > 0u) {
            bms_state.information.stringOpenTimeout--;
        }

        if (bms_state.information.stringCloseTimeout > 0u) {
            bms_state.information.stringCloseTimeout--;
        }

        if (bms_state.information.OscillationTimeout > 0u) {
            bms_state.information.OscillationTimeout--;
        }

        if (bms_state.timer > 0u) {
            if ((--bms_state.timer) > 0u) {
                bms_state.triggerEntry--;
                returnValue = STD_OK;
                earlyExit   = true;
            }
        }
    }

    if (earlyExit == false) {
        if (bms_state.currentState != BMS_FSM_STATE_UNINITIALIZED) {
            BMS_GetMeasurementValues();
            BMS_UpdateBatterySystemState(&bms_tablePackValues);
            SOA_CheckVoltages(&bms_tableMinMax);
            SOA_CheckTemperatures(&bms_tableMinMax, &bms_tablePackValues);
            SOA_CheckCurrent(&bms_tablePackValues);
            SOA_CheckSlaveTemperatures();
            BMS_CheckOpenSenseWire();
            CONT_CheckFeedback();
        }

        BMS_RunStateMachine(&bms_state);
        bms_state.triggerEntry--;
        bms_state.information.counter++;

        /* Send an asynchronous bms currentState message if the currentState or currentSubstate changed*/
        if ((bms_state.currentState != bms_state.previousState) ||
            (bms_state.currentSubstate != bms_state.previousSubstate)) {
            CANTX_TransmitBmsState();
        }
    }

    return returnValue;
}

extern BMS_CURRENT_FLOW_STATE_e BMS_GetBatterySystemState(void) {
    return bms_state.information.currentFlowState;
}

extern BMS_CURRENT_FLOW_STATE_e BMS_GetCurrentFlowDirection(int32_t current_mA) {
    /* AXIVION Routine Generic-MissingParameterAssert: current_mA: parameter accepts whole range */
    BMS_CURRENT_FLOW_STATE_e retVal = BMS_DISCHARGING;

    if (BS_POSITIVE_DISCHARGE_CURRENT == true) {
        if (current_mA >= BS_REST_CURRENT_mA) {
            retVal = BMS_DISCHARGING;
        } else if (current_mA <= -BS_REST_CURRENT_mA) {
            retVal = BMS_CHARGING;
        } else {
            retVal = BMS_AT_REST;
        }
    } else {
        if (current_mA <= -BS_REST_CURRENT_mA) {
            retVal = BMS_DISCHARGING;
        } else if (current_mA >= BS_REST_CURRENT_mA) {
            retVal = BMS_CHARGING;
        } else {
            retVal = BMS_AT_REST;
        }
    }
    return retVal;
}

extern bool BMS_IsStringClosed(uint8_t stringNumber) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    bool retval = false;
    if (bms_state.information.closedStrings[stringNumber] == true) {
        retval = true;
    }
    return retval;
}

extern bool BMS_IsStringPrecharging(uint8_t stringNumber) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    bool retval = false;
    if (bms_state.information.closedPrechargeContactors[stringNumber] == true) {
        retval = true;
    }
    return retval;
}

extern uint8_t BMS_GetNumberOfConnectedStrings(void) {
    return bms_state.information.numberOfClosedStrings;
}

extern bool BMS_IsTransitionToErrorStateActive(void) {
    return bms_state.information.transitionToErrorState;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
/* Static database copy getter functions */
extern DATA_BLOCK_MIN_MAX_s *TEST_BMS_GetMinMaxTable(void) {
    return &bms_tableMinMax;
}
extern DATA_BLOCK_OPEN_WIRE_s *TEST_BMS_GetOpenWireTable(void) {
    return &bms_tableOpenWire;
}
extern DATA_BLOCK_PACK_VALUES_s *TEST_BMS_GetPackValuesTable(void) {
    return &bms_tablePackValues;
}

extern void TEST_SetTablePackValues(DATA_BLOCK_PACK_VALUES_s *packValues) {
    bms_tablePackValues = *packValues;
}
extern BMS_RETURN_TYPE_e TEST_BMS_CheckStateRequest(BMS_STATE_REQUEST_e statereq) {
    return BMS_CheckStateRequest(statereq);
}
extern BMS_STATE_REQUEST_e TEST_BMS_TransferStateRequest(void) {
    return BMS_TransferStateRequest();
}
extern uint8_t TEST_BMS_CheckCanRequests(void) {
    return BMS_CheckCanRequests();
}
extern bool TEST_BMS_IsAnyFatalErrorFlagSet(void) {
    return BMS_IsAnyFatalErrorFlagSet();
}
extern STD_RETURN_TYPE_e TEST_BMS_IsBatterySystemStateOkay(BMS_STATE_s *pBmsState) {
    return BMS_IsBatterySystemStateOkay(pBmsState);
}
extern bool TEST_BMS_IsContactorFeedbackValid(uint8_t stringNumber, CONT_TYPE_e contactorType) {
    return BMS_IsContactorFeedbackValid(stringNumber, contactorType);
}
extern void TEST_BMS_GetMeasurementValues(void) {
    BMS_GetMeasurementValues();
}
extern void TEST_BMS_CheckOpenSenseWire(void) {
    BMS_CheckOpenSenseWire();
}
extern BMS_RESULT_PRECHARGE_PROCESS_e TEST_BMS_MonitorPrechargeProcess(
    BMS_STATE_s *pBmsState,
    uint8_t stringNumber,
    const DATA_BLOCK_PACK_VALUES_s *pPackValues,
    BS_PRECHARGE_MONITORING_e monitoringParameters,
    uint32_t timeout_ms) {
    return BMS_MonitorPrechargeProcess(pBmsState, stringNumber, pPackValues, monitoringParameters, timeout_ms);
}
extern uint8_t TEST_BMS_GetHighestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    return BMS_GetHighestString(precharge, pPackValues);
}
extern uint8_t TEST_BMS_GetClosestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    return BMS_GetClosestString(precharge, pPackValues);
}

extern uint8_t TEST_BMS_GetLowestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    return BMS_GetLowestString(precharge, pPackValues);
}
extern int32_t TEST_BMS_GetStringVoltageDifference(uint8_t string, DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    return BMS_GetStringVoltageDifference(string, pPackValues);
}
extern int32_t TEST_BMS_GetAverageStringCurrent(DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    return BMS_GetAverageStringCurrent(pPackValues);
}
extern BMS_FSM_STATES_e TEST_BMS_ProcessInitializedState(BMS_STATE_s *pBmsState) {
    return BMS_ProcessInitializedState(pBmsState);
}
extern BMS_FSM_STATES_e TEST_BMS_ProcessIdleState(BMS_STATE_s *pBmsState) {
    return BMS_ProcessIdleState(pBmsState);
}
extern BMS_FSM_STATES_e TEST_BMS_ProcessOpenContactorsToStandbyState(BMS_STATE_s *pBmsState) {
    return BMS_ProcessOpenContactorsToStandbyState(pBmsState);
}
extern BMS_FSM_STATES_e TEST_BMS_ProcessStandbyState(BMS_STATE_s *pBmsState) {
    return BMS_ProcessStandbyState(pBmsState);
}
extern BMS_FSM_STATES_e TEST_BMS_ProcessPrechargeState(BMS_STATE_s *pBmsState) {
    return BMS_ProcessPrechargeState(pBmsState);
}
extern BMS_FSM_STATES_e TEST_BMS_ProcessNormalState(BMS_STATE_s *pBmsState) {
    return BMS_ProcessNormalState(pBmsState);
}
extern BMS_FSM_STATES_e TEST_BMS_ProcessOpenContactorsToErrorState(BMS_STATE_s *pBmsState) {
    return BMS_ProcessOpenContactorsToErrorState(pBmsState);
}
extern BMS_FSM_STATES_e TEST_BMS_ProcessErrorState(BMS_STATE_s *pBmsState) {
    return BMS_ProcessErrorState(pBmsState);
}
extern STD_RETURN_TYPE_e TEST_BMS_RunStateMachine(BMS_STATE_s *pBmsState) {
    return BMS_RunStateMachine(pBmsState);
}
extern void TEST_BMS_UpdateBatterySystemState(DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    BMS_UpdateBatterySystemState(pPackValues);
}

#endif
