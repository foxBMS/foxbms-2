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
 * @file    bms.h
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup ENGINE
 * @prefix  BMS
 *
 * @brief   Declare the BMS driver interface
 * @details TODO
 */

#ifndef FOXBMS__BMS_H_
#define FOXBMS__BMS_H_

/*========== Includes =======================================================*/
#include "battery_system_cfg.h"
#include "bms_cfg.h"

#include "contactor.h"
#include "fstd_types.h"

#ifdef UNITY_UNIT_TEST
#include "database.h"
#endif /* UNITY_UNIT_TEST */

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

typedef enum {
    BMS_PRECHARGING_ONGOING,
    BMS_PRECHARGING_FAILED,
    BMS_PRECHARGING_FINISHED,
    BMS_PRECHARING_HAS_NOT_STARTED,
} BMS_RESULT_PRECHARGE_PROCESS_e;

/** States of the BMS state machine */
typedef enum {
    /* Init-Sequence */
    BMS_FSM_STATE_DUMMY,          /*!< dummy state - always the first state */
    BMS_FSM_STATE_HAS_NEVER_RUN,  /*!< never run state - always the second state */
    BMS_FSM_STATE_UNINITIALIZED,  /*!< uninitialized state */
    BMS_FSM_STATE_INITIALIZATION, /*!< initializing the state machine */
    BMS_FSM_STATE_INITIALIZED,
    BMS_FSM_STATE_IDLE,
    BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR,   /*!< open contactors to transition to error state */
    BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY, /*!< open contactors to transition to standby state */
    BMS_FSM_STATE_STANDBY,
    BMS_FSM_STATE_PRECHARGE,
    BMS_FSM_STATE_NORMAL,
    BMS_FSM_STATE_ERROR, /*!< state for error processing  */
} BMS_FSM_STATES_e;

/** Substates of the BMS state machine */
typedef enum {
    BMS_FSM_SUBSTATE_DUMMY,                       /*!< dummy state - always the first substate */
    BMS_FSM_SUBSTATE_ENTRY,                       /*!< Substate entry state */
    BMS_FSM_SUBSTATE_INITIALIZATION_IMD,          /*!< Substate initialization first fast IMD check */
    BMS_FSM_SUBSTATE_INITIALIZATION_EXIT,         /*!< Substate initialization exit state */
    BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_INTERLOCK, /*!< Substate check measurements after interlock closed */
    BMS_FSM_SUBSTATE_INTERLOCK_CHECKED,           /*!< Substate interlocked checked */
    BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS,        /*!< Substate check if there is a state request */
    BMS_FSM_SUBSTATE_CHECK_BALANCING_REQUESTS,    /*!< Substate check if there is a balancing request */
    BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS,           /*!< Substate check if any error flag set */
    BMS_FSM_SUBSTATE_CHECK_CONTACTOR_NORMAL_STATE, /*!< Substate in precharge, check if there contactors reached normal */
    BMS_FSM_SUBSTATE_CHECK_CONTACTOR_CHARGE_STATE, /*!< Substate in precharge, check if there contactors reached normal */
    BMS_FSM_SUBSTATE_PRECHARGE_CLOSE_MINUS,
    BMS_FSM_SUBSTATE_PRECHARGE_CLOSE_PRECHARGE,
    BMS_FSM_SUBSTATE_PRECHARGE_CHECK_PRECHARGE_PROCESS,
    BMS_FSM_SUBSTATE_PRECHARGE_OPEN_PRECHARGE,
    BMS_FSM_SUBSTATE_PRECHARGE_CHECK_OPEN_PRECHARGE,
    BMS_FSM_SUBSTATE_OPEN_FIRST_CONTACTOR,
    BMS_FSM_SUBSTATE_OPEN_SECOND_CONTACTOR_MINUS,
    BMS_FSM_SUBSTATE_OPEN_SECOND_CONTACTOR_PLUS,
    BMS_FSM_SUBSTATE_CHECK_CLOSE_SECOND_STRING_CONTACTOR_PRECHARGE_STATE,
    BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_PRECHARGE,
    BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_PRECHARGE_FIRST_STRING,
    BMS_FSM_SUBSTATE_PRECHARGE_CLOSE_NEXT_STRING,
    BMS_FSM_SUBSTATE_CLOSE_SECOND_CONTACTOR_PLUS,
    BMS_FSM_SUBSTATE_NORMAL_CHECK_STRING_CLOSED,
    BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_PRECHARGE_CLOSING_STRINGS,
    BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_CLOSING_PRECHARGE,
    BMS_FSM_SUBSTATE_NORMAL_CLOSE_NEXT_STRING,
    BMS_FSM_SUBSTATE_NORMAL_CLOSE_SECOND_STRING_CONTACTOR,
    BMS_FSM_SUBSTATE_OPEN_ALL_PRECHARGE_CONTACTORS,
    BMS_FSM_SUBSTATE_CHECK_ALL_PRECHARGE_CONTACTORS_OPEN,
    BMS_FSM_SUBSTATE_OPEN_STRINGS_ENTRY,
    BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR,
    BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR,
    BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR,
    BMS_FSM_SUBSTATE_HANDLE_SUPPLY_VOLTAGE_30C_LOSS,
    BMS_FSM_SUBSTATE_OPEN_STRINGS_EXIT,
} BMS_FSM_SUBSTATES_e;

/** CAN states of the BMS state machine */
typedef enum {
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

/** State requests for the BMS state machine */
typedef enum {
    BMS_STATE_INITIALIZATION_REQUEST, /*!< request for initialization */
    BMS_STATE_ERROR_REQUEST,          /*!< request for ERROR state */
    BMS_STATE_NO_REQUEST,             /*!< dummy request for no request */
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
 * This structure contains all the additional information relevant for the BMS state
 * machine.
 */
typedef struct {
    uint8_t stringNumber;
    uint32_t currentSystick;                   /*!< current system timestamp. Updated with every call of #BMS_Trigger */
    uint32_t ErrRequestCounter;                /*!< counts the number of illegal requests to the AFE state machine */
    STD_RETURN_TYPE_e initFinished;            /*!< #STD_OK if the initialization has passed, #STD_NOT_OK otherwise */
    uint8_t counter;                           /*!< general purpose counter */
    BMS_CURRENT_FLOW_STATE_e currentFlowState; /*!< state of battery system */
    uint32_t restTimer_10ms;                   /*!< timer until battery system is at rest */
    uint16_t OscillationTimeout;               /*!< timeout to prevent oscillation of contactors */
    uint8_t prechargeTryCounter;               /*!< timeout to prevent oscillation of contactors */
    BMS_POWER_PATH_TYPE_e powerPath;           /*!< power path type (discharge or charge) */
    uint8_t numberOfClosedStrings;             /*!< number of closed strings */
    uint16_t stringOpenTimeout;                /*!< timeout to abort if string opening takes too long */
    uint32_t nextStringClosedTimer;            /*!< timer to wait for the next string to be closed */
    uint16_t stringCloseTimeout;               /*!< timeout to abort if a string takes too long to close */
    uint8_t firstClosedString;                 /*!< strings with highest or lowest voltage, that was closed first */
    uint16_t prechargeOpenTimeout;             /*!< timeout to abort if string opening takes too long */
    uint16_t prechargeCloseTimeout;            /*!< timeout to abort if a string takes too long to close */
    uint32_t remainingDelay_ms;                /*!< time until state machine should switch to error state */
    uint32_t minimumActiveDelay_ms;            /*!< minimum delay time of all active fatal errors */
    uint32_t timeAboveContactorBreakCurrent_ms; /*!< duration of current flow above maximum contactor break current */
    uint8_t stringToBeOpened;                   /*!< string that is currently opened */
    CONT_TYPE_e contactorToBeOpened;            /*!< contactor that is currently opened */
    uint32_t startOfPrecharging;                /*!< systick, when precharging has been initiated */
    bool transitionToErrorState;                /*!< flag if fatal error has been detected and delay is active */
    bool closedPrechargeContactors[BS_NR_OF_STRINGS]; /*!< strings whose precharge contactors are closed */
    bool closedStrings[BS_NR_OF_STRINGS];             /*!< strings whose contactors are closed */
    bool deactivatedStrings[BS_NR_OF_STRINGS]; /*!< Deactivated strings after error detection, cannot be closed */
    uint8_t nextStringNumber;
} BMS_INFORMATION_s;

/**
 * This structure contains all the variables relevant for the BMS state
 * machine. The user can get the current state of the CONT state machine with
 * this variable
 */
typedef struct {
    uint16_t timer;                       /*!< timer of the state */
    uint8_t triggerEntry;                 /*!< trigger entry of the state */
    BMS_STATE_REQUEST_e stateRequest;     /*!< current state request made to the state machine */
    BMS_FSM_STATES_e nextState;           /*!< next state of the FSM */
    BMS_FSM_STATES_e currentState;        /*!< current state of the FSM */
    BMS_FSM_STATES_e previousState;       /*!< previous state of the FSM */
    BMS_FSM_SUBSTATES_e nextSubstate;     /*!< next substate of the FSM */
    BMS_FSM_SUBSTATES_e currentSubstate;  /*!< current substate of the FSM */
    BMS_FSM_SUBSTATES_e previousSubstate; /*!< previous substate of the FSM */
    BMS_INFORMATION_s information;        /*!< Some information to be stored */
} BMS_STATE_s;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   sets the current state request of the state variable bms_state.
 * @details Make a state request to the state machine,
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
 * @details Use this getter in the functioning of the SYS state machine.
 * @return  current state, taken from BMS_FSM_STATES_e
 */
extern BMS_FSM_STATES_e BMS_GetState(void);

/**
 * @brief   Returns the current substate.
 * @details Use this getter in the functioning of the SYS state machine.
 * @return  current substate, taken from BMS_FSM_SUBSTATES_e
 */
extern BMS_FSM_SUBSTATES_e BMS_GetSubstate(void);

/**
 * @brief   Gets the initialization state.
 * @details Get the BMS initialization state.
 * @return  #STD_OK if initialized, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e BMS_GetInitializationState(void);

/**
 * @brief   trigger function for the BMS driver state machine.
 * @details Execute the sequence of events in the BMS state
 *          machine.
 *          It must be called time-triggered, every 10 milliseconds.
 *          This function needs to be adapted to be adapted to the behavior
 *          the batter system shall provide to the target application.
 * @return  #STD_NOT_OK on re-entrance of the function, otherwise #STD_OK
 */
extern STD_RETURN_TYPE_e BMS_Trigger(void);

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
extern DATA_BLOCK_MIN_MAX_s *TEST_BMS_GetMinMaxTable(void);
extern DATA_BLOCK_OPEN_WIRE_s *TEST_BMS_GetOpenWireTable(void);
extern DATA_BLOCK_PACK_VALUES_s *TEST_BMS_GetPackValuesTable(void);
extern void TEST_SetTablePackValues(DATA_BLOCK_PACK_VALUES_s *packValues);
extern BMS_RETURN_TYPE_e TEST_BMS_CheckStateRequest(BMS_STATE_REQUEST_e statereq);
extern BMS_STATE_REQUEST_e TEST_BMS_TransferStateRequest(void);
extern uint8_t TEST_BMS_CheckReEntrance(void);
extern uint8_t TEST_BMS_CheckCanRequests(void);
extern STD_RETURN_TYPE_e TEST_BMS_IsBatterySystemStateOkay(BMS_STATE_s *pBmsState);
extern bool TEST_BMS_IsContactorFeedbackValid(uint8_t stringNumber, CONT_TYPE_e contactorType);
extern bool TEST_BMS_IsAnyFatalErrorFlagSet(void);
extern void TEST_BMS_GetMeasurementValues(void);
extern void TEST_BMS_CheckOpenSenseWire(void);
extern BMS_RESULT_PRECHARGE_PROCESS_e TEST_BMS_MonitorPrechargeProcess(
    BMS_STATE_s *pBmsState,
    uint8_t stringNumber,
    const DATA_BLOCK_PACK_VALUES_s *pPackValues,
    BS_PRECHARGE_MONITORING_e monitoringParameters,
    uint32_t timeout_ms);
extern STD_RETURN_TYPE_e TEST_BMS_CheckPrecharge(uint8_t stringNumber, DATA_BLOCK_PACK_VALUES_s *pPackValues);
extern uint8_t TEST_BMS_GetHighestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues);
extern uint8_t TEST_BMS_GetClosestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues);
extern uint8_t TEST_BMS_GetLowestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues);
extern int32_t TEST_BMS_GetStringVoltageDifference(uint8_t string, DATA_BLOCK_PACK_VALUES_s *pPackValues);
extern int32_t TEST_BMS_GetAverageStringCurrent(DATA_BLOCK_PACK_VALUES_s *pPackValues);
extern BMS_FSM_STATES_e TEST_BMS_ProcessInitializedState(BMS_STATE_s *pBmsState);
extern BMS_FSM_STATES_e TEST_BMS_ProcessIdleState(BMS_STATE_s *pBmsState);
extern BMS_FSM_STATES_e TEST_BMS_ProcessOpenContactorsToStandbyState(BMS_STATE_s *pBmsState);
extern BMS_FSM_STATES_e TEST_BMS_ProcessStandbyState(BMS_STATE_s *pBmsState);
extern BMS_FSM_STATES_e TEST_BMS_ProcessPrechargeState(BMS_STATE_s *pBmsState);
extern BMS_FSM_STATES_e TEST_BMS_ProcessNormalState(BMS_STATE_s *pBmsState);
extern BMS_FSM_STATES_e TEST_BMS_ProcessOpenContactorsToErrorState(BMS_STATE_s *pBmsState);
extern BMS_FSM_STATES_e TEST_BMS_ProcessErrorState(BMS_STATE_s *pBmsState);
extern STD_RETURN_TYPE_e TEST_BMS_RunStateMachine(BMS_STATE_s *pBmsState);
extern void TEST_BMS_UpdateBatterySystemState(DATA_BLOCK_PACK_VALUES_s *pPackValues);
#endif

#endif /* FOXBMS__BMS_H_ */
