/**
 *
 * @copyright &copy; 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    bender_iso165c.c
 * @author  foxBMS Team
 * @date    2019-04-07 (date of creation)
 * @updated 2024-12-20 (date of last update)
 * @version v1.8.0
 * @ingroup DRIVERS
 * @prefix  I165C
 *
 * @brief   Driver for the insulation monitoring
 * @details Main file of bender iso165C and iso165C-1 driver
 *
 */

/*========== Includes =======================================================*/
#include "bender_iso165c.h"

#include "database_cfg.h"

#include "can.h"
#include "can_cbs_rx.h"
#include "can_cbs_tx_imd-request.h"
#include "can_cfg_rx-message-definitions.h"
#include "can_cfg_tx-async-message-definitions.h"
#include "can_helper.h"
#include "database.h"
#include "ftask.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** state machine short time definition in trigger calls until next state is processed */
#define I165C_FSM_SHORT_TIME (1u)

/* return values for 165C_CheckResponse*/
typedef enum {
    I165C_RESPONSE_NO_RESPONSE, /*!< no IMD response message received yet */
    I165C_RESPONSE_SUCCESS,     /*!< IMD response message with the expected multiplexer was received */
    I165C_RESPONSE_ERROR,       /*!< IMD response reports error */
} I165C_RESPONSE_RETURN_VALUE_e;

/* -------------- State defines ---------------------------------------------*/

/** States of the initialization state machine */
typedef enum {
    I165C_FSM_STATE_INITIALIZATION_DUMMY,           /*!< dummy state - always the first state */
    I165C_FSM_STATE_INITIALIZATION_HAS_NEVER_RUN,   /*!< never run state - always the second state - unlock device */
    I165C_FSM_STATE_INITIALIZATION_UNLOCK_WAIT_ACK, /*!< wait acknowledge of unlocking */
    I165C_FSM_STATE_INITIALIZATION_CHECK_MEASUREMENT_STATE, /*!< check if measurement is enabled. if not enable it */
    I165C_FSM_STATE_INITIALIZATION_ENABLE_MEASUREMENT_WAIT_ACK, /*!< wait for acknowledge for enabling of measurement */
    I165C_FSM_STATE_INITIALIZATION_REQUEST_HV_RELAY_OPENING,    /*!< request HV relay state: open */
    I165C_FSM_STATE_INITIALIZATION_REQUEST_NEGATIVE_HV_RELAY_STATE, /*!< check HV relay state */
    I165C_FSM_STATE_INITIALIZATION_CHECK_NEGATIVE_HV_RELAY_STATE,   /*!< check negative HV relay state */
    I165C_FSM_STATE_INITIALIZATION_CHECK_POSITIVE_HV_RELAY_STATE,   /*!< check positive HV relay state */
    I165C_FSM_STATE_INITIALIZATION_REQUEST_SELF_TEST,               /*!< request self-test */
    I165C_FSM_STATE_INITIALIZATION_SELF_TEST_WAIT_ACK,              /*!< wait acknowledge of self-test */
    I165C_FSM_STATE_INITIALIZATION_WAIT_SELF_TEST,                  /*!< wait until self-test is finished */
    I165C_FSM_STATE_INITIALIZATION_SET_AVERAGING_FACTOR,            /*!< set averaging factor */
    I165C_FSM_STATE_INITIALIZATION_AVERAGING_FACTOR_WAIT_ACK,       /*!< wait acknowledge of averaging factor */
    I165C_FSM_STATE_INITIALIZATION_SET_ERROR_THRESHOLD,             /*!< configuration of error threshold */
    I165C_FSM_STATE_INITIALIZATION_ERROR_THRESHOLD_WAIT_ACK,        /*!< wait acknowledge of error threshold */
    I165C_FSM_STATE_INITIALIZATION_SET_WARNING_THRESHOLD,           /*!< configuration of warning threshold */
    I165C_FSM_STATE_INITIALIZATION_WARNING_THRESHOLD_WAIT_ACK,      /*!< wait acknowledge of warning threshold */
    I165C_FSM_STATE_INITIALIZATION_DISABLE_MEASUREMENT,             /*!< disable measurement */
    I165C_FSM_STATE_INITIALIZATION_DISABLE_MEASUREMENT_WAIT_ACK,    /*!< wait acknowledge of disable measurement */
} I165C_FSM_INITIALIZATION_STATES_e;

/** States of the enable state machine */
typedef enum {
    I165C_FSM_STATE_ENABLE_DUMMY,         /*!< dummy state - always the first state */
    I165C_FSM_STATE_ENABLE_HAS_NEVER_RUN, /*!< never run state - always the second state - unlock device */
    I165C_FSM_STATE_ENABLE_REQUEST_NEGATIVE_HV_RELAY_STATE, /*!< check negative HV relay state */
    I165C_FSM_STATE_ENABLE_CHECK_NEGATIVE_HV_RELAY_STATE,   /*!< check negative HV relay state */
    I165C_FSM_STATE_ENABLE_CHECK_POSITIVE_HV_RELAY_STATE,   /*!< check positive HV relay state */
    I165C_FSM_STATE_ENABLE_START_MEASUREMENT,               /*!< start insulation measurement */
    I165C_FSM_STATE_ENABLE_START_MEASUREMENT_WAIT_ACK,      /*!< check for acknowledge of start measurement request */
} I165C_FSM_ENABLE_STATES_e;

/** States of the running state machine */
typedef enum {
    I165C_FSM_STATE_RUNNING_DUMMY,         /*!< dummy state - always the first state */
    I165C_FSM_STATE_RUNNING_HAS_NEVER_RUN, /*!< never run state - always the second state */
    I165C_FSM_STATE_RUNNING_READ_RESISTANCE,
    I165C_FSM_STATE_RUNNING_READ_RESISTANCE_WAIT_ACK,
    I165C_FSM_STATE_RUNNING_GET_MEASUREMENT,
} I165C_FSM_RUNNING_STATES_e;

/** States of the disable state machine */
typedef enum {
    I165C_FSM_STATE_DISABLE_DUMMY,                /*!< dummy state - always the first state */
    I165C_FSM_STATE_DISABLE_HAS_NEVER_RUN,        /*!< never run state - always the second state - unlock device */
    I165C_FSM_STATE_MEASUREMENT_STOPPED_WAIT_ACK, /*!< wait for acknowledge of stop measurement request */
    I165C_FSM_STATE_DISABLE_SET_HV_RELAY_STATE,   /*!< request state of HV relays */
    I165C_FSM_STATE_DISABLE_REQUEST_NEGATIVE_HV_RELAY_STATE, /*!< request state of negative */
    I165C_FSM_STATE_DISABLE_CHECK_NEGATIVE_HV_RELAY_STATE,   /*!< check negative HV relay state */
    I165C_FSM_STATE_DISABLE_CHECK_POSITIVE_HV_RELAY_STATE,   /*!< check positive HV relay state */
} I165C_FSM_DISABLE_STATES_e;

/* -------------- State variables -------------------------------------------*/

/** This struct describes the state of the initialization state machine */
typedef struct {
    uint16_t timer;                                  /*!< timer of the state */
    uint8_t triggerEntry;                            /*!< trigger entry of the state */
    I165C_FSM_INITIALIZATION_STATES_e currentState;  /*!< current state of the FSM */
    I165C_FSM_INITIALIZATION_STATES_e previousState; /*!< previous state of the FSM */
    uint8_t receptionTries;
    uint8_t receptionTriesMessage;
    bool negativeRelayClosed;
    bool positiveRelayClosed;
} I165C_INITIALIZATION_STATE_s;

typedef struct {
    uint16_t timer;                          /*!< timer of the state */
    uint8_t triggerEntry;                    /*!< trigger entry of the state */
    I165C_FSM_ENABLE_STATES_e currentState;  /*!< current state of the FSM */
    I165C_FSM_ENABLE_STATES_e previousState; /*!< previous state of the FSM */
    uint8_t receptionTries;
    uint8_t receptionTriesMessage;
} I165C_ENABLE_STATE_s;

typedef struct {
    uint16_t timer;                           /*!< timer of the state */
    uint8_t triggerEntry;                     /*!< trigger entry of the state */
    I165C_FSM_RUNNING_STATES_e currentState;  /*!< current state of the FSM */
    I165C_FSM_RUNNING_STATES_e previousState; /*!< previous state of the FSM */
    uint8_t receptionTries;
    uint8_t receptionTriesMessage;
} I165C_RUNNING_STATE_s;

typedef struct {
    uint16_t timer;                           /*!< timer of the state */
    uint8_t triggerEntry;                     /*!< trigger entry of the state */
    I165C_FSM_DISABLE_STATES_e currentState;  /*!< current state of the FSM */
    I165C_FSM_DISABLE_STATES_e previousState; /*!< previous state of the FSM */
    uint8_t receptionTries;
    uint8_t receptionTriesMessage;
} I165C_DISABLE_STATE_s;

/*========== Static Constant and Variable Definitions =======================*/

static I165C_INITIALIZATION_STATE_s i165c_initializationState = {
    .timer                 = 0u,
    .triggerEntry          = 0u,
    .currentState          = I165C_FSM_STATE_INITIALIZATION_HAS_NEVER_RUN,
    .previousState         = I165C_FSM_STATE_INITIALIZATION_DUMMY,
    .receptionTries        = 0u,
    .receptionTriesMessage = 0u,
#if false == I165C_IS_165C_USED
    /* Default state after startup is closed for iso165C-1 */
    .negativeRelayClosed   = true,
    .positiveRelayClosed   = true,
#else
    /* Default state after startup is open for iso165C */
    .negativeRelayClosed = false,
    .positiveRelayClosed = false,
#endif
};

static I165C_ENABLE_STATE_s i165c_enableState = {
    .timer                 = 0u,
    .triggerEntry          = 0u,
    .currentState          = I165C_FSM_STATE_ENABLE_HAS_NEVER_RUN,
    .previousState         = I165C_FSM_STATE_ENABLE_DUMMY,
    .receptionTries        = 0u,
    .receptionTriesMessage = 0u,
};

static I165C_RUNNING_STATE_s i165c_runningState = {
    .timer                 = 0u,
    .triggerEntry          = 0u,
    .currentState          = I165C_FSM_STATE_RUNNING_HAS_NEVER_RUN,
    .previousState         = I165C_FSM_STATE_RUNNING_DUMMY,
    .receptionTries        = 0u,
    .receptionTriesMessage = 0u,
};

static I165C_DISABLE_STATE_s i165c_disableState = {
    .timer                 = 0u,
    .triggerEntry          = 0u,
    .currentState          = I165C_FSM_STATE_DISABLE_HAS_NEVER_RUN,
    .previousState         = I165C_FSM_STATE_DISABLE_DUMMY,
    .receptionTries        = 0u,
    .receptionTriesMessage = 0u,
};

static CAN_BUFFER_ELEMENT_s i165c_canRxMessage = {0u};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Sets the next state the timer value
 *          of the initialization state variable.
 * @param[in,out] pImdState      state of the initialization state machine
 * @param[in]     nextState      state to be transferred into
 * @param[in]     idleTime       wait time for the state machine
 */
static void I165C_SetInitializationState(
    I165C_INITIALIZATION_STATE_s *pImdState,
    I165C_FSM_INITIALIZATION_STATES_e nextState,
    uint16_t idleTime);

/**
 * @brief   Sets the next state the timer value
 *          of the enable state variable.
 * @param[in,out] pImdState      state of the enable state machine
 * @param[in]     nextState      state to be transferred into
 * @param[in]     idleTime       wait time for the state machine
 */
static void I165C_SetEnableState(
    I165C_ENABLE_STATE_s *pImdState,
    I165C_FSM_ENABLE_STATES_e nextState,
    uint16_t idleTime);

/**
 * @brief   Sets the next state the timer value
 *          of the running state variable.
 * @param[in,out] pImdState      state of the enable state machine
 * @param[in]     nextState      state to be transferred into
 * @param[in]     idleTime       wait time for the state machine
 */
static void I165C_SetRunningState(
    I165C_RUNNING_STATE_s *pImdState,
    I165C_FSM_RUNNING_STATES_e nextState,
    uint16_t idleTime);

/**
 * @brief   Sets the next state the timer value
 *          of the disable state variable.
 * @param[in,out] pImdState      state of the enable state machine
 * @param[in]     nextState      state to be transferred into
 * @param[in]     idleTime       wait time for the state machine
 */
static void I165C_SetDisableState(
    I165C_DISABLE_STATE_s *pImdState,
    I165C_FSM_DISABLE_STATES_e nextState,
    uint16_t idleTime);

/** Initialization state machine */
static IMD_FSM_STATES_e I165C_Initialize(void);

/** Enable state machine */
static IMD_FSM_STATES_e I165C_Enable(void);

/** Disable state machine */
static IMD_FSM_STATES_e I165C_Disable(void);

/**
 * @brief   trigger function for the i165c driver state machine.
 * @details This function contains the sequence of events in the i165c state
 *          machine.
 *          It must be called time-triggered, every 100ms.
 * @param pTableInsulationMonitoring   pointer to insulation database entry
 */
static IMD_FSM_STATES_e I165C_Running(DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring);

/**
 * @brief   Check if iso165c acknowledged reception of sent message
 *          and get corresponding data.
 * @details Gets data from the CAN module through a queue.
 * @param   command     check if this command is sent by iso165c to acknowledge
 *                      reception
 * @param[in] pCanMessage  pointer to CAN data sent by the iso165c
 * @return  true if transmission acknowledged, false otherwise
 */
static I165C_RESPONSE_RETURN_VALUE_e I165C_CheckResponse(uint8_t command, CAN_BUFFER_ELEMENT_s *pCanMessage);

/**
 * @brief   Get IMD Info from iso165c
 * @details Gets data from the CAN module through a queue.
 * @param[in] pCanMessage  pointer to CAN data sent by the iso165c
 * @return  true if IMD_Info message was received, false otherwise
 */
static bool I165C_GetImdInfo(CAN_BUFFER_ELEMENT_s *pCanMessage);

/**
 * @brief   Set state of HV relay
 * @param[in] relay set state of positive or negative relay
 * @param[in] relayState open or close relay
 */
static void I165C_SetRelayState(uint8_t relay, uint8_t relayState);

/**
 * @brief   Request state of HV relay
 * @param[in] relay       positive or negative relay
 */
static void I165C_RequestRelayState(uint8_t relay);

/**
 * @brief   Set measurement mode
 * @param[in] mode set IMD measurement mode
 */
static void I165C_SetMeasurementMode(uint8_t mode);

/**
 * @brief   Set average factor of the insulation resistance averaging algorithm
 * @param[in] averagingFactor set IMD averaging factor
 */
static void I165C_SetAveragingFactor(uint8_t averagingFactor);

/**
 * @brief   Check if iso165c acknowledged reception of command
 * @param command         command to be acknowledged
 * @param[out] pTries     pointer to variable counting the number of tries for reception
 * @param[in] pCanMessage pointer to CAN data sent by the iso165c
 * @return  true if Acknowledge has been received, otherwise false
 */
static I165C_RESPONSE_RETURN_VALUE_e I165C_CheckAcknowledgeArrived(
    uint8_t command,
    uint8_t *pTries,
    CAN_BUFFER_ELEMENT_s *pCanMessage);

/**
 * @brief Check if insulation measurement is valid
 * @param pTableInsulationMonitoring data table where information is stored
 */
static void I165C_IsInsulationMeasurementValid(DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring);

/*========== Static Function Implementations ================================*/
static void I165C_SetInitializationState(
    I165C_INITIALIZATION_STATE_s *pImdState,
    I165C_FSM_INITIALIZATION_STATES_e nextState,
    uint16_t idleTime) {
    FAS_ASSERT(pImdState != NULL_PTR);
    /* Set time */
    pImdState->timer = idleTime;
    /* Set state */
    if (nextState != pImdState->currentState) {
        /* Next state is different: switch to it and set substate to entry value */
        pImdState->previousState = pImdState->currentState;
        pImdState->currentState  = nextState;
    } else {
        /* Next state equal to current state: nothing to do */
    }
}

static void I165C_SetEnableState(
    I165C_ENABLE_STATE_s *pImdState,
    I165C_FSM_ENABLE_STATES_e nextState,
    uint16_t idleTime) {
    FAS_ASSERT(pImdState != NULL_PTR);
    /* Set time */
    pImdState->timer = idleTime;
    /* Set state */
    if (nextState != pImdState->currentState) {
        /* Next state is different: switch to it and set substate to entry value */
        pImdState->previousState = pImdState->currentState;
        pImdState->currentState  = nextState;
    } else {
        /* Next state equal to current state: nothing to do */
    }
}

static void I165C_SetRunningState(
    I165C_RUNNING_STATE_s *pImdState,
    I165C_FSM_RUNNING_STATES_e nextState,
    uint16_t idleTime) {
    FAS_ASSERT(pImdState != NULL_PTR);
    /* Set time */
    pImdState->timer = idleTime;
    /* Set state */
    if (nextState != pImdState->currentState) {
        /* Next state is different: switch to it and set substate to entry value */
        pImdState->previousState = pImdState->currentState;
        pImdState->currentState  = nextState;
    } else {
        /* Next state equal to current state: nothing to do */
    }
}

static void I165C_SetDisableState(
    I165C_DISABLE_STATE_s *pImdState,
    I165C_FSM_DISABLE_STATES_e nextState,
    uint16_t idleTime) {
    FAS_ASSERT(pImdState != NULL_PTR);
    /* Set time */
    pImdState->timer = idleTime;
    /* Set state */
    if (nextState != pImdState->currentState) {
        /* Next state is different: switch to it and set substate to entry value */
        pImdState->previousState = pImdState->currentState;
        pImdState->currentState  = nextState;
    } else {
        /* Next state equal to current state: nothing to do */
    }
}

static I165C_RESPONSE_RETURN_VALUE_e I165C_CheckResponse(uint8_t command, CAN_BUFFER_ELEMENT_s *pCanMessage) {
    FAS_ASSERT(pCanMessage != NULL_PTR);
    I165C_RESPONSE_RETURN_VALUE_e messageReceived = I165C_RESPONSE_NO_RESPONSE;
    uint8_t numberItems                           = 0u;
    uint8_t queueReadTries                        = I165C_MAX_QUEUE_READS;

    /* Use loop on queue because IMD_info message could come meanwhile */
    do {
        numberItems = OS_GetNumberOfStoredMessagesInQueue(ftsk_imdCanDataQueue);
        if (numberItems > 0u) {
            if (OS_ReceiveFromQueue(ftsk_imdCanDataQueue, (void *)pCanMessage, 0u) == OS_SUCCESS) {
                /* data queue was not empty */
                if ((command == pCanMessage->data[CAN_BYTE_0_POSITION]) && (pCanMessage->id == CANRX_IMD_RESPONSE_ID) &&
                    (pCanMessage->idType == CANRX_IMD_RESPONSE_ID_TYPE)) {
                    /* Response with expected multiplexer was received */
                    messageReceived = I165C_RESPONSE_SUCCESS;
                    break;
                } else {
                    if ((pCanMessage->data[CAN_BYTE_0_POSITION] == 0xFF) &&
                        (pCanMessage->id == CANRX_IMD_RESPONSE_ID) &&
                        (pCanMessage->idType == CANRX_IMD_RESPONSE_ID_TYPE)) {
                        /* Response with error multiplexer was received */
                        messageReceived = I165C_RESPONSE_ERROR;
                    }
                }
            }
        }
        queueReadTries--;
    } while ((numberItems > 0u) && (queueReadTries > 0u));

    return messageReceived;
}

static bool I165C_GetImdInfo(CAN_BUFFER_ELEMENT_s *pCanMessage) {
    FAS_ASSERT(pCanMessage != NULL_PTR);
    bool imdInfoReceived   = false;
    uint8_t numberItems    = 0u;
    uint8_t queueReadTries = I165C_MAX_QUEUE_READS;

    /* Use loop on queue because other messages could come meanwhile */
    do {
        numberItems = OS_GetNumberOfStoredMessagesInQueue(ftsk_imdCanDataQueue);
        if (numberItems > 0u) {
            if (OS_ReceiveFromQueue(ftsk_imdCanDataQueue, (void *)pCanMessage, 0u) == OS_SUCCESS) {
                /* data queue was not empty */
                if (pCanMessage->id == I165C_MESSAGE_TYPE_IMD_INFO) {
                    imdInfoReceived = true;
                    break;
                }
            }
        }
        queueReadTries--;
    } while ((numberItems > 0u) && (queueReadTries > 0u));

    return imdInfoReceived;
}

static void I165C_SetRelayState(uint8_t relay, uint8_t relayState) {
    FAS_ASSERT((relay == I165C_D_VIFC_HV_RELAIS_NEGATIVE) || (relay == I165C_D_VIFC_HV_RELAIS_POSITIVE));
    FAS_ASSERT((relayState == I165C_RELAY_STATE_OPEN) || (relayState == I165C_RELAY_STATE_CLOSED));

    if (relay == I165C_D_VIFC_HV_RELAIS_NEGATIVE) {
        if (relayState == I165C_RELAY_STATE_OPEN) {
            CANTX_ImdRequest(CANTX_IMD_REQUEST_OPEN_NEGATIVE_RELAY);
        } else {
            CANTX_ImdRequest(CANTX_IMD_REQUEST_CLOSE_NEGATIVE_RELAY);
        }
    } else {
        if (relayState == I165C_RELAY_STATE_OPEN) {
            CANTX_ImdRequest(CANTX_IMD_REQUEST_OPEN_POSITIVE_RELAY);
        } else {
            CANTX_ImdRequest(CANTX_IMD_REQUEST_CLOSE_POSITIVE_RELAY);
        }
    }
}

static void I165C_RequestRelayState(uint8_t relay) {
    FAS_ASSERT((relay == I165C_D_VIFC_HV_RELAIS_NEGATIVE) || (relay == I165C_D_VIFC_HV_RELAIS_POSITIVE));

    switch (relay) {
        case I165C_D_VIFC_HV_RELAIS_NEGATIVE:
            CANTX_ImdRequest(CANTX_IMD_REQUEST_NEGATIVE_RELAY_STATE);
            break;
        case I165C_D_VIFC_HV_RELAIS_POSITIVE:
            CANTX_ImdRequest(CANTX_IMD_REQUEST_POSITIVE_RELAY_STATE);
            break;
        default:
            FAS_ASSERT(FAS_TRAP);
            break;
    }
}

static void I165C_SetMeasurementMode(uint8_t mode) {
    FAS_ASSERT((mode == I165C_ENABLE_MEASUREMENT) || (mode == I165C_DISABLE_MEASUREMENT));

    switch (mode) {
        case I165C_ENABLE_MEASUREMENT:
            CANTX_ImdRequest(CANTX_IMD_REQUEST_ENABLE_MEASUREMENT);
            break;
        case I165C_DISABLE_MEASUREMENT:
            CANTX_ImdRequest(CANTX_IMD_REQUEST_DISABLE_MEASUREMENT);
            break;
    }
}

static void I165C_SetAveragingFactor(uint8_t averagingFactor) {
    /* Averaging factor must be in the range 1...20 */
    FAS_ASSERT(averagingFactor != 0u);
    FAS_ASSERT(averagingFactor <= 20u);

    CANTX_ImdRequest(CANTX_IMD_REQUEST_SET_AVERAGING_FACTOR);
}

static I165C_RESPONSE_RETURN_VALUE_e I165C_CheckAcknowledgeArrived(
    uint8_t command,
    uint8_t *pTries,
    CAN_BUFFER_ELEMENT_s *pCanMessage) {
    FAS_ASSERT(pTries != NULL_PTR);
    FAS_ASSERT(pCanMessage != NULL_PTR);
    /* AXIVION Routine Generic-MissingParameterAssert: command: parameter accepts whole range */

    I165C_RESPONSE_RETURN_VALUE_e acknowledgeReceived = I165C_CheckResponse(command, pCanMessage);
    if (acknowledgeReceived == I165C_RESPONSE_NO_RESPONSE) {
        (*pTries)++;
    } else {
        *pTries = 0u;
    }
    return acknowledgeReceived;
}

static void I165C_IsInsulationMeasurementValid(DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring) {
    if ((pTableInsulationMonitoring->areDeviceFlagsValid == true) &&
        (pTableInsulationMonitoring->isImdRunning == true) &&
        (pTableInsulationMonitoring->dfIsMeasurementUpToDate == true)) {
        pTableInsulationMonitoring->isInsulationMeasurementValid = true;
    } else {
        pTableInsulationMonitoring->isInsulationMeasurementValid = false;
    }
}

static IMD_FSM_STATES_e I165C_Initialize(void) {
    IMD_FSM_STATES_e nextState                  = IMD_FSM_STATE_INITIALIZATION; /* stay in initialization state */
    bool earlyExit                              = false;
    I165C_RESPONSE_RETURN_VALUE_e responseValue = I165C_RESPONSE_NO_RESPONSE;

    if (i165c_initializationState.timer > 0u) {
        if ((--i165c_initializationState.timer) > 0u) {
            i165c_initializationState.triggerEntry--;
            earlyExit = true;
        }
    }

    if (earlyExit == false) {
        switch (i165c_initializationState.currentState) {
            case I165C_FSM_STATE_INITIALIZATION_HAS_NEVER_RUN:
                /* Unlock device in case it was locked */
                CANTX_ImdRequest(CANTX_IMD_REQUEST_INITIALIZATION_UNLOCK);
                I165C_SetInitializationState(
                    &i165c_initializationState, I165C_FSM_STATE_INITIALIZATION_UNLOCK_WAIT_ACK, I165C_FSM_SHORT_TIME);
                break;

            case I165C_FSM_STATE_INITIALIZATION_UNLOCK_WAIT_ACK:
                responseValue = I165C_CheckAcknowledgeArrived(
                    I165C_CMD_S_VIFC_CTL_LOCK, &i165c_initializationState.receptionTries, &i165c_canRxMessage);
                if (responseValue == I165C_RESPONSE_SUCCESS) {
                    I165C_SetInitializationState(
                        &i165c_initializationState,
                        I165C_FSM_STATE_INITIALIZATION_CHECK_MEASUREMENT_STATE,
                        I165C_FSM_SHORT_TIME);
                } else {
                    /* Restart initialization process if iso165c responds with an error message */
                    if (responseValue == I165C_RESPONSE_ERROR) {
                        I165C_SetInitializationState(
                            &i165c_initializationState,
                            I165C_FSM_STATE_INITIALIZATION_HAS_NEVER_RUN,
                            I165C_FSM_SHORT_TIME);
                    }
                    /* Issue: 621 */
                }
                break;

            case I165C_FSM_STATE_INITIALIZATION_CHECK_MEASUREMENT_STATE:
                if (I165C_GetImdInfo(&i165c_canRxMessage) == true) {
                    if (CANRX_ImdInfoCheckMeasurementMode(&i165c_canRxMessage.data[0], I165C_ENABLE_MEASUREMENT) ==
                        false) {
                        /* Measurement is not enabled -> Enable measurement as otherwise the following
                         * initialization procedure would fail */
                        I165C_SetMeasurementMode(I165C_ENABLE_MEASUREMENT);
                        I165C_SetInitializationState(
                            &i165c_initializationState,
                            I165C_FSM_STATE_INITIALIZATION_ENABLE_MEASUREMENT_WAIT_ACK,
                            I165C_FSM_SHORT_TIME);
                    } else {
                        /* Measurement enabled -> continue with initialization procedure */
                        I165C_SetInitializationState(
                            &i165c_initializationState,
                            I165C_FSM_STATE_INITIALIZATION_REQUEST_HV_RELAY_OPENING,
                            I165C_FSM_SHORT_TIME);
                    }
                }
                break;

            case I165C_FSM_STATE_INITIALIZATION_ENABLE_MEASUREMENT_WAIT_ACK:
                responseValue = I165C_CheckAcknowledgeArrived(
                    I165C_CMD_S_VIFC_CTL_MEASUREMENT, &i165c_initializationState.receptionTries, &i165c_canRxMessage);
                if (responseValue == I165C_RESPONSE_SUCCESS) {
                    /* Measurement enabled -> continue with initialization procedure */
                    I165C_SetInitializationState(
                        &i165c_initializationState,
                        I165C_FSM_STATE_INITIALIZATION_REQUEST_HV_RELAY_OPENING,
                        I165C_FSM_SHORT_TIME);
                } else {
                    /* Restart initialization process if iso165c responds with an error message */
                    if (responseValue == I165C_RESPONSE_ERROR) {
                        I165C_SetInitializationState(
                            &i165c_initializationState,
                            I165C_FSM_STATE_INITIALIZATION_HAS_NEVER_RUN,
                            I165C_FSM_SHORT_TIME);
                    }
                    /* Issue: 621 */
                }
                break;

            case I165C_FSM_STATE_INITIALIZATION_REQUEST_HV_RELAY_OPENING:
                /* Open negative relay */
                I165C_SetRelayState(I165C_D_VIFC_HV_RELAIS_NEGATIVE, I165C_RELAY_STATE_OPEN);
                /* Open positive relay */
                I165C_SetRelayState(I165C_D_VIFC_HV_RELAIS_POSITIVE, I165C_RELAY_STATE_OPEN);
                /* Switch to next state */
                I165C_SetInitializationState(
                    &i165c_initializationState,
                    I165C_FSM_STATE_INITIALIZATION_REQUEST_NEGATIVE_HV_RELAY_STATE,
                    I165C_FSM_SHORT_TIME);
                break;

            case I165C_FSM_STATE_INITIALIZATION_REQUEST_NEGATIVE_HV_RELAY_STATE:
                I165C_RequestRelayState(I165C_D_VIFC_HV_RELAIS_NEGATIVE);
                I165C_SetInitializationState(
                    &i165c_initializationState,
                    I165C_FSM_STATE_INITIALIZATION_CHECK_NEGATIVE_HV_RELAY_STATE,
                    I165C_FSM_SHORT_TIME);
                break;

            case I165C_FSM_STATE_INITIALIZATION_CHECK_NEGATIVE_HV_RELAY_STATE:
                /* Check if HV relay is open and measurement has been stopped */
                responseValue = I165C_CheckResponse(I165C_CMD_S_VIFC_GET_HV_RELAIS, &i165c_canRxMessage);
                if (responseValue == I165C_RESPONSE_SUCCESS) {
                    if (CANRX_ImdResponseCheckRelayState(
                            &i165c_canRxMessage.data[0], I165C_D_VIFC_HV_RELAIS_NEGATIVE, I165C_RELAY_STATE_OPEN) ==
                        true) {
                        i165c_initializationState.receptionTries        = 0u;
                        i165c_initializationState.receptionTriesMessage = 0u;

                        /* Request state of positive HV relay */
                        I165C_RequestRelayState(I165C_D_VIFC_HV_RELAIS_POSITIVE);

                        I165C_SetInitializationState(
                            &i165c_initializationState,
                            I165C_FSM_STATE_INITIALIZATION_CHECK_POSITIVE_HV_RELAY_STATE,
                            I165C_FSM_SHORT_TIME);
                    } else {
                        i165c_initializationState.receptionTries++;
                        /* Issue: 621 */
                    }
                } else {
                    /* Restart initialization process if iso165c responds with an error message */
                    if (responseValue == I165C_RESPONSE_ERROR) {
                        I165C_SetInitializationState(
                            &i165c_initializationState,
                            I165C_FSM_STATE_INITIALIZATION_HAS_NEVER_RUN,
                            I165C_FSM_SHORT_TIME);
                    }
                    i165c_initializationState.receptionTriesMessage++;
                    /* Issue: 621 */
                }
                break;

            case I165C_FSM_STATE_INITIALIZATION_CHECK_POSITIVE_HV_RELAY_STATE:
                /* Check if HV relays are open and measurement has been stopped */
                responseValue = I165C_CheckResponse(I165C_CMD_S_VIFC_GET_HV_RELAIS, &i165c_canRxMessage);
                if (responseValue == I165C_RESPONSE_SUCCESS) {
                    if (CANRX_ImdResponseCheckRelayState(
                            &i165c_canRxMessage.data[0], I165C_D_VIFC_HV_RELAIS_POSITIVE, I165C_RELAY_STATE_OPEN) ==
                        true) {
                        i165c_initializationState.receptionTries        = 0u;
                        i165c_initializationState.receptionTriesMessage = 0u;

                        I165C_SetInitializationState(
                            &i165c_initializationState,
                            I165C_FSM_STATE_INITIALIZATION_REQUEST_SELF_TEST,
                            I165C_FSM_SHORT_TIME);
                    } else {
                        i165c_initializationState.receptionTries++;
                        /* Issue: 621 */
                    }
                } else {
                    /* Restart initialization process if iso165c responds with an error message */
                    if (responseValue == I165C_RESPONSE_ERROR) {
                        I165C_SetInitializationState(
                            &i165c_initializationState,
                            I165C_FSM_STATE_INITIALIZATION_HAS_NEVER_RUN,
                            I165C_FSM_SHORT_TIME);
                    }
                    i165c_initializationState.receptionTriesMessage++;
                    /* Issue: 621 */
                }
                break;

            case I165C_FSM_STATE_INITIALIZATION_REQUEST_SELF_TEST:
                /* A self test must be requested and can only be carried out
                 when the coupling relays are open. */

                if (I165C_GetImdInfo(&i165c_canRxMessage) == true) {
                    if (CANRX_ImdInfoHasSelfTestBeenExecuted(&i165c_canRxMessage.data[0]) == false) {
                        CANTX_ImdRequest(CANTX_IMD_REQUEST_INITIALIZATION_SELF_TEST);
                        I165C_SetInitializationState(
                            &i165c_initializationState,
                            I165C_FSM_STATE_INITIALIZATION_SELF_TEST_WAIT_ACK,
                            I165C_FSM_SHORT_TIME);
                    } else {
                        /* Self-test has already been performed -> skip following initialization steps as the device
                           has previously been successfully configured. */
                        I165C_SetInitializationState(
                            &i165c_initializationState,
                            I165C_FSM_STATE_INITIALIZATION_DISABLE_MEASUREMENT,
                            I165C_FSM_SHORT_TIME);
                    }
                }

                break;

            case I165C_FSM_STATE_INITIALIZATION_SELF_TEST_WAIT_ACK:
                responseValue = I165C_CheckAcknowledgeArrived(
                    I165C_CMD_S_IMC_CTL_SELFTEST, &i165c_initializationState.receptionTries, &i165c_canRxMessage);
                if (responseValue == I165C_RESPONSE_SUCCESS) {
                    I165C_SetInitializationState(
                        &i165c_initializationState,
                        I165C_FSM_STATE_INITIALIZATION_WAIT_SELF_TEST,
                        I165C_FSM_SHORT_TIME);
                } else {
                    /* Issue: 621 */
                    if (i165c_initializationState.receptionTries > I165C_TRANSMISSION_ATTEMPTS) {
                        /* Issue: 621 */
                        I165C_SetInitializationState(
                            &i165c_initializationState,
                            I165C_FSM_STATE_INITIALIZATION_HAS_NEVER_RUN,
                            I165C_FSM_SHORT_TIME);
                        i165c_initializationState.receptionTries = 0;
                    }
                    /* Restart initialization process if iso165c responds with an error message */
                    if (responseValue == I165C_RESPONSE_ERROR) {
                        I165C_SetInitializationState(
                            &i165c_initializationState,
                            I165C_FSM_STATE_INITIALIZATION_HAS_NEVER_RUN,
                            I165C_FSM_SHORT_TIME);
                    }
                }
                break;
            case I165C_FSM_STATE_INITIALIZATION_WAIT_SELF_TEST:
                if (I165C_GetImdInfo(&i165c_canRxMessage) == true) {
                    if (CANRX_ImdInfoIsSelfTestFinished(&i165c_canRxMessage.data[0]) == false) {
                        i165c_initializationState.receptionTries++;
                    } else {
                        i165c_initializationState.receptionTries        = 0u;
                        i165c_initializationState.receptionTriesMessage = 0u;
                        I165C_SetInitializationState(
                            &i165c_initializationState,
                            I165C_FSM_STATE_INITIALIZATION_SET_AVERAGING_FACTOR,
                            I165C_FSM_SHORT_TIME);
                    }
                } else {
                    /* Issue: 621 */
                    i165c_initializationState.receptionTriesMessage++;
                    if (i165c_initializationState.receptionTriesMessage >= I165C_IMD_INFO_RECEIVE_ATTEMPTS) {
                        /* Initialization not working: restart initialization procedure */
                        /* Issue: 621 */
                    }
                }
                break;

            case I165C_FSM_STATE_INITIALIZATION_SET_AVERAGING_FACTOR:
                I165C_SetAveragingFactor(I165C_MEASUREMENT_AVERAGING_FACTOR);
                I165C_SetInitializationState(
                    &i165c_initializationState,
                    I165C_FSM_STATE_INITIALIZATION_AVERAGING_FACTOR_WAIT_ACK,
                    I165C_FSM_SHORT_TIME);
                break;

            case I165C_FSM_STATE_INITIALIZATION_AVERAGING_FACTOR_WAIT_ACK:
                responseValue = I165C_CheckAcknowledgeArrived(
                    I165C_CMD_S_IMC_SET_MEAN_FACTOR, &i165c_initializationState.receptionTries, &i165c_canRxMessage);
                if (responseValue == I165C_RESPONSE_SUCCESS) {
                    I165C_SetInitializationState(
                        &i165c_initializationState,
                        I165C_FSM_STATE_INITIALIZATION_SET_ERROR_THRESHOLD,
                        I165C_FSM_SHORT_TIME);
                } else {
                    /* Restart initialization process if iso165c responds with an error message */
                    if (responseValue == I165C_RESPONSE_ERROR) {
                        I165C_SetInitializationState(
                            &i165c_initializationState,
                            I165C_FSM_STATE_INITIALIZATION_HAS_NEVER_RUN,
                            I165C_FSM_SHORT_TIME);
                    }
                    /* Issue: 621 */
                }
                break;

            case I165C_FSM_STATE_INITIALIZATION_SET_ERROR_THRESHOLD:
                CANTX_ImdRequest(CANTX_IMD_REQUEST_INITIALIZATION_SET_ERROR_THRESHOLD);
                I165C_SetInitializationState(
                    &i165c_initializationState,
                    I165C_FSM_STATE_INITIALIZATION_ERROR_THRESHOLD_WAIT_ACK,
                    I165C_FSM_SHORT_TIME);
                break;
            case I165C_FSM_STATE_INITIALIZATION_ERROR_THRESHOLD_WAIT_ACK:
                responseValue = I165C_CheckAcknowledgeArrived(
                    I165C_CMD_S_IMC_SET_R_ISO_ERR_THR, &i165c_initializationState.receptionTries, &i165c_canRxMessage);
                if (responseValue == I165C_RESPONSE_SUCCESS) {
                    I165C_SetInitializationState(
                        &i165c_initializationState,
                        I165C_FSM_STATE_INITIALIZATION_SET_WARNING_THRESHOLD,
                        I165C_FSM_SHORT_TIME);
                } else {
                    /* Restart initialization process if iso165c responds with an error message */
                    if (responseValue == I165C_RESPONSE_ERROR) {
                        I165C_SetInitializationState(
                            &i165c_initializationState,
                            I165C_FSM_STATE_INITIALIZATION_HAS_NEVER_RUN,
                            I165C_FSM_SHORT_TIME);
                    }
                    /* Issue: 621 */
                }
                break;
            case I165C_FSM_STATE_INITIALIZATION_SET_WARNING_THRESHOLD:
                CANTX_ImdRequest(CANTX_IMD_REQUEST_INITIALIZATION_SET_WARNING_THRESHOLD);
                I165C_SetInitializationState(
                    &i165c_initializationState,
                    I165C_FSM_STATE_INITIALIZATION_WARNING_THRESHOLD_WAIT_ACK,
                    I165C_FSM_SHORT_TIME);
                break;
            case I165C_FSM_STATE_INITIALIZATION_WARNING_THRESHOLD_WAIT_ACK:
                responseValue = I165C_CheckAcknowledgeArrived(
                    I165C_CMD_S_IMC_SET_R_ISO_WRN_THR, &i165c_initializationState.receptionTries, &i165c_canRxMessage);
                if (responseValue == I165C_RESPONSE_SUCCESS) {
                    I165C_SetInitializationState(
                        &i165c_initializationState,
                        I165C_FSM_STATE_INITIALIZATION_DISABLE_MEASUREMENT,
                        I165C_FSM_SHORT_TIME);
                } else {
                    /* Restart initialization process if iso165c responds with an error message */
                    if (responseValue == I165C_RESPONSE_ERROR) {
                        I165C_SetInitializationState(
                            &i165c_initializationState,
                            I165C_FSM_STATE_INITIALIZATION_HAS_NEVER_RUN,
                            I165C_FSM_SHORT_TIME);
                    }
                    /* Issue: 621 */
                }
                break;

            case I165C_FSM_STATE_INITIALIZATION_DISABLE_MEASUREMENT:
                I165C_SetMeasurementMode(I165C_DISABLE_MEASUREMENT);
                I165C_SetInitializationState(
                    &i165c_initializationState,
                    I165C_FSM_STATE_INITIALIZATION_DISABLE_MEASUREMENT_WAIT_ACK,
                    I165C_FSM_SHORT_TIME);
                break;

            case I165C_FSM_STATE_INITIALIZATION_DISABLE_MEASUREMENT_WAIT_ACK:
                responseValue = I165C_CheckAcknowledgeArrived(
                    I165C_CMD_S_VIFC_CTL_MEASUREMENT, &i165c_initializationState.receptionTries, &i165c_canRxMessage);
                if (responseValue == I165C_RESPONSE_SUCCESS) {
                    /* Initialized -> switch to next state in IMD state machine */
                    nextState = IMD_FSM_STATE_IMD_ENABLE;
                    /* Reset state machine in case a re-initialization is necessary */
                    I165C_SetInitializationState(
                        &i165c_initializationState, I165C_FSM_STATE_INITIALIZATION_HAS_NEVER_RUN, I165C_FSM_SHORT_TIME);
                } else {
                    /* Restart initialization process if iso165c responds with an error message */
                    if (responseValue == I165C_RESPONSE_ERROR) {
                        I165C_SetInitializationState(
                            &i165c_initializationState,
                            I165C_FSM_STATE_INITIALIZATION_HAS_NEVER_RUN,
                            I165C_FSM_SHORT_TIME);
                    }
                    /* Issue: 621 */
                }
                break;

            default:
                FAS_ASSERT(FAS_TRAP);
                break; /* LCOV_EXCL_LINE */
        }
        i165c_initializationState.triggerEntry--;
    }
    return nextState;
}

static IMD_FSM_STATES_e I165C_Enable(void) {
    IMD_FSM_STATES_e nextState = IMD_FSM_STATE_IMD_ENABLE; /* stay in enable state */
    bool earlyExit             = false;
    I165C_RESPONSE_RETURN_VALUE_e responseValue;

    if (i165c_enableState.timer > 0u) {
        if ((--i165c_enableState.timer) > 0u) {
            i165c_enableState.triggerEntry--;
            earlyExit = true;
        }
    }

    if (earlyExit == false) {
        switch (i165c_enableState.currentState) {
            case I165C_FSM_STATE_ENABLE_HAS_NEVER_RUN:
                /* Close negative relay */
                I165C_SetRelayState(I165C_D_VIFC_HV_RELAIS_NEGATIVE, I165C_RELAY_STATE_CLOSED);
                /* Close positive relay */
                I165C_SetRelayState(I165C_D_VIFC_HV_RELAIS_POSITIVE, I165C_RELAY_STATE_CLOSED);
                /* Switch to next state */
                I165C_SetEnableState(
                    &i165c_enableState, I165C_FSM_STATE_ENABLE_REQUEST_NEGATIVE_HV_RELAY_STATE, I165C_FSM_SHORT_TIME);
                break;

            case I165C_FSM_STATE_ENABLE_REQUEST_NEGATIVE_HV_RELAY_STATE:
                I165C_RequestRelayState(I165C_D_VIFC_HV_RELAIS_NEGATIVE);
                I165C_SetEnableState(
                    &i165c_enableState, I165C_FSM_STATE_ENABLE_CHECK_NEGATIVE_HV_RELAY_STATE, I165C_FSM_SHORT_TIME);
                break;

            case I165C_FSM_STATE_ENABLE_CHECK_NEGATIVE_HV_RELAY_STATE:
                responseValue = I165C_CheckResponse(I165C_CMD_S_VIFC_GET_HV_RELAIS, &i165c_canRxMessage);
                if (responseValue == I165C_RESPONSE_SUCCESS) {
                    if (CANRX_ImdResponseCheckRelayState(
                            &i165c_canRxMessage.data[0], I165C_D_VIFC_HV_RELAIS_NEGATIVE, I165C_RELAY_STATE_CLOSED) ==
                        true) {
                        i165c_enableState.receptionTries        = 0u;
                        i165c_enableState.receptionTriesMessage = 0u;

                        /* Request state of positive HV relay */
                        I165C_RequestRelayState(I165C_D_VIFC_HV_RELAIS_POSITIVE);
                        I165C_SetEnableState(
                            &i165c_enableState,
                            I165C_FSM_STATE_ENABLE_CHECK_POSITIVE_HV_RELAY_STATE,
                            I165C_FSM_SHORT_TIME);
                    } else {
                        i165c_enableState.receptionTries++;
                        /* Issue: 621 */
                    }
                } else {
                    /* Reset to previous state to send the request message again */
                    if (responseValue == I165C_RESPONSE_ERROR) {
                        I165C_SetEnableState(&i165c_enableState, i165c_enableState.previousState, I165C_FSM_SHORT_TIME);
                    }
                    i165c_enableState.receptionTriesMessage++;
                    /* Issue: 621 */
                    if (i165c_enableState.receptionTriesMessage > I165C_TRANSMISSION_ATTEMPTS) {
                        I165C_SetEnableState(
                            &i165c_enableState,
                            I165C_FSM_STATE_ENABLE_REQUEST_NEGATIVE_HV_RELAY_STATE,
                            I165C_FSM_SHORT_TIME);
                        i165c_enableState.receptionTriesMessage = 0u;
                    }
                    /* Issue: 621 */
                }
                break;

            case I165C_FSM_STATE_ENABLE_CHECK_POSITIVE_HV_RELAY_STATE:
                responseValue = I165C_CheckResponse(I165C_CMD_S_VIFC_GET_HV_RELAIS, &i165c_canRxMessage);
                if (responseValue == I165C_RESPONSE_SUCCESS) {
                    if (CANRX_ImdResponseCheckRelayState(
                            &i165c_canRxMessage.data[0], I165C_D_VIFC_HV_RELAIS_POSITIVE, I165C_RELAY_STATE_CLOSED) ==
                        true) {
                        i165c_enableState.receptionTries        = 0u;
                        i165c_enableState.receptionTriesMessage = 0u;

                        I165C_SetEnableState(
                            &i165c_enableState, I165C_FSM_STATE_ENABLE_START_MEASUREMENT, I165C_FSM_SHORT_TIME);
                    } else {
                        i165c_enableState.receptionTries++;
                        /* Issue: 621 */
                    }
                } else {
                    /* Reset to previous state to send the request message again*/
                    if (responseValue == I165C_RESPONSE_ERROR) {
                        I165C_SetEnableState(&i165c_enableState, i165c_enableState.previousState, I165C_FSM_SHORT_TIME);
                    }
                    i165c_enableState.receptionTriesMessage++;
                    /* Issue: 621 */
                }
                break;

            case I165C_FSM_STATE_ENABLE_START_MEASUREMENT:
                I165C_SetMeasurementMode(I165C_ENABLE_MEASUREMENT);
                I165C_SetEnableState(
                    &i165c_enableState, I165C_FSM_STATE_ENABLE_START_MEASUREMENT_WAIT_ACK, I165C_FSM_SHORT_TIME);
                break;

            case I165C_FSM_STATE_ENABLE_START_MEASUREMENT_WAIT_ACK:
                responseValue = I165C_CheckAcknowledgeArrived(
                    I165C_CMD_S_VIFC_CTL_MEASUREMENT, &i165c_enableState.receptionTries, &i165c_canRxMessage);
                if (responseValue == I165C_RESPONSE_SUCCESS) {
                    /* Enabled -> switch to next state in IMD state machine */
                    nextState = IMD_FSM_STATE_RUNNING;
                    /* Reset state machine in case a re-enabling is necessary */
                    I165C_SetEnableState(
                        &i165c_enableState, I165C_FSM_STATE_ENABLE_HAS_NEVER_RUN, I165C_FSM_SHORT_TIME);
                } else {
                    /* Reset to previous state so the request message gets send again */
                    if (responseValue == I165C_RESPONSE_ERROR) {
                        I165C_SetEnableState(&i165c_enableState, i165c_enableState.previousState, I165C_FSM_SHORT_TIME);
                    }
                    /* Issue: 621 */
                }
                break;
        }
    }
    return nextState;
}

static IMD_FSM_STATES_e I165C_Running(DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring) {
    FAS_ASSERT(pTableInsulationMonitoring != NULL_PTR);
    IMD_FSM_STATES_e nextState = IMD_FSM_STATE_RUNNING; /* stay in running state */
    bool earlyExit             = false;
    I165C_RESPONSE_RETURN_VALUE_e responseValue;

    if (i165c_runningState.timer > 0u) {
        if ((--i165c_runningState.timer) > 0u) {
            i165c_runningState.triggerEntry--;
            earlyExit = true;
        }
    }

    if (earlyExit == false) {
        switch (i165c_runningState.currentState) {
            case I165C_FSM_STATE_RUNNING_HAS_NEVER_RUN:
                /* Switch to next state */
                I165C_SetRunningState(
                    &i165c_runningState, I165C_FSM_STATE_RUNNING_READ_RESISTANCE, I165C_FSM_SHORT_TIME);
                break;

            case I165C_FSM_STATE_RUNNING_READ_RESISTANCE:
                CANTX_ImdRequest(CANTX_IMD_REQUEST_READ_RESISTANCE);
                I165C_SetRunningState(
                    &i165c_runningState, I165C_FSM_STATE_RUNNING_READ_RESISTANCE_WAIT_ACK, I165C_FSM_SHORT_TIME);
                break;

            case I165C_FSM_STATE_RUNNING_READ_RESISTANCE_WAIT_ACK:
                responseValue = I165C_CheckAcknowledgeArrived(
                    I165C_CMD_S_IMC_GET_R_ISO, &i165c_runningState.receptionTries, &i165c_canRxMessage);
                if (responseValue == I165C_RESPONSE_SUCCESS) {
                    CANRX_ImdResponseReadInsulationResistance(&i165c_canRxMessage.data[0], pTableInsulationMonitoring);

                    CANRX_ImdResponseCheckInsulationFaultTendency(
                        &i165c_canRxMessage.data[0], pTableInsulationMonitoring);

                    I165C_SetRunningState(
                        &i165c_runningState, I165C_FSM_STATE_RUNNING_GET_MEASUREMENT, I165C_FSM_SHORT_TIME);
                } else {
                    /* Reset to previous state to send the request message again */
                    if (responseValue == I165C_RESPONSE_ERROR) {
                        I165C_SetRunningState(
                            &i165c_runningState, i165c_runningState.previousState, I165C_FSM_SHORT_TIME);
                    }
                    /* Issue: 621 */
                }
                break;

            case I165C_FSM_STATE_RUNNING_GET_MEASUREMENT:
                if (I165C_GetImdInfo(&i165c_canRxMessage) == false) {
                    i165c_runningState.receptionTriesMessage++;
                    /* Issue: 621 */
                    /* IMD_Info not coming: restart initialization procedure?
                    if (i165c_runningState.receptionTriesMessage >= I165C_IMD_INFO_RECEIVE_ATTEMPTS) {*/
                } else {
                    i165c_runningState.receptionTriesMessage = 0u;

                    CANRX_ImdInfoGetDataFromMessage(&i165c_canRxMessage.data[0], pTableInsulationMonitoring);
                    I165C_IsInsulationMeasurementValid(pTableInsulationMonitoring);
                }
                /* Restart measurement cycle */
                I165C_SetRunningState(
                    &i165c_runningState, I165C_FSM_STATE_RUNNING_READ_RESISTANCE, I165C_FSM_SHORT_TIME);
                break;

            default:
                /* invalid state */
                nextState = IMD_FSM_STATE_ERROR;
                FAS_ASSERT(FAS_TRAP);
                break;
        }
    }
    return nextState;
}

/** Disable state machine */
static IMD_FSM_STATES_e I165C_Disable(void) {
    IMD_FSM_STATES_e nextState = IMD_FSM_STATE_SHUTDOWN; /* stay in shutdown state */
    bool earlyExit             = false;
    I165C_RESPONSE_RETURN_VALUE_e responseValue;

    if (i165c_disableState.timer > 0u) {
        if ((--i165c_disableState.timer) > 0u) {
            i165c_disableState.triggerEntry--;
            earlyExit = true;
        }
    }

    if (earlyExit == false) {
        switch (i165c_disableState.currentState) {
            case I165C_FSM_STATE_DISABLE_HAS_NEVER_RUN:
                /* The I165C_Running state-machine, does not know when the
                 * disable command is received by the superimposed IMD state
                 * machine. Thus, the I165C_Running state machine needs to be
                 * reset at this point to correctly call the running state
                 * machine after re-enabling. */
                I165C_SetRunningState(&i165c_runningState, I165C_FSM_STATE_RUNNING_HAS_NEVER_RUN, I165C_FSM_SHORT_TIME);

                /* Request stop of measurement */
                I165C_SetMeasurementMode(I165C_DISABLE_MEASUREMENT);
                I165C_SetDisableState(
                    &i165c_disableState, I165C_FSM_STATE_MEASUREMENT_STOPPED_WAIT_ACK, I165C_FSM_SHORT_TIME);
                break;

            case I165C_FSM_STATE_MEASUREMENT_STOPPED_WAIT_ACK:
                responseValue = I165C_CheckAcknowledgeArrived(
                    I165C_CMD_S_VIFC_CTL_MEASUREMENT, &i165c_disableState.receptionTries, &i165c_canRxMessage);
                if (responseValue == I165C_RESPONSE_SUCCESS) {
                    I165C_SetDisableState(
                        &i165c_disableState, I165C_FSM_STATE_DISABLE_SET_HV_RELAY_STATE, I165C_FSM_SHORT_TIME);
                } else {
                    /* Reset to previous state to send request message again */
                    if (responseValue == I165C_RESPONSE_ERROR) {
                        I165C_SetDisableState(
                            &i165c_disableState, i165c_disableState.previousState, I165C_FSM_SHORT_TIME);
                    }
                    /* Issue: 621 */
                }
                break;

            case I165C_FSM_STATE_DISABLE_SET_HV_RELAY_STATE:
                /* Open negative relay */
                I165C_SetRelayState(I165C_D_VIFC_HV_RELAIS_NEGATIVE, I165C_RELAY_STATE_OPEN);
                /* Open positive relay */
                I165C_SetRelayState(I165C_D_VIFC_HV_RELAIS_POSITIVE, I165C_RELAY_STATE_OPEN);
                I165C_SetDisableState(
                    &i165c_disableState, I165C_FSM_STATE_DISABLE_REQUEST_NEGATIVE_HV_RELAY_STATE, I165C_FSM_SHORT_TIME);
                break;

            case I165C_FSM_STATE_DISABLE_REQUEST_NEGATIVE_HV_RELAY_STATE:
                I165C_RequestRelayState(I165C_D_VIFC_HV_RELAIS_NEGATIVE);
                I165C_SetDisableState(
                    &i165c_disableState, I165C_FSM_STATE_DISABLE_CHECK_NEGATIVE_HV_RELAY_STATE, I165C_FSM_SHORT_TIME);
                break;

            case I165C_FSM_STATE_DISABLE_CHECK_NEGATIVE_HV_RELAY_STATE:
                responseValue = I165C_CheckResponse(I165C_CMD_S_VIFC_GET_HV_RELAIS, &i165c_canRxMessage);
                if (responseValue == I165C_RESPONSE_SUCCESS) {
                    if (CANRX_ImdResponseCheckRelayState(
                            &i165c_canRxMessage.data[0], I165C_D_VIFC_HV_RELAIS_NEGATIVE, I165C_RELAY_STATE_OPEN) ==
                        true) {
                        i165c_disableState.receptionTries        = 0u;
                        i165c_disableState.receptionTriesMessage = 0u;

                        /* Request state of positive HV relay */
                        I165C_RequestRelayState(I165C_D_VIFC_HV_RELAIS_POSITIVE);
                        I165C_SetDisableState(
                            &i165c_disableState,
                            I165C_FSM_STATE_DISABLE_CHECK_POSITIVE_HV_RELAY_STATE,
                            I165C_FSM_SHORT_TIME);
                    } else {
                        i165c_disableState.receptionTries++;
                        /* Issue: #621 */
                    }
                } else {
                    /* Reset to previous state to send request message again */
                    if (responseValue == I165C_RESPONSE_ERROR) {
                        I165C_SetDisableState(
                            &i165c_disableState, i165c_disableState.previousState, I165C_FSM_SHORT_TIME);
                    }
                    i165c_disableState.receptionTriesMessage++;
                    /* Issue: #621 */
                }
                break;

            case I165C_FSM_STATE_DISABLE_CHECK_POSITIVE_HV_RELAY_STATE:
                responseValue = I165C_CheckResponse(I165C_CMD_S_VIFC_GET_HV_RELAIS, &i165c_canRxMessage);
                if (responseValue == I165C_RESPONSE_SUCCESS) {
                    if (CANRX_ImdResponseCheckRelayState(
                            &i165c_canRxMessage.data[0], I165C_D_VIFC_HV_RELAIS_POSITIVE, I165C_RELAY_STATE_OPEN) ==
                        true) {
                        i165c_disableState.receptionTries        = 0u;
                        i165c_disableState.receptionTriesMessage = 0u;

                        /* Reset disable state machine in case a another disabling is necessary */
                        I165C_SetDisableState(
                            &i165c_disableState, I165C_FSM_STATE_DISABLE_HAS_NEVER_RUN, I165C_FSM_SHORT_TIME);

                        /* IMD successfully disabled -> switch to next state in superimposed IMD state machine */
                        nextState = IMD_FSM_STATE_IMD_ENABLE;
                    } else {
                        i165c_disableState.receptionTries++;
                        /* Issue: 621 */
                    }
                } else {
                    /* Reset to previous state to send request message again */
                    if (responseValue == I165C_RESPONSE_ERROR) {
                        I165C_SetDisableState(
                            &i165c_disableState, i165c_disableState.previousState, I165C_FSM_SHORT_TIME);
                    }
                    i165c_disableState.receptionTriesMessage++;
                    /* Issue: 621 */
                }
                break;
        }
    }
    return nextState;
}

/*========== Extern Function Implementations ================================*/

extern IMD_FSM_STATES_e IMD_ProcessInitializationState(void) {
    return I165C_Initialize();
}

extern IMD_FSM_STATES_e IMD_ProcessEnableState(void) {
    return I165C_Enable();
}

extern IMD_FSM_STATES_e IMD_ProcessRunningState(DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring) {
    FAS_ASSERT(pTableInsulationMonitoring != NULL_PTR);
    return I165C_Running(pTableInsulationMonitoring);
}

extern IMD_FSM_STATES_e IMD_ProcessShutdownState(void) {
    return I165C_Disable();
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern bool TEST_I165C_CheckResponse(uint8_t command, CAN_BUFFER_ELEMENT_s *canMessage) {
    return I165C_CheckResponse(command, canMessage);
}
extern bool TEST_I165C_GetImdInfo(CAN_BUFFER_ELEMENT_s *canMessage) {
    return I165C_GetImdInfo(canMessage);
}
extern bool TEST_I165C_CheckAcknowledgeArrived(uint8_t command, uint8_t *tries, CAN_BUFFER_ELEMENT_s *canMessage) {
    return I165C_CheckAcknowledgeArrived(command, tries, canMessage);
}

#endif
