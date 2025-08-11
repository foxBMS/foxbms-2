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
 * @file    bms.h
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup ENGINE
 * @prefix  BMS
 *
 * @brief   BMS driver header
 * @details TODO
 */

#ifndef FOXBMS__BMS_H_
#define FOXBMS__BMS_H_

/*========== Includes =======================================================*/
#include "battery_system_cfg.h"
#include "bms_cfg.h"

#include "contactor.h"
#include "fstd_types.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** Symbolic names for battery system state */
typedef enum {
    BMS_CHARGING,    /*!< battery is charged */
    BMS_DISCHARGING, /*!< battery is discharged */
    BMS_RELAXATION,  /*!< battery relaxation ongoing */
    BMS_AT_REST,     /*!< battery is resting */
} BMS_CURRENT_FLOW_STATE_e;

/** Symbolic names for busyness of the BMS control */
typedef enum {
    BMS_CHECK_OK,     /*!< BMS control ok */
    BMS_CHECK_BUSY,   /*!< BMS control busy */
    BMS_CHECK_NOT_OK, /*!< BMS control not ok */
} BMS_CHECK_e;

/** Symbolic names to take precharge into account or not */
typedef enum {
    BMS_DO_NOT_TAKE_PRECHARGE_INTO_ACCOUNT, /*!< do not take precharge into account */
    BMS_TAKE_PRECHARGE_INTO_ACCOUNT,        /*!< do take precharge into account */
} BMS_CONSIDER_PRECHARGE_e;

/** States of the BMS state machine */
typedef enum {
    /* Init-Sequence */
    BMS_STATEMACH_UNINITIALIZED,
    BMS_STATEMACH_INITIALIZATION,
    BMS_STATEMACH_INITIALIZED,
    BMS_STATEMACH_IDLE,
    BMS_STATEMACH_OPEN_CONTACTORS,
    BMS_STATEMACH_STANDBY,
    BMS_STATEMACH_PRECHARGE,
    BMS_STATEMACH_NORMAL,
    BMS_STATEMACH_DISCHARGE,
    BMS_STATEMACH_CHARGE,
    BMS_STATEMACH_ERROR,
    BMS_STATEMACH_UNDEFINED,
    BMS_STATEMACH_RESERVED1,
} BMS_STATEMACH_e;

/** CAN states of the BMS state machine */
typedef enum {
    /* Init-Sequence */
    BMS_CAN_STATE_UNINITIALIZED,
    BMS_CAN_STATE_INITIALIZATION,
    BMS_CAN_STATE_INITIALIZED,
    BMS_CAN_STATE_IDLE,
    BMS_CAN_STATE_OPEN_CONTACTORS,
    BMS_CAN_STATE_STANDBY,
    BMS_CAN_STATE_PRECHARGE,
    BMS_CAN_STATE_NORMAL,
    BMS_CAN_STATE_CHARGE,
    BMS_CAN_STATE_ERROR,
} BMS_CAN_STATE_e;

/** Substates of the BMS state machine */
typedef enum {
    BMS_ENTRY,                        /*!< Substate entry state */
    BMS_CHECK_ERROR_FLAGS_INTERLOCK,  /*!< Substate check measurements after interlock closed */
    BMS_INTERLOCK_CHECKED,            /*!< Substate interlocked checked */
    BMS_CHECK_STATE_REQUESTS,         /*!< Substate check if there is a state request */
    BMS_CHECK_BALANCING_REQUESTS,     /*!< Substate check if there is a balancing request */
    BMS_CHECK_ERROR_FLAGS,            /*!< Substate check if any error flag set */
    BMS_CHECK_CONTACTOR_NORMAL_STATE, /*!< Substate in precharge, check if there contactors reached normal */
    BMS_CHECK_CONTACTOR_CHARGE_STATE, /*!< Substate in precharge, check if there contactors reached normal */
    BMS_PRECHARGE_CLOSE_MINUS,
    BMS_PRECHARGE_CLOSE_PRECHARGE,
    BMS_PRECHARGE_CHECK_VOLTAGES,
    BMS_PRECHARGE_OPEN_PRECHARGE,
    BMS_PRECHARGE_CHECK_OPEN_PRECHARGE,
    BMS_OPEN_FIRST_CONTACTOR,
    BMS_OPEN_SECOND_CONTACTOR_MINUS,
    BMS_OPEN_SECOND_CONTACTOR_PLUS,
    BMS_CHECK_CLOSE_SECOND_STRING_CONTACTOR_PRECHARGE_STATE,
    BMS_CHECK_ERROR_FLAGS_PRECHARGE,
    BMS_CHECK_ERROR_FLAGS_PRECHARGE_FIRST_STRING,
    BMS_PRECHARGE_CLOSE_NEXT_STRING,
    BMS_CLOSE_SECOND_CONTACTOR_PLUS,
    BMS_CHECK_STRING_CLOSED,
    BMS_CHECK_ERROR_FLAGS_PRECHARGE_CLOSING_STRINGS,
    BMS_CHECK_ERROR_FLAGS_CLOSING_PRECHARGE,
    BMS_NORMAL_CLOSE_NEXT_STRING,
    BMS_NORMAL_CLOSE_SECOND_STRING_CONTACTOR,
    BMS_OPEN_ALL_PRECHARGE_CONTACTORS,
    BMS_CHECK_ALL_PRECHARGE_CONTACTORS_OPEN,
    BMS_OPEN_STRINGS_ENTRY,
    BMS_OPEN_FIRST_STRING_CONTACTOR,
    BMS_OPEN_SECOND_STRING_CONTACTOR,
    BMS_CHECK_SECOND_STRING_CONTACTOR,
    BMS_HANDLE_SUPPLY_VOLTAGE_30C_LOSS,
    BMS_OPEN_STRINGS_EXIT,
} BMS_STATEMACH_SUB_e;

/** State requests for the BMS state machine */
typedef enum {
    BMS_STATE_INIT_REQUEST,  /*!< request for initialization */
    BMS_STATE_ERROR_REQUEST, /*!< request for ERROR state */
    BMS_STATE_NO_REQUEST,    /*!< dummy request for no request */
} BMS_STATE_REQUEST_e;

/** Possible return values when state requests are made to the BMS state machine */
typedef enum {
    BMS_OK,                  /*!< request was successful */
    BMS_REQUEST_PENDING,     /*!< error: another request is currently processed */
    BMS_ILLEGAL_REQUEST,     /*!< error: request can not be executed */
    BMS_ALREADY_INITIALIZED, /*!< error: BMS state machine already initialized */
} BMS_RETURN_TYPE_e;

/** Power path type (discharge or charge) */
typedef enum {
    BMS_POWER_PATH_OPEN, /* contactors open */
    BMS_POWER_PATH_0,    /* power path */
    BMS_POWER_PATH_1,    /* second power path */
} BMS_POWER_PATH_TYPE_e;

/**
 * This structure contains all the variables relevant for the CONT state
 * machine. The user can get the current state of the CONT state machine with
 * this variable
 */
typedef struct {
    uint16_t timer; /*!< time in ms before the state machine processes the next state, e.g. in counts of 1ms */
    BMS_STATE_REQUEST_e stateRequest;           /*!< current state request made to the state machine */
    BMS_STATEMACH_e state;                      /*!< current state of State Machine */
    BMS_STATEMACH_SUB_e substate;               /*!< current substate of the state machine */
    BMS_STATEMACH_e lastState;                  /*!< previous state of the state machine */
    BMS_STATEMACH_SUB_e lastSubstate;           /*!< previous substate of the state machine */
    uint32_t ErrRequestCounter;                 /*!< counts the number of illegal requests to the LTC state machine */
    STD_RETURN_TYPE_e initFinished;             /*!< #STD_OK if the initialization has passed, #STD_NOT_OK otherwise */
    uint8_t triggerentry;                       /*!< counter for re-entrance protection (function running flag) */
    uint8_t counter;                            /*!< general purpose counter */
    BMS_CURRENT_FLOW_STATE_e currentFlowState;  /*!< state of battery system */
    uint32_t restTimer_10ms;                    /*!< timer until battery system is at rest */
    uint16_t OscillationTimeout;                /*!< timeout to prevent oscillation of contactors */
    uint8_t prechargeTryCounter;                /*!< timeout to prevent oscillation of contactors */
    BMS_POWER_PATH_TYPE_e powerPath;            /*!< power path type (discharge or charge) */
    uint8_t numberOfClosedStrings;              /*!< number of closed strings */
    uint16_t stringOpenTimeout;                 /*!< timeout to abort if string opening takes too long */
    uint32_t nextStringClosedTimer;             /*!< timer to wait if the next string was closed */
    uint16_t stringCloseTimeout;                /*!< timeout to abort if a string takes too long to close */
    BMS_STATEMACH_e nextState;                  /*!< next state of the State Machine */
    uint8_t firstClosedString;                  /*!< strings with highest or lowest voltage, that was closed first */
    uint16_t prechargeOpenTimeout;              /*!< timeout to abort if string opening takes too long */
    uint16_t prechargeCloseTimeout;             /*!< timeout to abort if a string takes too long to close */
    uint32_t remainingDelay_ms;                 /*!< time until state machine should switch to error state */
    uint32_t minimumActiveDelay_ms;             /*!< minimum delay time of all active fatal errors */
    uint32_t timeAboveContactorBreakCurrent_ms; /*!< duration of current flow above maximum contactor break current */
    uint8_t stringToBeOpened;                   /*!< string that is currently opened */
    CONT_TYPE_e contactorToBeOpened;            /*!< contactor that is currently opened */
    bool transitionToErrorState;                /*!< flag if fatal error has been detected and delay is active */
    uint8_t closedPrechargeContactors[BS_NR_OF_STRINGS]; /*!< strings whose precharge contactors are closed */
    uint8_t closedStrings[BS_NR_OF_STRINGS];             /*!< strings whose contactors are closed */
    uint8_t deactivatedStrings[BS_NR_OF_STRINGS]; /*!< Deactivated strings after error detection, cannot be closed */
} BMS_STATE_s;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   sets the current state request of the state variable bms_state.
 * @details This function is used to make a state request to the state machine,
 *          e.g, start voltage measurement, read result of voltage measurement,
 *          re-initialization.
 *          It calls #BMS_CheckStateRequest() to check if the request is valid.
 *          The state request is rejected if is not valid. The result of the
 *          check is returned immediately, so that the requester can act in
 *          case it made a non-valid state request.
 * @param   statereq    state request to set
 * @return  current state request
 */
extern BMS_RETURN_TYPE_e BMS_SetStateRequest(BMS_STATE_REQUEST_e statereq);

/**
 * @brief   Returns the current state.
 * @details This function is used in the functioning of the SYS state machine.
 * @return  current state, taken from BMS_STATEMACH_e
 */
extern BMS_STATEMACH_e BMS_GetState(void);

/**
 * @brief   Returns the current substate.
 * @details This function is used in the functioning of the SYS state machine.
 * @return  current substate, taken from BMS_STATEMACH_SUB_e
 */
extern BMS_STATEMACH_SUB_e BMS_GetSubstate(void);

/**
 * @brief   Gets the initialization state.
 * @details This function is used for getting the BMS initialization state.
 * @return  #STD_OK if initialized, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e BMS_GetInitializationState(void);

/**
 * @brief   trigger function for the BMS driver state machine.
 * @details This function contains the sequence of events in the BMS state
 *          machine.
 *          It must be called time-triggered, every 10 milliseconds.
 *          This function needs to be adapted to be adapted to the behavior
 *          the batter system shall provide to the target application.
 */
extern void BMS_Trigger(void);

/**
 * @brief   Returns current battery system state (charging/discharging,
 *          resting or in relaxation phase)
 *
 * @return  #BMS_CURRENT_FLOW_STATE_e
 */
extern BMS_CURRENT_FLOW_STATE_e BMS_GetBatterySystemState(void);

/**
 * @brief   Get current flow direction, current value as function parameter
 * @param[in]   current_mA current that is flowing
 * @return  #BMS_DISCHARGING or #BMS_CHARGING depending on current direction.
 *          Return #BMS_AT_REST. ((type: #BMS_CURRENT_FLOW_STATE_e)
 */
extern BMS_CURRENT_FLOW_STATE_e BMS_GetCurrentFlowDirection(int32_t current_mA);

/**
 * @brief   Returns string state (closed or open)
 * @param[in]   stringNumber   string addressed
 * @return  false if string is open, true if string is closed
 */
extern bool BMS_IsStringClosed(uint8_t stringNumber);

/**
 * @brief   Returns if string is currently precharging or not
 * @param[in]   stringNumber   string addressed
 * @return  false if precharge contactor is open, true if closed and string is
 *          precharging
 */
extern bool BMS_IsStringPrecharging(uint8_t stringNumber);

/**
 * @brief   Returns number of connected strings
 * @return  Returns number of connected strings
 */
extern uint8_t BMS_GetNumberOfConnectedStrings(void);

/**
 * @brief   Check if transition in to error state is active
 * @return  True, if transition into error state is ongoing, otherwise false
 */
extern bool BMS_IsTransitionToErrorStateActive(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
/* database.h is only included in bms.c and there used as function parameter
 * for static functions. Thus, we need to add the required include here. */
#include "database.h"

extern BMS_RETURN_TYPE_e TEST_BMS_CheckStateRequest(BMS_STATE_REQUEST_e statereq);
extern BMS_STATE_REQUEST_e TEST_BMS_TransferStateRequest(void);
extern uint8_t TEST_BMS_CheckReEntrance(void);
extern uint8_t TEST_BMS_CheckCanRequests(void);
extern STD_RETURN_TYPE_e TEST_BMS_IsBatterySystemStateOkay(void);
extern bool TEST_BMS_IsContactorFeedbackValid(uint8_t stringNumber, CONT_TYPE_e contactorType);
extern bool TEST_BMS_IsAnyFatalErrorFlagSet(void);
extern void TEST_BMS_GetMeasurementValues(void);
extern void TEST_BMS_CheckOpenSenseWire(void);
extern STD_RETURN_TYPE_e TEST_BMS_CheckPrecharge(uint8_t stringNumber, DATA_BLOCK_PACK_VALUES_s *pPackValues);
extern uint8_t TEST_BMS_GetHighestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues);
extern uint8_t TEST_BMS_GetClosestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues);
extern uint8_t TEST_BMS_GetLowestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues);
extern int32_t TEST_BMS_GetStringVoltageDifference(uint8_t string, DATA_BLOCK_PACK_VALUES_s *pPackValues);
extern int32_t TEST_BMS_GetAverageStringCurrent(DATA_BLOCK_PACK_VALUES_s *pPackValues);
extern void TEST_BMS_UpdateBatterySystemState(DATA_BLOCK_PACK_VALUES_s *pPackValues);
#endif

#endif /* FOXBMS__BMS_H_ */
