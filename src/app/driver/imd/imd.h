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
 * @file    imd.h
 * @author  foxBMS Team
 * @date    2020-11-20 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  IMD
 *
 * @brief   API header for the insulation monitoring device
 * @details TODO
 */

#ifndef FOXBMS__IMD_H_
#define FOXBMS__IMD_H_

/*========== Includes =======================================================*/

#include "can_cfg.h"
#include "ftask_cfg.h"

#include "database.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
#define IMD_PERIODIC_CALL_TIME_ms (FTSK_TASK_CYCLIC_100MS_CYCLE_TIME)

/** Minimum required insulation resistance - Error threshold */
#define IMD_ERROR_THRESHOLD_INSULATION_RESISTANCE_kOhm (500u)

/** Minimum required insulation resistance - Warning threshold */
#define IMD_WARNING_THRESHOLD_INSULATION_RESISTANCE_kOhm (750u)

/** Possible return values when state requests are made to the IMD state machine */
typedef enum {
    IMD_REQUEST_OK,          /*!< request was successful */
    IMD_REQUEST_PENDING,     /*!< requested to be executed */
    IMD_ILLEGAL_REQUEST,     /*!< Request can not be executed */
    IMD_ALREADY_INITIALIZED, /*!< IMD state machine already initialized */
} IMD_RETURN_TYPE_e;

/** State requests for the IMD state machine */
typedef enum {
    IMD_STATE_INITIALIZE_REQUEST, /*!< request for initialization */
    IMD_STATE_SWITCH_ON_REQUEST,  /*!< request to switch on IMD device */
    IMD_STATE_SHUTDOWN_REQUEST,   /*!< request for shut down  */
    IMD_STATE_NO_REQUEST,         /*!< dummy request for no request */
} IMD_STATE_REQUEST_e;

/** States of the state machine */
typedef enum {
    IMD_FSM_STATE_DUMMY,          /*!< dummy state - always the first state */
    IMD_FSM_STATE_HAS_NEVER_RUN,  /*!< never run state - always the second state */
    IMD_FSM_STATE_UNINITIALIZED,  /*!< uninitialized state */
    IMD_FSM_STATE_INITIALIZATION, /*!< initializing the state machine */
    IMD_FSM_STATE_IMD_ENABLE,     /*!< switch on IMD */
    IMD_FSM_STATE_SHUTDOWN,       /*!< shut down state of IMD state machine */
    IMD_FSM_STATE_RUNNING,        /*!< operational mode of the state machine */
    IMD_FSM_STATE_ERROR,          /*!< state for error processing  */
} IMD_FSM_STATES_e;

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
    bool isStateMachineInitialized; /*!< true if initialized, otherwise false */
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

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   Request initialization of IMD state machine
 * @return  Always #IMD_REQUEST_OK
 */
extern IMD_RETURN_TYPE_e IMD_RequestInitialization(void);

/**
 * @brief   Request to activate the actual IMD measurement
 * @return  Always #IMD_REQUEST_OK
 */
extern IMD_RETURN_TYPE_e IMD_RequestInsulationMeasurement(void);

/**
 * @brief   Request to deactivate the actual IMD measurement
 * @return  Always #IMD_REQUEST_OK
 */
extern IMD_RETURN_TYPE_e IMD_RequestMeasurementStop(void);

/**
 * @brief   Gets the initialization state.
 * @details This function is used for getting the IMD initialization state.
 * @return  true if state machine initialized, otherwise false
 */
extern bool IMD_GetInitializationState(void);

/**
 * @brief   trigger function for the IMD driver state machine.
 * @details This function contains the sequence of events in the IMD state
 *          machine. It must be called time-triggered, every 100ms.
 * @return  returns #STD_OK if trigger called successfully
 */
extern STD_RETURN_TYPE_e IMD_Trigger(void);

/**************************************************** FOLLOWING FUNCTIONS NEED TO BE IMPLEMENTED BY THE IMD DRIVER */
/* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE imd-documentation */
/**
 * @brief   Processes the initialization state
 * @details This function needs to be implemented in the dedicated driver. This
 *          function initializes the required SW modules and peripherals but
 *          does not start the actual IMD measurement.
 * @return  #IMD_FSM_STATE_INITIALIZATION if initialization not finished and
 *          another call is required. #IMD_FSM_STATE_IMD_ENABLE if
 *          initialization is finished and #IMD_FSM_STATE_ERROR if an error is
 *          detected that prohibits a working IMD driver.
 */
extern IMD_FSM_STATES_e IMD_ProcessInitializationState(void);

/**
 * @brief   Processes the IMD enable state
 * @details This function needs to be implemented in the dedicated driver. This
 *          function enables the actual IMD device to start the insulation
 *          measurement. Functionality need to be fulfilled after one call.
 * @return  #IMD_FSM_STATE_RUNNING if startup has been completed. Returns #IMD_FSM_STATE_ERROR if an error is
 *          detected that prohibits a working IMD driver.
 */
extern IMD_FSM_STATES_e IMD_ProcessEnableState(void);

/**
 * @brief   Processes the running state
 * @details This function needs to be implemented in the dedicated driver
 * @param   pTableInsulationMonitoring   pointer to insulation monitoring
 *                                       database entry
 * @return  #IMD_FSM_STATE_RUNNING if measurement works as expected, otherwise
 *          #IMD_FSM_STATE_ERROR if an error is detected that prohibits a
 *          further execution of the IMD driver.
 */
extern IMD_FSM_STATES_e IMD_ProcessRunningState(DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring);

/**
 * @brief   Processes the shutdown state
 * @details This function needs to be implemented in the dedicated driver. This
 *          function disables the actual IMD device to stop the insulation
 *          measurement.
 * @return  #IMD_FSM_STATE_SHUTDOWN if shutdown state is not finished and
 *          another call is required. #IMD_FSM_STATE_IMD_ENABLE if shut down
 *          has been completed and #IMD_FSM_STATE_ERROR if an error is detected
 *          that prohibits a working IMD driver.
 */
extern IMD_FSM_STATES_e IMD_ProcessShutdownState(void);

/* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE imd-documentation */
/*******************************************************************************************************************/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
IMD_RETURN_TYPE_e TEST_IMD_SetStateRequest(IMD_STATE_s *pImdState, IMD_STATE_REQUEST_e stateRequest);

IMD_RETURN_TYPE_e TEST_IMD_CheckStateRequest(IMD_STATE_s *pImdState, IMD_STATE_REQUEST_e stateRequest);

IMD_STATE_REQUEST_e TEST_IMD_TransferStateRequest(IMD_STATE_s *pImdState);

IMD_CHECK_MULTIPLE_CALLS_e TEST_IMD_CheckMultipleCalls(IMD_STATE_s *pImdState);

void TEST_IMD_SetState(
    IMD_STATE_s *pImdState,
    IMD_FSM_STATES_e nextState,
    IMD_FSM_SUBSTATES_e nextSubstate,
    uint16_t idleTime);
#endif

#endif /* FOXBMS__IMD_H_ */
