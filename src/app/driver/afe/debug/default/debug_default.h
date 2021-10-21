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
 * @file    debug_default.h
 * @author  foxBMS Team
 * @date    2020-09-17 (date of creation)
 * @updated 2020-11-09 (date of last update)
 * @ingroup DRIVERS
 * @prefix  FAKE
 *
 * @brief   Header for the driver of the fake AFE driver
 *
 */

#ifndef FOXBMS__DEBUG_DEFAULT_H_
#define FOXBMS__DEBUG_DEFAULT_H_

/*========== Includes =======================================================*/
#include "general.h"

#include "database_cfg.h"

/*========== Macros and Definitions =========================================*/

/** States of the state machine */
typedef enum FAKE_FSM_STATES {
    FAKE_FSM_STATE_DUMMY,          /*!< dummy state - always the first state */
    FAKE_FSM_STATE_HAS_NEVER_RUN,  /*!< never run state - always the second state */
    FAKE_FSM_STATE_UNINITIALIZED,  /*!< uninitialized state */
    FAKE_FSM_STATE_INITIALIZATION, /*!< initializing the state machine */
    FAKE_FSM_STATE_RUNNING,        /*!< operational mode of the state machine  */
    FAKE_FSM_STATE_ERROR,          /*!< state for error processing  */
} FAKE_FSM_STATES_e;

/** Substates of the state machine */
typedef enum FAKE_FSM_SUBSTATES {
    FAKE_FSM_SUBSTATE_DUMMY,                                     /*!< dummy state - always the first substate */
    FAKE_FSM_SUBSTATE_ENTRY,                                     /*!< entry state - always the second substate */
    FAKE_FSM_SUBSTATE_INITIALIZATION_FINISH_FIRST_MEASUREMENT,   /*!< finish the first fake measurement */
    FAKE_FSM_SUBSTATE_INITIALIZATION_FIRST_MEASUREMENT_FINISHED, /*!< cleanup substate after the first fake measurement */
    FAKE_FSM_SUBSTATE_INITIALIZATION_EXIT,                       /*!< last initialization substate */
    FAKE_FSM_SUBSTATE_RUNNING_SAVE_VOLTAGE_MEASUREMENT_DATA,     /*!< state to continuously save the measurement data */
    FAKE_FSM_SUBSTATE_RUNNING_SAVE_TEMPERATURE_MEASUREMENT_DATA, /*!< state to continuously save the measurement data */
} FAKE_FSM_SUBSTATES_e;

/** This struct contains pointer to used data buffers */
typedef struct FAKE_DATABASE_ENTRIES {
    DATA_BLOCK_CELL_VOLTAGE_s *cellVoltage;             /*!< cell voltage */
    DATA_BLOCK_CELL_TEMPERATURE_s *cellTemperature;     /*!< cell temperature */
    DATA_BLOCK_BALANCING_FEEDBACK_s *balancingFeedback; /*!< balancing feedback */
    DATA_BLOCK_BALANCING_CONTROL_s *balancingControl;   /*!< balancing control */
    DATA_BLOCK_SLAVE_CONTROL_s *slaveControl;           /*!< slave control */
    DATA_BLOCK_ALL_GPIO_VOLTAGES_s *allGpioVoltages;    /*!< voltage of the slaves' GPIOs */
    DATA_BLOCK_OPEN_WIRE_s *openWire;                   /*!< open wire status */
} FAKE_DATABASE_ENTRIES_s;

/** This struct describes the state of the monitoring instance */
typedef struct FAKE_STATE {
    uint16_t timer;                        /*!< timer of the state */
    uint8_t triggerEntry;                  /*!< trigger entry of the state */
    FAKE_FSM_STATES_e nextState;           /*!< next state of the FSM */
    FAKE_FSM_STATES_e currentState;        /*!< current state of the FSM */
    FAKE_FSM_STATES_e previousState;       /*!< previous state of the FSM */
    FAKE_FSM_SUBSTATES_e nextSubstate;     /*!< next substate of the FSM */
    FAKE_FSM_SUBSTATES_e currentSubstate;  /*!< current substate of the FSM */
    FAKE_FSM_SUBSTATES_e previousSubstate; /*!< previous substate of the FSM */
    bool firstMeasurementFinished;         /*!< indicator if the fist measurement has been successful */
    FAKE_DATABASE_ENTRIES_s data;          /*!< contains pointers to the local data buffer */
} FAKE_STATE_s;

/*========== Extern Constant and Variable Declarations ======================*/

/** state of the fake state machine */
extern FAKE_STATE_s fake_state;

/*========== Extern Function Prototypes =====================================*/

/** @brief  initialize driver */
extern STD_RETURN_TYPE_e FAKE_Initialize(void);

/**
 * @brief   return whether the first measurement cycle is finished
 * @param   pFakeState current state of the fake driver
 * @returns true if the first measurement cycle was successfully finished,
 *          false otherwise
 */
extern bool FAKE_IsFirstMeasurementCycleFinished(FAKE_STATE_s *pFakeState);

/**
 * @brief   Trigger function for the driver, called to advance the
 *          state machine
 * @param   pFakeState current state of the fake driver
 * @returns returns always #STD_OK
 */
extern STD_RETURN_TYPE_e FAKE_TriggerAfe(FAKE_STATE_s *pFakeState);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern bool TEST_FAKE_CheckMultipleCalls(FAKE_STATE_s *pFakeState);

extern void TEST_FAKE_SetFirstMeasurementCycleFinished(FAKE_STATE_s *pFakeState);

extern void TEST_FAKE_SetState(
    FAKE_STATE_s *pFakeState,
    FAKE_FSM_STATES_e nextState,
    FAKE_FSM_SUBSTATES_e nextSubstate,
    uint16_t idleTime);

extern STD_RETURN_TYPE_e TEST_FAKE_SaveFakeVoltageMeasurementData(FAKE_STATE_s *pFakeState);

extern STD_RETURN_TYPE_e TEST_FAKE_SaveFakeTemperatureMeasurementData(FAKE_STATE_s *pFakeState);

#endif

#endif /* FOXBMS__DEBUG_DEFAULT_H_ */
