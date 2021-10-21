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
 * @file    bms.h
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2021-07-29 (date of last update)
 * @ingroup ENGINE
 * @prefix  BMS
 *
 * @brief   bms driver header
 *
 *
 */

#ifndef FOXBMS__BMS_H_
#define FOXBMS__BMS_H_

/*========== Includes =======================================================*/
#include "battery_system_cfg.h"
#include "bms_cfg.h"

/*========== Macros and Definitions =========================================*/

/** Symbolic names for battery system state */
typedef enum BMS_CURRENT_FLOW_STATE {
    BMS_CHARGING,    /*!< battery is charged */
    BMS_DISCHARGING, /*!< battery is discharged */
    BMS_RELAXATION,  /*!< battery relaxation ongoing */
    BMS_AT_REST,     /*!< battery is resting */
} BMS_CURRENT_FLOW_STATE_e;

/** Symbolic names for busyness of the syscontrol */
typedef enum BMS_CHECK {
    BMS_CHECK_OK,     /*!< syscontrol ok */
    BMS_CHECK_BUSY,   /*!< syscontrol busy */
    BMS_CHECK_NOT_OK, /*!< syscontrol not ok */
} BMS_CHECK_e;

/** Symbolic names to take precharge into account or not */
typedef enum BMS_CONSIDER_PRECHARGE {
    BMS_DO_NOT_TAKE_PRECHARGE_INTO_ACCCOUNT, /*!< do not take precharge into account */
    BMS_TAKE_PRECHARGE_INTO_ACCCOUNT,        /*!< do take precharge into account */
} BMS_CONSIDER_PRECHARGE_e;

/** States of the SYS state machine */
typedef enum BMS_STATEMACH {
    /* Init-Sequence */
    BMS_STATEMACH_UNINITIALIZED,
    BMS_STATEMACH_INITIALIZATION,
    BMS_STATEMACH_INITIALIZED,
    BMS_STATEMACH_IDLE,
    BMS_STATEMACH_OPENCONTACTORS,
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
typedef enum BMS_CANSTATE {
    /* Init-Sequence */
    BMS_CANSTATE_UNINITIALIZED,
    BMS_CANSTATE_INITIALIZATION,
    BMS_CANSTATE_INITIALIZED,
    BMS_CANSTATE_IDLE,
    BMS_CANSTATE_OPENCONTACTORS,
    BMS_CANSTATE_STANDBY,
    BMS_CANSTATE_PRECHARGE,
    BMS_CANSTATE_NORMAL,
    BMS_CANSTATE_CHARGE,
    BMS_CANSTATE_ERROR,
} BMS_CANSTATE_e;

/** Substates of the SYS state machine */
typedef enum BMS_STATEMACH_SUB {
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
    BMS_OPEN_FIRST_CONTACTOR,
    BMS_OPEN_SECOND_CONTACTOR_MINUS,
    BMS_OPEN_SECOND_CONTACTOR_PLUS,
    BMS_CLOSE_FIRST_STRING_PRECHARGE_STATE,
    BMS_CHECK_CLOSE_FIRST_STRING_PRECHARGE_STATE,
    BMS_CHECK_ERROR_FLAGS_PRECHARGE,
    BMS_CHECK_ERROR_FLAGS_PRECHARGE_FIRST_STRING,
    BMS_PRECHARGE_CLOSE_NEXT_STRING,
    BMS_CHECK_STRING_CLOSED,
    BMS_CHECK_ERROR_FLAGS_PRECHARGE_CLOSINGSTRINGS,
    BMS_CHECK_ERROR_FLAGS_CLOSINGPRECHARGE,
    BMS_NORMAL_CLOSE_NEXT_STRING,
    BMS_OPEN_ALL_PRECHARGES,
    BMS_CHECK_ALL_PRECHARGES_OPEN,
    BMS_OPEN_STRINGS_ENTRY,
    BMS_OPEN_STRINGS,
    BMS_CHECK_STRING_OPEN,
    BMS_OPEN_STRINGS_EXIT,
} BMS_STATEMACH_SUB_e;

/** State requests for the BMS statemachine */
typedef enum BMS_STATE_REQUEST {
    BMS_STATE_INIT_REQUEST,  /*!< request for initialization */
    BMS_STATE_ERROR_REQUEST, /*!< request for ERROR state */
    BMS_STATE_NO_REQUEST,    /*!< dummy request for no request */
} BMS_STATE_REQUEST_e;

/**
 * Possible return values when state requests are made to the SYS statemachine
 */
typedef enum BMS_RETURN_TYPE {
    BMS_OK,                  /*!< CONT --> ok */
    BMS_BUSY_OK,             /*!< CONT under load --> ok */
    BMS_REQUEST_PENDING,     /*!< requested to be executed */
    BMS_ILLEGAL_REQUEST,     /*!< Request can not be executed */
    BMS_ALREADY_INITIALIZED, /*!< Initialization of LTC already finished */
    BMS_ILLEGAL_TASK_TYPE,   /*!< Illegal */
} BMS_RETURN_TYPE_e;

/** Power path type (discharge or charge) */
typedef enum BMS_POWER_PATH_TYPE {
    BMS_POWER_PATH_OPEN, /* contactors open */
    BMS_POWER_PATH_0,    /* power path */
    BMS_POWER_PATH_1,    /* second power path */
} BMS_POWER_PATH_TYPE_e;

/**
 * This structure contains all the variables relevant for the CONT state
 * machine. The user can get the current state of the CONT state machine with
 * this variable
 */
typedef struct BMS_STATE {
    uint16_t timer; /*!< time in ms before the state machine processes the next state, e.g. in counts of 1ms */
    BMS_STATE_REQUEST_e stateRequest;          /*!< current state request made to the state machine */
    BMS_STATEMACH_e state;                     /*!< current state of State Machine */
    BMS_STATEMACH_SUB_e substate;              /*!< current substate of the state machine */
    BMS_STATEMACH_e laststate;                 /*!< previous state of the state machine */
    BMS_STATEMACH_SUB_e lastsubstate;          /*!< previous substate of the state machine */
    uint32_t ErrRequestCounter;                /*!< counts the number of illegal requests to the LTC state machine */
    STD_RETURN_TYPE_e initFinished;            /*!< #STD_OK if the initialization has passed, #STD_NOT_OK otherwise */
    uint8_t triggerentry;                      /*!< counter for re-entrance protection (function running flag) */
    uint8_t counter;                           /*!< general purpose counter */
    BMS_CURRENT_FLOW_STATE_e currentFlowState; /*!< state of battery system */
    uint32_t restTimer_10ms;                   /*!< timer until battery system is at rest */
    uint16_t OscillationTimeout;               /*!< timeout to prevent oscillation of contactors */
    uint8_t PrechargeTryCounter;               /*!< timeout to prevent oscillation of contactors */
    BMS_POWER_PATH_TYPE_e powerPath;           /*!< power path type (discharge or charge) */
    uint8_t numberOfClosedStrings;             /*!< number of closed strings */
    uint16_t stringOpenTimeout;                /*!< timeout to abort if string opening takes too long */
    uint32_t nextstringclosedtimer;            /*!< timer to wait if the next string was closed */
    uint16_t stringCloseTimeout;               /*!< timeout to abort if a string takes too long to close */
    BMS_STATEMACH_e nextstate;                 /*!< next state of the State Machine */
    uint8_t firstClosedString;                 /*!< strings with highest or lowest voltage, that was closed first */
    uint16_t prechargeOpenTimeout;             /*!< timeout to abort if string opening takes too long */
    uint16_t prechargeCloseTimeout;            /*!< timeout to abort if a string takes too long to close */
    uint32_t remainingDelay_ms;                /*!< time until statemachine should switch to error state */
    uint32_t minimumActiveDelay_ms;            /*!< minimum delay time of all active fatal errors */
    bool transitionToErrorState;               /*!< flag if fatal error has been detected and delay is active */
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
 * @brief   Gets the initialization state.
 * @details This function is used for getting the BMS initialization state.
 * @return  #STD_OK if initialized, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e BMS_GetInitializationState(void);

/**
 * @brief   trigger function for the SYS driver state machine.
 * @details This function contains the sequence of events in the SYS state
 *          machine. It must be called time-triggered, every 10 milliseconds.
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
extern bool TEST_BMS_IsAnyFatalErrorFlagSet(void);
extern void TEST_BMS_GetMeasurementValues(void);
extern void TEST_BMS_CheckOpenSenseWire(void);
extern STD_RETURN_TYPE_e TEST_BMS_CheckPrecharge(uint8_t stringNumber, DATA_BLOCK_PACK_VALUES_s *pPackValues);
extern uint8_t TEST_BMS_GetHighestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues);
extern uint8_t TEST_BMS_GetClosestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues);
extern uint8_t TEST_BMS_GetLowestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues);
extern int32_t TEST_BMS_GetStringVoltageDifference(uint8_t string, DATA_BLOCK_PACK_VALUES_s *pPackValues);
extern int32_t TEST_BMS_GetAverageStringCurrent(DATA_BLOCK_PACK_VALUES_s *pPackValues);
extern void TEST_BMS_UpdateBatsysState(DATA_BLOCK_PACK_VALUES_s *pPackValues);
#endif

#endif /* FOXBMS__BMS_H_ */
