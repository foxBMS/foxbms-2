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
 * @file    bms.c
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2021-10-18 (date of last update)
 * @ingroup ENGINE
 * @prefix  BMS
 *
 * @brief   bms driver implementation
 */

/*========== Includes =======================================================*/
#include "bms.h"

#include "battery_cell_cfg.h"

#include "afe.h"
#include "bal.h"
#include "contactor.h"
#include "database.h"
#include "diag.h"
#include "foxmath.h"
#include "led.h"
#include "meas.h"
#include "os.h"
#include "soa.h"

/*========== Macros and Definitions =========================================*/
/** default value for unset "active delay time" */
#define BMS_NO_ACTIVE_DELAY_TIME_ms (UINT32_MAX)

/**
 * Saves the last state and the last substate
 */
#define BMS_SAVELASTSTATES()                  \
    bms_state.laststate    = bms_state.state; \
    bms_state.lastsubstate = bms_state.substate

/*========== Static Constant and Variable Definitions =======================*/

/**
 * contains the state of the bms state machine
 */
static BMS_STATE_s bms_state = {
    .timer                     = 0,
    .stateRequest              = BMS_STATE_NO_REQUEST,
    .state                     = BMS_STATEMACH_UNINITIALIZED,
    .substate                  = BMS_ENTRY,
    .laststate                 = BMS_STATEMACH_UNINITIALIZED,
    .lastsubstate              = BMS_ENTRY,
    .triggerentry              = 0u,
    .ErrRequestCounter         = 0u,
    .initFinished              = STD_NOT_OK,
    .counter                   = 0u,
    .OscillationTimeout        = 0u,
    .PrechargeTryCounter       = 0u,
    .powerPath                 = BMS_POWER_PATH_OPEN,
    .closedStrings             = {0u},
    .closedPrechargeContactors = {0u},
    .numberOfClosedStrings     = 0u,
    .deactivatedStrings        = {0},
    .firstClosedString         = 0u,
    .stringOpenTimeout         = 0u,
    .nextstringclosedtimer     = 0u,
    .stringCloseTimeout        = 0u,
    .nextstate                 = BMS_STATEMACH_STANDBY,
    .restTimer_10ms            = BS_RELAXATION_PERIOD_10ms,
    .currentFlowState          = BMS_RELAXATION,
    .remainingDelay_ms         = BMS_NO_ACTIVE_DELAY_TIME_ms,
    .minimumActiveDelay_ms     = BMS_NO_ACTIVE_DELAY_TIME_ms,
    .transitionToErrorState    = false,
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
 * @brief       checks the state requests that are made.
 * @details     This function checks the validity of the state requests. The
 *              results of the checked is returned immediately.
 * @param[in]   statereq    state request to be checked
 * @return      result of the state request that was made
 */
static BMS_RETURN_TYPE_e BMS_CheckStateRequest(BMS_STATE_REQUEST_e statereq);

/**
 * @brief   transfers the current state request to the state machine.
 * @details This function takes the current state request from #bms_state
 *          transfers it to the state machine. It resets the value from
 *          #bms_state to #BMS_STATE_NO_REQUEST
 * @return  current state request
 */
static BMS_STATE_REQUEST_e BMS_TransferStateRequest(void);

/**
 * @brief   re-entrance check of SYS state machine trigger function
 * @details This function is not re-entrant and should only be called time- or
 *          event-triggered. It increments the triggerentry counter from the
 *          state variable ltc_state. It should never be called by two
 *          different processes, so if it is the case, triggerentry should
 *          never be higher than 0 when this function is called.
 * @return  retval  0 if no further instance of the function is active, 0xff
 *          else
 */
static uint8_t BMS_CheckReEntrance(void);

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
static STD_RETURN_TYPE_e BMS_IsBatterySystemStateOkay(void);

/** Get latest database entries for static module variables */
static void BMS_GetMeasurementValues(void);

/**
 * @brief   Check for any open voltage sense wire
 */
static void BMS_CheckOpenSenseWire(void);

/**
 * @brief       Checks if the current limitations are violated
 * @param[in]   stringNumber    string addressed
 * @param[in]   pPackValues     pointer to pack values database entry
 * @return      #STD_OK if the current limitations are NOT violated, else
 *              #STD_NOT_OK (type: #STD_RETURN_TYPE_e)
 */
static STD_RETURN_TYPE_e BMS_CheckPrecharge(uint8_t stringNumber, const DATA_BLOCK_PACK_VALUES_s *pPackValues);

/**
 * @brief   Returns ID of string with highest total voltage
 * @details This is used to close the first string when drive-off is requested.
 * @param[in]   precharge   If #BMS_DO_NOT_TAKE_PRECHARGE_INTO_ACCCOUNT,
 *                          precharge availability for string is ignored.
 *                          if #BMS_TAKE_PRECHARGE_INTO_ACCCOUNT, only select
 *                          string that has precharge available.
 * @param[in]   pPackValues pointer to pack values database entry
 * @return  index of string with highest voltage If no string is available,
 *          returns #BMS_NO_STRING_AVAILABLE.
 */
static uint8_t BMS_GetHighestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues);

/**
 * @brief   Returns ID of string with voltage closest to first closed string voltage
 * @details This is used to close further strings in drive.
 * @param[in]   precharge   If #BMS_DO_NOT_TAKE_PRECHARGE_INTO_ACCCOUNT,
 *                          precharge availability for string is ignored.
 *                          if #BMS_TAKE_PRECHARGE_INTO_ACCCOUNT, only select
 *                          string that has precharge available.
 * @param[in]   pPackValues pointer to pack values database entry
 * @return  index of string with voltage closest to the first closed string voltage.
 *          If no string is available, returns #BMS_NO_STRING_AVAILABLE.
 */
static uint8_t BMS_GetClosestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues);

/**
 * @brief   Returns ID of string with lowest total voltage
 * @details This is used to close the first string when charge-off is requested.
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
 * @details This function is used to check voltage when trying to close further
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
 * @details This function is used when closing strings.
 * @param[in]   pPackValues pointer to pack values database entry
 * @return  average current taking all strings into account in mA. INT32_MAX if there is no valid current measurement
 */
static int32_t BMS_GetAverageStringCurrent(DATA_BLOCK_PACK_VALUES_s *pPackValues);

/**
 * @brief   Updates battery system state variable depending on measured/recent
 *          current values
 * @param[in]   pPackValues  recent measured values from current sensor
 */
static void BMS_UpdateBatsysState(DATA_BLOCK_PACK_VALUES_s *pPackValues);

/*========== Static Function Implementations ================================*/

static BMS_RETURN_TYPE_e BMS_CheckStateRequest(BMS_STATE_REQUEST_e statereq) {
    if (statereq == BMS_STATE_ERROR_REQUEST) {
        return BMS_OK;
    }

    if (bms_state.stateRequest == BMS_STATE_NO_REQUEST) {
        /* init only allowed from the uninitialized state */
        if (statereq == BMS_STATE_INIT_REQUEST) {
            if (bms_state.state == BMS_STATEMACH_UNINITIALIZED) {
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

static uint8_t BMS_CheckReEntrance(void) {
    uint8_t retval = 0;
    OS_EnterTaskCritical();
    if (!bms_state.triggerentry) {
        bms_state.triggerentry++;
    } else {
        retval = 0xFF; /* multiple calls of function */
    }
    OS_ExitTaskCritical();
    return retval;
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
    uint8_t retVal                    = BMS_REQ_ID_NOREQ;
    DATA_BLOCK_STATEREQUEST_s request = {.header.uniqueId = DATA_BLOCK_ID_STATEREQUEST};

    DATA_READ_DATA(&request);

    if (request.stateRequestViaCan == BMS_REQ_ID_STANDBY) {
        retVal = BMS_REQ_ID_STANDBY;
    } else if (request.stateRequestViaCan == BMS_REQ_ID_NORMAL) {
        retVal = BMS_REQ_ID_NORMAL;
    }

#if BS_SEPARATE_POWER_PATHS == 1
    else if (request.stateRequestViaCan == BMS_REQ_ID_CHARGE) { /* NOLINT(readability/braces) */
        retVal = BMS_REQ_ID_CHARGE;
    }
#endif /*  BS_SEPARATE_POWER_PATHS == 1 */

    return retVal;
}

static void BMS_CheckOpenSenseWire(void) {
    uint8_t openWireDetected = 0;

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        /* Iterate over all modules */
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES; m++) {
            /* Iterate over all voltage sense wires: cells per module + 1 */
            for (uint8_t wire = 0u; wire < (BS_NR_OF_CELLS_PER_MODULE + 1); wire++) {
                /* open wire detected */
                if (bms_tableOpenWire.openwire[s][(wire + (m * (BS_NR_OF_CELLS_PER_MODULE + 1))) == 1] > 0u) {
                    openWireDetected++;

                    /* Add additional error handling here */
                }
            }
        }
        /* Set error if open wire detected */
        if (openWireDetected == 0u) {
            DIAG_Handler(DIAG_ID_OPEN_WIRE, DIAG_EVENT_OK, DIAG_STRING, s);
        } else {
            DIAG_Handler(DIAG_ID_OPEN_WIRE, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
        }
    }
}

static STD_RETURN_TYPE_e BMS_CheckPrecharge(uint8_t stringNumber, const DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    STD_RETURN_TYPE_e retVal = STD_NOT_OK;
    /* make sure that we do not access the arrays in the database
       tables out of bounds */
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    FAS_ASSERT(pPackValues != NULL_PTR);

    /* Only check precharging if current value and voltages are valid */
    if ((0u == pPackValues->invalidStringCurrent[stringNumber]) &&
        (0u == pPackValues->invalidStringVoltage[stringNumber]) && (0u == pPackValues->invalidHvBusVoltage)) {
        /* Only current not current direction is checked */
        const int32_t current_mA                = MATH_AbsInt32_t(pPackValues->stringCurrent_mA[stringNumber]);
        const int64_t cont_prechargeVoltDiff_mV = MATH_AbsInt64_t(
            (int64_t)pPackValues->stringVoltage_mV[stringNumber] - (int64_t)pPackValues->highVoltageBusVoltage_mV);

        if ((cont_prechargeVoltDiff_mV < BMS_PRECHARGE_VOLTAGE_THRESHOLD_mV) &&
            (current_mA < BMS_PRECHARGE_CURRENT_THRESHOLD_mA)) {
            retVal = STD_OK;
        } else {
            retVal = STD_NOT_OK;
        }
    }

    return retVal;
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
            if (bms_state.minimumActiveDelay_ms > kDelay_ms) {
                bms_state.minimumActiveDelay_ms = kDelay_ms;
            }
            fatalErrorActive = true;
        }
    }
    return fatalErrorActive;
}

static STD_RETURN_TYPE_e BMS_IsBatterySystemStateOkay(void) {
    STD_RETURN_TYPE_e retVal          = STD_OK; /* is set to STD_NOT_OK if error detected */
    static uint32_t previousTimestamp = 0u;
    uint32_t timestamp                = OS_GetTickCount();

    /* Check if any fatal error is detected */
    const bool isErrorActive = BMS_IsAnyFatalErrorFlagSet();

    /** Check if a fatal error has been detected previously. If yes, check delay */
    if (true == bms_state.transitionToErrorState) {
        /* Decrease active delay since last call */
        const uint32_t timeSinceLastCall_ms = timestamp - previousTimestamp;
        if (timeSinceLastCall_ms <= bms_state.remainingDelay_ms) {
            bms_state.remainingDelay_ms -= timeSinceLastCall_ms;
        } else {
            bms_state.remainingDelay_ms = 0u;
        }

        /* Check if delay from a new error is shorter then active delay from
         * previously detected error in BMS statemachine */
        if (bms_state.remainingDelay_ms >= bms_state.minimumActiveDelay_ms) {
            bms_state.remainingDelay_ms = bms_state.minimumActiveDelay_ms;
        }
    } else {
        /* Delay is not active, check if it should be activated */
        if (true == isErrorActive) {
            bms_state.transitionToErrorState = true;
            bms_state.remainingDelay_ms      = bms_state.minimumActiveDelay_ms;
        }
    }

    /** Set previous timestamp for next call */
    previousTimestamp = timestamp;

    /* Check if bms statemachine should switch to error state. This is the case
     * if the delay is activated and the remaining delay is down to 0 */
    if ((true == bms_state.transitionToErrorState) && (0u == bms_state.remainingDelay_ms)) {
        retVal = STD_NOT_OK;
    }

    return retVal;
}

static uint8_t BMS_GetHighestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    FAS_ASSERT(pPackValues != NULL_PTR);
    uint8_t highest_string_index = BMS_NO_STRING_AVAILABLE;
    int32_t max_stringVoltage_mV = INT32_MIN;

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        if ((pPackValues->stringVoltage_mV[s] >= max_stringVoltage_mV) &&
            (0u == pPackValues->invalidStringVoltage[s])) {
            if (bms_state.deactivatedStrings[s] == 0u) {
                if (precharge == BMS_DO_NOT_TAKE_PRECHARGE_INTO_ACCCOUNT) {
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
    if (0u == pPackValues->invalidStringVoltage[bms_state.firstClosedString]) {
        closedStringVoltage_mV = pPackValues->stringVoltage_mV[bms_state.firstClosedString];
        searchString           = true;
    } else if (0u == pPackValues->invalidHvBusVoltage) {
        /* Use high voltage bus voltage if measured string voltage is invalid */
        closedStringVoltage_mV = pPackValues->highVoltageBusVoltage_mV;
        searchString           = true;
    } else {
        /* Do not search for next string  if no valid voltages could be measured */
        searchString = false;
    }

    if (true == searchString) {
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            const bool isStringClosed          = BMS_IsStringClosed(s);
            const uint8_t isStringVoltageValid = pPackValues->invalidStringVoltage[s];
            if ((false == isStringClosed) && (0u == isStringVoltageValid)) {
                /* Only check open strings with valid voltages */
                int32_t minimumVoltageDifference_mV = INT32_MAX;
                int32_t voltageDifference_mV        = labs(closedStringVoltage_mV - pPackValues->stringVoltage_mV[s]);
                if (voltageDifference_mV <= minimumVoltageDifference_mV) {
                    if (bms_state.deactivatedStrings[s] == 0u) {
                        if (precharge == BMS_TAKE_PRECHARGE_INTO_ACCCOUNT) {
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
            (0u == pPackValues->invalidStringVoltage[s])) {
            if (bms_state.deactivatedStrings[s] == 0u) {
                if (precharge == BMS_DO_NOT_TAKE_PRECHARGE_INTO_ACCCOUNT) {
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
    if ((0u == pPackValues->invalidStringVoltage[string]) &&
        (0u == pPackValues->invalidStringVoltage[bms_state.firstClosedString])) {
        /* Calculate difference between string voltages */
        voltageDifference_mV = MATH_AbsInt32_t(
            pPackValues->stringVoltage_mV[string] - pPackValues->stringVoltage_mV[bms_state.firstClosedString]);
    } else if ((0u == pPackValues->invalidStringVoltage[string]) && (0u == pPackValues->invalidHvBusVoltage)) {
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
    if (1u == pPackValues->invalidPackCurrent) {
        average_current = INT32_MAX;
    }
    return average_current;
}

static void BMS_UpdateBatsysState(DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    FAS_ASSERT(pPackValues != NULL_PTR);

    /* Only update system state if current value is valid */
    if (0u == pPackValues->invalidPackCurrent) {
        if (POSITIVE_DISCHARGE_CURRENT == true) {
            /* Positive current values equal a discharge of the battery system */
            if (pPackValues->packCurrent_mA >= BS_REST_CURRENT_mA) { /* TODO: string use pack current */
                bms_state.currentFlowState = BMS_DISCHARGING;
                bms_state.restTimer_10ms   = BS_RELAXATION_PERIOD_10ms;
            } else if (pPackValues->packCurrent_mA <= -BS_REST_CURRENT_mA) {
                bms_state.currentFlowState = BMS_CHARGING;
                bms_state.restTimer_10ms   = BS_RELAXATION_PERIOD_10ms;
            } else {
                /* Current below rest current: either battery system is at rest
             * or the relaxation process is still ongoing */
                if (bms_state.restTimer_10ms == 0u) {
                    /* Rest timer elapsed -> battery system at rest */
                    bms_state.currentFlowState = BMS_AT_REST;
                } else {
                    bms_state.restTimer_10ms--;
                    bms_state.currentFlowState = BMS_RELAXATION;
                }
            }
        } else {
            /* Negative current values equal a discharge of the battery system */
            if (pPackValues->packCurrent_mA <= -BS_REST_CURRENT_mA) {
                bms_state.currentFlowState = BMS_DISCHARGING;
                bms_state.restTimer_10ms   = BS_RELAXATION_PERIOD_10ms;
            } else if (pPackValues->packCurrent_mA >= BS_REST_CURRENT_mA) {
                bms_state.currentFlowState = BMS_CHARGING;
                bms_state.restTimer_10ms   = BS_RELAXATION_PERIOD_10ms;
            } else {
                /* Current below rest current: either battery system is at rest
             * or the relaxation process is still ongoing */
                if (bms_state.restTimer_10ms == 0u) {
                    /* Rest timer elapsed -> battery system at rest */
                    bms_state.currentFlowState = BMS_AT_REST;
                } else {
                    bms_state.restTimer_10ms--;
                    bms_state.currentFlowState = BMS_RELAXATION;
                }
            }
        }
    }
}

/*========== Extern Function Implementations ================================*/

extern STD_RETURN_TYPE_e BMS_GetInitializationState(void) {
    return bms_state.initFinished;
}

extern BMS_STATEMACH_e BMS_GetState(void) {
    return bms_state.state;
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

void BMS_Trigger(void) {
    BMS_STATE_REQUEST_e statereq           = BMS_STATE_NO_REQUEST;
    DATA_BLOCK_SYSTEMSTATE_s systemstate   = {.header.uniqueId = DATA_BLOCK_ID_SYSTEMSTATE};
    uint32_t timestamp                     = OS_GetTickCount();
    static uint32_t nextOpenWireCheck      = 0;
    STD_RETURN_TYPE_e retVal               = STD_NOT_OK;
    static uint8_t stringNumber            = 0u;
    static uint8_t nextStringNumber        = 0u;
    CONT_ELECTRICAL_STATE_TYPE_e contstate = CONT_SWITCH_OFF;
    STD_RETURN_TYPE_e contRetVal           = STD_NOT_OK;

    if (bms_state.state != BMS_STATEMACH_UNINITIALIZED) {
        BMS_GetMeasurementValues();
        BMS_UpdateBatsysState(&bms_tablePackValues);
        SOA_CheckVoltages(&bms_tableMinMax);
        SOA_CheckTemperatures(&bms_tableMinMax, &bms_tablePackValues);
        SOA_CheckCurrent(&bms_tablePackValues);
        SOA_CheckSlaveTemperatures();
        BMS_CheckOpenSenseWire();
        CONT_CheckFeedback();
    }
    /* Check re-entrance of function */
    if (BMS_CheckReEntrance() > 0u) {
        return;
    }

    if (bms_state.nextstringclosedtimer > 0u) {
        bms_state.nextstringclosedtimer--;
    }
    if (bms_state.stringOpenTimeout > 0u) {
        bms_state.stringOpenTimeout--;
    }

    if (bms_state.stringCloseTimeout > 0u) {
        bms_state.stringCloseTimeout--;
    }

    if (bms_state.OscillationTimeout > 0u) {
        bms_state.OscillationTimeout--;
    }

    if (bms_state.timer > 0u) {
        if ((--bms_state.timer) > 0u) {
            bms_state.triggerentry--;
            return; /* handle state machine only if timer has elapsed */
        }
    }

    /****Happens every time the state machine is triggered**************/
    switch (bms_state.state) {
        /****************************UNINITIALIZED****************************/
        case BMS_STATEMACH_UNINITIALIZED:
            /* waiting for Initialization Request */
            statereq = BMS_TransferStateRequest();
            if (statereq == BMS_STATE_INIT_REQUEST) {
                BMS_SAVELASTSTATES();
                bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                bms_state.state    = BMS_STATEMACH_INITIALIZATION;
                bms_state.substate = BMS_ENTRY;
            } else if (statereq == BMS_STATE_NO_REQUEST) {
                /* no actual request pending */
            } else {
                bms_state.ErrRequestCounter++; /* illegal request pending */
            }
            break;

        /****************************INITIALIZATION***************************/
        case BMS_STATEMACH_INITIALIZATION:
            BMS_SAVELASTSTATES();

            bms_state.timer    = BMS_STATEMACH_LONGTIME;
            bms_state.state    = BMS_STATEMACH_INITIALIZED;
            bms_state.substate = BMS_ENTRY;
            break;

        /****************************INITIALIZED******************************/
        case BMS_STATEMACH_INITIALIZED:
            BMS_SAVELASTSTATES();
            bms_state.initFinished = STD_OK;
            bms_state.timer        = BMS_STATEMACH_SHORTTIME;
            bms_state.state        = BMS_STATEMACH_IDLE;
            bms_state.substate     = BMS_ENTRY;
            break;

        /****************************IDLE*************************************/
        case BMS_STATEMACH_IDLE:
            BMS_SAVELASTSTATES();

            if (bms_state.substate == BMS_ENTRY) {
                DATA_READ_DATA(&systemstate);
                systemstate.bmsCanState = BMS_CANSTATE_IDLE;
                DATA_WRITE_DATA(&systemstate);
                bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                bms_state.substate = BMS_CHECK_ERROR_FLAGS;
                break;
            } else if (bms_state.substate == BMS_CHECK_ERROR_FLAGS) {
                if (BMS_IsBatterySystemStateOkay() == STD_NOT_OK) {
                    bms_state.timer     = BMS_STATEMACH_SHORTTIME;
                    bms_state.state     = BMS_STATEMACH_OPENCONTACTORS;
                    bms_state.nextstate = BMS_STATEMACH_ERROR;
                    bms_state.substate  = BMS_ENTRY;
                    break;
                } else {
                    bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                    bms_state.substate = BMS_CHECK_STATE_REQUESTS;
                    break;
                }
            } else if (bms_state.substate == BMS_CHECK_STATE_REQUESTS) {
                if (BMS_CheckCanRequests() == BMS_REQ_ID_STANDBY) {
                    bms_state.timer     = BMS_STATEMACH_SHORTTIME;
                    bms_state.state     = BMS_STATEMACH_OPENCONTACTORS;
                    bms_state.nextstate = BMS_STATEMACH_STANDBY;
                    bms_state.substate  = BMS_ENTRY;
                    break;
                } else {
                    bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                    bms_state.substate = BMS_CHECK_ERROR_FLAGS;
                    break;
                }
            }
            break;

        /****************************OPEN CONTACTORS**************************/
        case BMS_STATEMACH_OPENCONTACTORS:
            BMS_SAVELASTSTATES();

            if (bms_state.substate == BMS_ENTRY) {
                BAL_SetStateRequest(BAL_STATE_NO_BALANCING_REQUEST);
                bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                bms_state.substate = BMS_OPEN_ALL_PRECHARGES;
                break;
            } else if (bms_state.substate == BMS_OPEN_ALL_PRECHARGES) {
                for (stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
                    if (bs_stringsWithPrecharge[stringNumber] == BS_STRING_WITH_PRECHARGE) {
                        CONT_OpenPrecharge(stringNumber);
                        bms_state.closedPrechargeContactors[stringNumber] = 0u;
                    }
                }
                /* Now go to string opening */
                stringNumber       = BS_NR_OF_STRINGS - 1u; /* Select last string */
                bms_state.substate = BMS_OPEN_STRINGS;
                bms_state.timer    = BMS_TIME_WAIT_AFTER_OPENING_PRECHARGE;
            } else if (bms_state.substate == BMS_OPEN_STRINGS) {
                CONT_OpenString(stringNumber);
                bms_state.timer             = BMS_TIME_WAIT_AFTER_OPENING_STRING;
                bms_state.substate          = BMS_CHECK_STRING_OPEN;
                bms_state.stringOpenTimeout = BMS_STRING_OPEN_TIMEOUT;
                break;
            } else if (bms_state.substate == BMS_CHECK_STRING_OPEN) {
                contstate = CONT_GetState(stringNumber);
                if (contstate == CONT_SWITCH_OFF) {
                    if (bms_state.numberOfClosedStrings > 0u) {
                        bms_state.numberOfClosedStrings--;
                    }
                    bms_state.closedStrings[stringNumber] = 0u;
                    if (stringNumber > 0u) {
                        stringNumber--;
                        bms_state.substate = BMS_OPEN_STRINGS;
                        bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                        break;
                    } else {
                        bms_state.substate = BMS_OPEN_STRINGS_EXIT;
                        bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                    }
                    break;
                } else if (bms_state.stringOpenTimeout == 0u) {
                    /* String takes too long to close, go to next string */
                    bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                    bms_state.substate = BMS_OPEN_STRINGS;
                    break;
                } else {
                    /* String not opened, re-issue closing request */
                    CONT_OpenString(nextStringNumber);
                    bms_state.timer = BMS_STATEMACH_SHORTTIME;
                    break;
                }
            } else if (bms_state.substate == BMS_OPEN_STRINGS_EXIT) {
                if (bms_state.nextstate == BMS_STATEMACH_STANDBY) {
                    bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                    bms_state.state    = BMS_STATEMACH_STANDBY;
                    bms_state.substate = BMS_ENTRY;
                    break;
                } else {
                    bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                    bms_state.state    = BMS_STATEMACH_ERROR;
                    bms_state.substate = BMS_ENTRY;
                }
            }
            break;

        /****************************STANDBY**********************************/
        case BMS_STATEMACH_STANDBY:
            BMS_SAVELASTSTATES();
            if (bms_state.substate == BMS_ENTRY) {
                BAL_SetStateRequest(BAL_STATE_ALLOWBALANCING_REQUEST);
#if LTC_STANDBY_PERIODIC_OPEN_WIRE_CHECK == TRUE
                nextOpenWireCheck = timestamp + LTC_STANDBY_OPEN_WIRE_PERIOD_ms;
#endif /* LTC_STANDBY_PERIODIC_OPEN_WIRE_CHECK == TRUE */
                bms_state.timer    = BMS_STATEMACH_MEDIUMTIME;
                bms_state.substate = BMS_CHECK_ERROR_FLAGS_INTERLOCK;
                DATA_READ_DATA(&systemstate);
                systemstate.bmsCanState = BMS_CANSTATE_STANDBY;
                DATA_WRITE_DATA(&systemstate);
                break;
            } else if (bms_state.substate == BMS_CHECK_ERROR_FLAGS_INTERLOCK) {
                if (BMS_IsBatterySystemStateOkay() == STD_NOT_OK) {
                    bms_state.timer     = BMS_STATEMACH_SHORTTIME;
                    bms_state.state     = BMS_STATEMACH_OPENCONTACTORS;
                    bms_state.nextstate = BMS_STATEMACH_ERROR;
                    bms_state.substate  = BMS_ENTRY;
                    break;
                } else {
                    bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                    bms_state.substate = BMS_INTERLOCK_CHECKED;
                    break;
                }
            } else if (bms_state.substate == BMS_INTERLOCK_CHECKED) {
                bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                bms_state.substate = BMS_CHECK_ERROR_FLAGS;
                break;
            } else if (bms_state.substate == BMS_CHECK_ERROR_FLAGS) {
                if (BMS_IsBatterySystemStateOkay() == STD_NOT_OK) {
                    bms_state.timer     = BMS_STATEMACH_SHORTTIME;
                    bms_state.state     = BMS_STATEMACH_OPENCONTACTORS;
                    bms_state.nextstate = BMS_STATEMACH_ERROR;
                    bms_state.substate  = BMS_ENTRY;
                    break;
                } else {
                    bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                    bms_state.substate = BMS_CHECK_STATE_REQUESTS;
                    break;
                }
            } else if (bms_state.substate == BMS_CHECK_STATE_REQUESTS) {
                if (BMS_CheckCanRequests() == BMS_REQ_ID_NORMAL) {
                    bms_state.powerPath = BMS_POWER_PATH_0;
                    bms_state.nextstate = BMS_STATEMACH_DISCHARGE;
                    bms_state.timer     = BMS_STATEMACH_SHORTTIME;
                    bms_state.state     = BMS_STATEMACH_PRECHARGE;
                    bms_state.substate  = BMS_ENTRY;
                    break;
                }
                if (BMS_CheckCanRequests() == BMS_REQ_ID_CHARGE) {
                    bms_state.powerPath = BMS_POWER_PATH_1;
                    bms_state.nextstate = BMS_STATEMACH_CHARGE;
                    bms_state.timer     = BMS_STATEMACH_SHORTTIME;
                    bms_state.state     = BMS_STATEMACH_PRECHARGE;
                    bms_state.substate  = BMS_ENTRY;
                    break;
                } else {
#if LTC_STANDBY_PERIODIC_OPEN_WIRE_CHECK == TRUE
                    if (nextOpenWireCheck <= timestamp) {
                        MEAS_RequestOpenWireCheck();
                        nextOpenWireCheck = timestamp + LTC_STANDBY_OPEN_WIRE_PERIOD_ms;
                    }
#endif /* LTC_STANDBY_PERIODIC_OPEN_WIRE_CHECK == TRUE */
                    bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                    bms_state.substate = BMS_CHECK_ERROR_FLAGS;
                    break;
                }
            }
            break;

        /****************************PRECHARGE********************************/
        case BMS_STATEMACH_PRECHARGE:
            BMS_SAVELASTSTATES();

            if (bms_state.substate == BMS_ENTRY) {
                DATA_READ_DATA(&systemstate);
                systemstate.bmsCanState = BMS_CANSTATE_PRECHARGE;
                DATA_WRITE_DATA(&systemstate);
                if (bms_state.nextstate == BMS_STATEMACH_CHARGE) {
                    stringNumber = BMS_GetLowestString(BMS_TAKE_PRECHARGE_INTO_ACCCOUNT, &bms_tablePackValues);
                } else {
                    stringNumber = BMS_GetHighestString(BMS_TAKE_PRECHARGE_INTO_ACCCOUNT, &bms_tablePackValues);
                }
                if (stringNumber == BMS_NO_STRING_AVAILABLE) {
                    bms_state.timer     = BMS_STATEMACH_SHORTTIME;
                    bms_state.state     = BMS_STATEMACH_OPENCONTACTORS;
                    bms_state.nextstate = BMS_STATEMACH_ERROR;
                    bms_state.substate  = BMS_ENTRY;
                    break;
                }
                bms_state.firstClosedString = stringNumber;
                if (bms_state.OscillationTimeout == 0u) {
                    bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                    bms_state.substate = BMS_PRECHARGE_CLOSE_PRECHARGE;
                } else if (BMS_IsBatterySystemStateOkay() == STD_NOT_OK) {
                    /* If precharge re-enter timeout not elapsed, wait (and check errors while waiting) */
                    bms_state.timer     = BMS_STATEMACH_SHORTTIME;
                    bms_state.state     = BMS_STATEMACH_OPENCONTACTORS;
                    bms_state.nextstate = BMS_STATEMACH_ERROR;
                    bms_state.substate  = BMS_ENTRY;
                    break;
                }
                break;
            } else if (bms_state.substate == BMS_PRECHARGE_CLOSE_PRECHARGE) {
                bms_state.OscillationTimeout                      = BMS_OSCILLATION_TIMEOUT;
                contRetVal                                        = CONT_ClosePrecharge(bms_state.firstClosedString);
                bms_state.closedPrechargeContactors[stringNumber] = 1u;
                if (contRetVal == STD_OK) {
                    bms_state.timer               = BMS_TIME_WAIT_AFTER_CLOSING_PRECHARGE;
                    bms_state.substate            = BMS_CHECK_ERROR_FLAGS_CLOSINGPRECHARGE;
                    bms_state.PrechargeTryCounter = 0u;
                } else {
                    bms_state.timer     = BMS_STATEMACH_SHORTTIME;
                    bms_state.state     = BMS_STATEMACH_OPENCONTACTORS;
                    bms_state.nextstate = BMS_STATEMACH_ERROR;
                    bms_state.substate  = BMS_ENTRY;
                }
                break;
            } else if (bms_state.substate == BMS_CHECK_ERROR_FLAGS_CLOSINGPRECHARGE) {
                if (BMS_IsBatterySystemStateOkay() == STD_NOT_OK) {
                    bms_state.timer     = BMS_STATEMACH_SHORTTIME;
                    bms_state.state     = BMS_STATEMACH_OPENCONTACTORS;
                    bms_state.nextstate = BMS_STATEMACH_ERROR;
                    bms_state.substate  = BMS_ENTRY;
                    break;
                } else {
                    bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                    bms_state.substate = BMS_CHECK_STATE_REQUESTS;
                    break;
                }
            } else if (bms_state.substate == BMS_CHECK_STATE_REQUESTS) {
                if (BMS_CheckCanRequests() == BMS_REQ_ID_STANDBY) {
                    bms_state.timer     = BMS_STATEMACH_SHORTTIME;
                    bms_state.state     = BMS_STATEMACH_OPENCONTACTORS;
                    bms_state.nextstate = BMS_STATEMACH_STANDBY;
                    bms_state.substate  = BMS_ENTRY;
                    break;
                } else {
                    bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                    bms_state.substate = BMS_PRECHARGE_CHECK_VOLTAGES;
                }
            } else if (bms_state.substate == BMS_PRECHARGE_CHECK_VOLTAGES) {
                retVal = BMS_CheckPrecharge(bms_state.firstClosedString, &bms_tablePackValues);
                if (retVal == STD_OK) {
                    CONT_CloseString(bms_state.firstClosedString);
                    bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                    bms_state.substate = BMS_CLOSE_FIRST_STRING_PRECHARGE_STATE;
                    break;
                } else {
                    if (bms_state.PrechargeTryCounter < (BMS_PRECHARGE_TRIES - 1u)) {
                        contRetVal = CONT_OpenPrecharge(bms_state.firstClosedString);
                        bms_state.closedPrechargeContactors[stringNumber] = 0u;
                        if (contRetVal == STD_OK) {
                            bms_state.timer    = BMS_TIME_WAIT_AFTERPRECHARGEFAIL;
                            bms_state.substate = BMS_PRECHARGE_CLOSE_PRECHARGE;
                            bms_state.PrechargeTryCounter++;
                        } else {
                            bms_state.timer     = BMS_STATEMACH_SHORTTIME;
                            bms_state.state     = BMS_STATEMACH_OPENCONTACTORS;
                            bms_state.nextstate = BMS_STATEMACH_ERROR;
                            bms_state.substate  = BMS_ENTRY;
                        }
                        break;
                    } else {
                        contRetVal = CONT_OpenPrecharge(bms_state.firstClosedString);
                        bms_state.closedPrechargeContactors[stringNumber] = 0u;
                        bms_state.timer                                   = BMS_STATEMACH_SHORTTIME;
                        bms_state.state                                   = BMS_STATEMACH_OPENCONTACTORS;
                        bms_state.nextstate                               = BMS_STATEMACH_ERROR;
                        bms_state.substate                                = BMS_ENTRY;
                        break;
                    }
                }
            } else if (bms_state.substate == BMS_CLOSE_FIRST_STRING_PRECHARGE_STATE) {
                CONT_CloseString(bms_state.firstClosedString);
                bms_state.stringCloseTimeout = BMS_STRING_CLOSE_TIMEOUT;
                bms_state.timer              = BMS_TIME_WAIT_AFTER_STRING_CLOSED;
                bms_state.substate           = BMS_CHECK_CLOSE_FIRST_STRING_PRECHARGE_STATE;
            } else if (bms_state.substate == BMS_CHECK_CLOSE_FIRST_STRING_PRECHARGE_STATE) {
                contstate = CONT_GetState(bms_state.firstClosedString);
                if (contstate == CONT_SWITCH_ON) {
                    bms_state.closedStrings[bms_state.firstClosedString] = 1u;
                    bms_state.numberOfClosedStrings++;
                    bms_state.timer    = BMS_TIME_WAIT_AFTER_STRING_CLOSED;
                    bms_state.substate = BMS_CHECK_ERROR_FLAGS_PRECHARGE_CLOSINGSTRINGS;
                    break;
                } else if (bms_state.stringCloseTimeout == 0u) {
                    /* String takes too long to close */
                    bms_state.timer     = BMS_STATEMACH_SHORTTIME;
                    bms_state.state     = BMS_STATEMACH_OPENCONTACTORS;
                    bms_state.nextstate = BMS_STATEMACH_ERROR;
                    bms_state.substate  = BMS_ENTRY;
                    break;
                } else {
                    /* String not closed, re-issue closing request */
                    CONT_CloseString(bms_state.firstClosedString);
                    bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                    bms_state.substate = BMS_CHECK_ERROR_FLAGS_PRECHARGE_FIRST_STRING;
                    break;
                }
            } else if (bms_state.substate == BMS_CHECK_ERROR_FLAGS_PRECHARGE_FIRST_STRING) {
                if (BMS_IsBatterySystemStateOkay() == STD_NOT_OK) {
                    bms_state.timer     = BMS_STATEMACH_SHORTTIME;
                    bms_state.state     = BMS_STATEMACH_OPENCONTACTORS;
                    bms_state.nextstate = BMS_STATEMACH_ERROR;
                    bms_state.substate  = BMS_ENTRY;
                    break;
                } else {
                    bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                    bms_state.substate = BMS_CHECK_CLOSE_FIRST_STRING_PRECHARGE_STATE;
                    break;
                }
            } else if (bms_state.substate == BMS_CHECK_ERROR_FLAGS_PRECHARGE_CLOSINGSTRINGS) {
                /* Always make one error check after the first string was closed successfully */
                if (BMS_IsBatterySystemStateOkay() == STD_NOT_OK) {
                    bms_state.timer     = BMS_STATEMACH_SHORTTIME;
                    bms_state.state     = BMS_STATEMACH_OPENCONTACTORS;
                    bms_state.nextstate = BMS_STATEMACH_ERROR;
                    bms_state.substate  = BMS_ENTRY;
                    break;
                } else {
                    bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                    bms_state.substate = BMS_PRECHARGE_OPEN_PRECHARGE;
                    break;
                }
            } else if (bms_state.substate == BMS_PRECHARGE_OPEN_PRECHARGE) {
                contRetVal = CONT_OpenPrecharge(bms_state.firstClosedString);
                if (contRetVal == STD_OK) {
                    bms_state.closedPrechargeContactors[stringNumber] = 0u;
                    bms_state.timer                                   = BMS_TIME_WAIT_AFTER_OPENING_PRECHARGE;
                    bms_state.state                                   = BMS_STATEMACH_NORMAL;
                    bms_state.substate                                = BMS_ENTRY;
                } else {
                    bms_state.timer     = BMS_STATEMACH_SHORTTIME;
                    bms_state.state     = BMS_STATEMACH_OPENCONTACTORS;
                    bms_state.nextstate = BMS_STATEMACH_ERROR;
                    bms_state.substate  = BMS_ENTRY;
                }
                break;
            }
            break;

        /****************************NORMAL**************************************/
        case BMS_STATEMACH_NORMAL:
            BMS_SAVELASTSTATES();

            if (bms_state.substate == BMS_ENTRY) {
#if LTC_NORMAL_PERIODIC_OPEN_WIRE_CHECK == TRUE
                nextOpenWireCheck = timestamp + LTC_NORMAL_OPEN_WIRE_PERIOD_ms;
#endif /* LTC_NORMAL_PERIODIC_OPEN_WIRE_CHECK == TRUE */
                DATA_READ_DATA(&systemstate);
                if (bms_state.nextstate == BMS_STATEMACH_CHARGE) {
                    systemstate.bmsCanState = BMS_CANSTATE_CHARGE;
                } else {
                    systemstate.bmsCanState = BMS_CANSTATE_NORMAL;
                }
                DATA_WRITE_DATA(&systemstate);
                bms_state.timer                 = BMS_STATEMACH_SHORTTIME;
                bms_state.substate              = BMS_CHECK_ERROR_FLAGS;
                bms_state.nextstringclosedtimer = 0u;
                break;
            } else if (bms_state.substate == BMS_CHECK_ERROR_FLAGS) {
                if (BMS_IsBatterySystemStateOkay() == STD_NOT_OK) {
                    bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                    bms_state.state    = BMS_STATEMACH_ERROR;
                    bms_state.substate = BMS_ENTRY;
                    break;
                } else {
                    bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                    bms_state.substate = BMS_CHECK_STATE_REQUESTS;
                    break;
                }
            } else if (bms_state.substate == BMS_CHECK_STATE_REQUESTS) {
                if (BMS_CheckCanRequests() == BMS_REQ_ID_STANDBY) {
                    bms_state.timer     = BMS_STATEMACH_SHORTTIME;
                    bms_state.state     = BMS_STATEMACH_OPENCONTACTORS;
                    bms_state.nextstate = BMS_STATEMACH_STANDBY;
                    bms_state.substate  = BMS_ENTRY;
                    break;
                } else {
#if LTC_NORMAL_PERIODIC_OPEN_WIRE_CHECK == TRUE
                    if (nextOpenWireCheck <= timestamp) {
                        MEAS_RequestOpenWireCheck();
                        nextOpenWireCheck = timestamp + LTC_NORMAL_OPEN_WIRE_PERIOD_ms;
                    }
#endif /* LTC_NORMAL_PERIODIC_OPEN_WIRE_CHECK == TRUE */
                    bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                    bms_state.substate = BMS_NORMAL_CLOSE_NEXT_STRING;
                    break;
                }
            } else if (bms_state.substate == BMS_NORMAL_CLOSE_NEXT_STRING) {
                if (bms_state.nextstringclosedtimer == 0u) {
                    nextStringNumber =
                        BMS_GetClosestString(BMS_DO_NOT_TAKE_PRECHARGE_INTO_ACCCOUNT, &bms_tablePackValues);
                    if (nextStringNumber == BMS_NO_STRING_AVAILABLE) {
                        bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                        bms_state.substate = BMS_CHECK_ERROR_FLAGS;
                        break;
                    } else if (
                        (BMS_GetStringVoltageDifference(nextStringNumber, &bms_tablePackValues) <=
                         BMS_NEXT_STRING_VOLTAGE_LIMIT_MV) &&
                        (BMS_GetAverageStringCurrent(&bms_tablePackValues) <= BMS_AVERAGE_STRING_CURRENT_LIMIT_MA)) {
                        /* Voltage/current conditions suitable to close a further string */
                        CONT_CloseString(nextStringNumber);
                        bms_state.nextstringclosedtimer = BMS_STRING_CLOSE_TIMEOUT;
                        bms_state.timer                 = BMS_STATEMACH_SHORTTIME;
                        bms_state.substate              = BMS_CHECK_STRING_CLOSED;
                        break;
                    }
                } else {
                    bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                    bms_state.substate = BMS_CHECK_ERROR_FLAGS;
                    break;
                }
            } else if (bms_state.substate == BMS_CHECK_STRING_CLOSED) {
                contstate = CONT_GetState(nextStringNumber);
                if (contstate == CONT_SWITCH_ON) {
                    bms_state.numberOfClosedStrings++;
                    bms_state.closedStrings[nextStringNumber] = 1u;
                    bms_state.nextstringclosedtimer           = BMS_TIME_WAIT_AFTER_STRING_CLOSED;
                    /* Go to begin of NORMAL case to redo the full procedure with error check and request check */
                    bms_state.substate = BMS_CHECK_ERROR_FLAGS;
                    break;
                } else if (bms_state.stringCloseTimeout == 0u) {
                    /* String takes too long to close */
                    bms_state.timer     = BMS_STATEMACH_SHORTTIME;
                    bms_state.state     = BMS_STATEMACH_OPENCONTACTORS;
                    bms_state.nextstate = BMS_STATEMACH_ERROR;
                    bms_state.substate  = BMS_ENTRY;
                    break;
                } else if (BMS_IsBatterySystemStateOkay() == STD_NOT_OK) {
                    bms_state.timer     = BMS_STATEMACH_SHORTTIME;
                    bms_state.state     = BMS_STATEMACH_OPENCONTACTORS;
                    bms_state.nextstate = BMS_STATEMACH_ERROR;
                    bms_state.substate  = BMS_ENTRY;
                    break;
                } else if (BMS_CheckCanRequests() == BMS_REQ_ID_STANDBY) {
                    bms_state.timer     = BMS_STATEMACH_SHORTTIME;
                    bms_state.state     = BMS_STATEMACH_OPENCONTACTORS;
                    bms_state.nextstate = BMS_STATEMACH_STANDBY;
                    bms_state.substate  = BMS_ENTRY;
                    break;
                } else {
                    /* String not closed, re-issue closing request */
                    CONT_CloseString(nextStringNumber);
                    bms_state.timer = BMS_STATEMACH_SHORTTIME;
                    break;
                }
            }
            break;

        /****************************ERROR*************************************/
        case BMS_STATEMACH_ERROR:
            BMS_SAVELASTSTATES();

            if (bms_state.substate == BMS_ENTRY) {
                /* Set BMS System state to error */
                DATA_READ_DATA(&systemstate);
                systemstate.bmsCanState = BMS_CANSTATE_ERROR;
                DATA_WRITE_DATA(&systemstate);
                /* Deactivate balancing */
                BAL_SetStateRequest(BAL_STATE_NO_BALANCING_REQUEST);
                /* Change LED toggle frequency to indicate an error */
                LED_SetToggleTime(LED_ERROR_OPERATION_ON_OFF_TIME_ms);
                /* Set timer for next open wire check */
                nextOpenWireCheck = timestamp + AFE_ERROR_OPEN_WIRE_PERIOD_ms;
                /* Switch to next substate */
                bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                bms_state.substate = BMS_CHECK_ERROR_FLAGS;
                break;
            } else if (bms_state.substate == BMS_CHECK_ERROR_FLAGS) {
                if (true == DIAG_IsAnyFatalErrorSet()) {
                    /* we stay already in requested state */
                    if (nextOpenWireCheck <= timestamp) {
                        /* Perform open-wire check periodically */
                        /* MEAS_RequestOpenWireCheck(); */ /*TODO: check with strings */
                        nextOpenWireCheck = timestamp + AFE_ERROR_OPEN_WIRE_PERIOD_ms;
                    }
                } else {
                    /* No error detected anymore - reset fatal error related variables */
                    bms_state.minimumActiveDelay_ms  = BMS_NO_ACTIVE_DELAY_TIME_ms;
                    bms_state.minimumActiveDelay_ms  = BMS_NO_ACTIVE_DELAY_TIME_ms;
                    bms_state.transitionToErrorState = false;
                    /* Check for STANDBY request */
                    bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                    bms_state.substate = BMS_CHECK_STATE_REQUESTS;
                    break;
                }
            } else if (bms_state.substate == BMS_CHECK_STATE_REQUESTS) {
                if (BMS_CheckCanRequests() == BMS_REQ_ID_STANDBY) {
                    /* Activate balancing again */
                    BAL_SetStateRequest(BAL_STATE_ALLOWBALANCING_REQUEST);
                    /* Set LED frequency to normal operation as we leave error
                       state subsequently */
                    LED_SetToggleTime(LED_NORMAL_OPERATION_ON_OFF_TIME_ms);

                    /* Verify that all contactors are opened and switch to
                     * STANDBY state afterwards */
                    bms_state.state     = BMS_STATEMACH_OPENCONTACTORS;
                    bms_state.nextstate = BMS_STATEMACH_STANDBY;
                    bms_state.substate  = BMS_ENTRY;
                    break;
                } else {
                    bms_state.timer    = BMS_STATEMACH_SHORTTIME;
                    bms_state.substate = BMS_CHECK_ERROR_FLAGS;
                    break;
                }
            } else {
                /* invalid state -> this should never be reached */
                FAS_ASSERT(FAS_TRAP);
            }
            break;
        default:
            /* invalid state */
            FAS_ASSERT(FAS_TRAP);
            break;
    } /* end switch (bms_state.state) */

    bms_state.triggerentry--;
    bms_state.counter++;
}

extern BMS_CURRENT_FLOW_STATE_e BMS_GetBatterySystemState(void) {
    return bms_state.currentFlowState;
}

extern BMS_CURRENT_FLOW_STATE_e BMS_GetCurrentFlowDirection(int32_t current_mA) {
    BMS_CURRENT_FLOW_STATE_e retVal = BMS_DISCHARGING;

    if (POSITIVE_DISCHARGE_CURRENT == true) {
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
    if (bms_state.closedStrings[stringNumber] == 1u) {
        retval = true;
    }
    return retval;
}

extern bool BMS_IsStringPrecharging(uint8_t stringNumber) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    bool retval = false;
    if (bms_state.closedPrechargeContactors[stringNumber] == 1u) {
        retval = true;
    }
    return retval;
}

extern uint8_t BMS_GetNumberOfConnectedStrings(void) {
    return bms_state.numberOfClosedStrings;
}

extern bool BMS_IsTransitionToErrorStateActive(void) {
    return bms_state.transitionToErrorState;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern BMS_RETURN_TYPE_e TEST_BMS_CheckStateRequest(BMS_STATE_REQUEST_e statereq) {
    return BMS_CheckStateRequest(statereq);
}
extern BMS_STATE_REQUEST_e TEST_BMS_TransferStateRequest(void) {
    return BMS_TransferStateRequest();
}
extern uint8_t TEST_BMS_CheckReEntrance(void) {
    return BMS_CheckReEntrance();
}
extern uint8_t TEST_BMS_CheckCanRequests(void) {
    return BMS_CheckCanRequests();
}
extern bool TEST_BMS_IsAnyFatalErrorFlagSet(void) {
    return BMS_IsAnyFatalErrorFlagSet();
}
extern STD_RETURN_TYPE_e TEST_BMS_IsBatterySystemStateOkay(void) {
    return BMS_IsBatterySystemStateOkay();
}
extern void TEST_BMS_GetMeasurementValues(void) {
    BMS_GetMeasurementValues();
}
extern void TEST_BMS_CheckOpenSenseWire(void) {
    BMS_CheckOpenSenseWire();
}
extern STD_RETURN_TYPE_e TEST_BMS_CheckPrecharge(uint8_t stringNumber, DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    return BMS_CheckPrecharge(stringNumber, pPackValues);
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
extern void TEST_BMS_UpdateBatsysState(DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    BMS_UpdateBatsysState(pPackValues);
}

#endif
