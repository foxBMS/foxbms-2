/**
 *
 * @copyright &copy; 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    bender_iso165c.c
 * @author  foxBMS Team
 * @date    2019-04-07 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS
 * @prefix  I165C
 *
 * @brief   Driver for the insulation monitoring
 *
 * @details main file of bender iso165C and iso165C-1 driver
 *
 */

/*========== Includes =======================================================*/
#include "bender_iso165c.h"

#include "database_cfg.h"

#include "can.h"
#include "can_cfg_rx-message-definitions.h"
#include "can_cfg_tx-message-definitions.h"
#include "can_helper.h"
#include "database.h"
#include "ftask.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** state machine short time definition in trigger calls until next state is processed */
#define I165C_FSM_SHORT_TIME (1u)

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

static CAN_BUFFER_ELEMENT_s i165c_canTxMessage = {0u};
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
 * @brief   Reset CAN data.
 * @details Used before starting a new transmission.
 * @param[in,out] pCanMessage  pointer to CAN data to be reset
 */
static void I165C_ResetCanData(CAN_BUFFER_ELEMENT_s *pCanMessage);

/**
 * @brief   Write data in data word for CAN transmission.
 * @param   dataWord    data word ("position") in CAN structure to be written
 *                      to (see data sheet page 15)
 * @param   data        data to be written in data word
 * @param[in] pCanMessage  CAN structure to be used for transmission
 */
static void I165C_WriteDataWord(uint8_t dataWord, uint16_t data, CAN_BUFFER_ELEMENT_s *pCanMessage);

/**
 * @brief   Get data in data word from CAN transmission.
 * @param   dataWord  data word ("position") in CAN structure to be read from
 *                    (see data sheet page 15)
 * @param[out] pData  pointer where to put read data from data word
 * @param canMessage  CAN structure used for transmission
 */
static void I165C_ReadDataWord(uint8_t dataWord, uint16_t *pData, CAN_BUFFER_ELEMENT_s canMessage);

/**
 * @brief   Get data in data word from CAN transmission, for the specific
 *          IMD_Info message.
 * @param   dataWord    data word ("position") in CAN structure to be read from
 *                      (see data sheet page 15)
 * @param[out] pData  pointer where to put read data from data word
 * @param   canMessage  CAN structure used for transmission
 */
static void I165C_ReadDataWordImdInfo(uint8_t dataWord, uint16_t *pData, CAN_BUFFER_ELEMENT_s canMessage);

/**
 * @brief   Get data in data byte from CAN transmission.
 * @param   dataByte    data byte ("position") in CAN structure to be read from
 *                      (see data sheet page 15)
 * @param[out] pData  pointer where to put read data from data byte
 * @param   canMessage  CAN structure used for transmission
 */
static void I165C_ReadDataByte(uint8_t dataByte, uint8_t *pData, CAN_BUFFER_ELEMENT_s canMessage);

/**
 * @brief   Compose CAN message for CAN transmission.
 * @details Write CMD byte.
 * @param   id          CAN ID to use
 * @param   command     command to be used (see data page 15 section 6.3 and
 *                      further)
 * @param[out] pCanMessage  pointer to CAN structure to be used for transmission
 */
static void I165C_WriteCmd(uint8_t id, uint8_t command, CAN_BUFFER_ELEMENT_s *pCanMessage);

/**
 * @brief   Check if iso165c acknowledged reception of sent message
 *          and get corresponding data.
 * @details Gets data from the CAN module through a queue.
 * @param   command     check if this command is sent by iso165c to acknowledge
 *                      reception
 * @param[in] pCanMessage  pointer to CAN data sent by the iso165c
 * @return  true if transmission acknowledged, false otherwise
 */
static bool I165C_CheckResponse(uint8_t command, CAN_BUFFER_ELEMENT_s *pCanMessage);

/**
 * @brief   Get IMD Info from iso165c
 * @details Gets data from the CAN module through a queue.
 * @param[in] pCanMessage  pointer to CAN data sent by the iso165c
 * @return  true if IMD_Info message was received, false otherwise
 */
static bool I165C_GetImdInfo(CAN_BUFFER_ELEMENT_s *pCanMessage);

/**
 * @brief   Check if iso165c was initialized and is running
 * @details Check is made using the CAN IMD_Info data sent by the iso165c.
 *          - the IMC is up and running with no errors:
 *          - insulation measurement deactivated
 *          - self-test executed
 * @param[in] canMessage  IMD_Info to be checked, sent by the iso165c
 * @return  true if IMD_Info was received, false otherwise
 */
static bool I165C_IsSelfTestFinished(CAN_BUFFER_ELEMENT_s canMessage);

/**
 * @brief   Check if iso165c has already been performed previously
 * @details Check is made using the CAN IMD_Info data sent by the iso165c.
 *          - self-test executed
 * @param[in] canMessage  IMD_Info to be checked, sent by the iso165c
 * @return  true if self-test has already been executed, false otherwise
 */
static bool I165C_HasSelfTestBeenExecuted(CAN_BUFFER_ELEMENT_s canMessage);

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
 * @brief   Check state of HV relay
 * @param[in] canMessage  IMD_Info to be checked, sent by the iso165c
 * @param[in] relay       positive or negative relay
 * @param[in] relayState  relay opened or closed
 * @return  true if state matches, otherwise false
 */
static bool I165C_CheckRelayState(CAN_BUFFER_ELEMENT_s canMessage, uint8_t relay, uint8_t relayState);

/**
 * @brief   Set measurement mode
 * @param[in] mode set IMD measurement mode
 */
static void I165C_SetMeasurementMode(uint8_t mode);

/**
 * @brief   Check measurement mode
 * @param[in] canMessage  IMD_Info to be checked, sent by the iso165c
 * @param[in] mode IMD measurement mode (activated or deactivated)
 * @return true, if measurement mode matches, otherwise false
 */
static bool I165C_CheckMeasurementMode(CAN_BUFFER_ELEMENT_s canMessage, uint8_t mode);

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
static bool I165C_CheckAcknowledgeArrived(uint8_t command, uint8_t *pTries, CAN_BUFFER_ELEMENT_s *pCanMessage);

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

static void I165C_ResetCanData(CAN_BUFFER_ELEMENT_s *pCanMessage) {
    FAS_ASSERT(pCanMessage != NULL_PTR);
    for (uint8_t i = 0u; i < CAN_DEFAULT_DLC; i++) {
        pCanMessage->data[i] = 0u;
    }
}

static void I165C_WriteDataWord(uint8_t dataWord, uint16_t data, CAN_BUFFER_ELEMENT_s *pCanMessage) {
    FAS_ASSERT(pCanMessage != NULL_PTR);
    /* See data sheet section 6.1 page 15 */
    if (dataWord == I165C_DW1) {
        pCanMessage->data[CAN_BYTE_1_POSITION] = (uint8_t)(data & 0xFFu);
        pCanMessage->data[CAN_BYTE_2_POSITION] = (uint8_t)((data >> 8u) & 0xFFu);
    }
    if (dataWord == I165C_DW2) {
        pCanMessage->data[CAN_BYTE_3_POSITION] = (uint8_t)(data & 0xFFu);
        pCanMessage->data[CAN_BYTE_4_POSITION] = (uint8_t)((data >> 8u) & 0xFFu);
    }
}

static void I165C_ReadDataWord(uint8_t dataWord, uint16_t *pData, CAN_BUFFER_ELEMENT_s canMessage) {
    FAS_ASSERT(pData != NULL_PTR);
    /* See data sheet section 6.1 page 15 */
    if (dataWord == I165C_DW1) {
        *pData = canMessage.data[CAN_BYTE_1_POSITION];
        *pData |= (((uint16_t)canMessage.data[CAN_BYTE_2_POSITION]) << 8u) & 0xFF00u;
    }
    if (dataWord == I165C_DW2) {
        *pData = canMessage.data[CAN_BYTE_3_POSITION];
        *pData |= (((uint16_t)canMessage.data[CAN_BYTE_4_POSITION]) << 8u) & 0xFF00u;
    }
    if (dataWord == I165C_DW3) {
        *pData = canMessage.data[CAN_BYTE_5_POSITION];
        *pData |= (((uint16_t)canMessage.data[CAN_BYTE_6_POSITION]) << 8u) & 0xFF00u;
    }
}

static void I165C_ReadDataWordImdInfo(uint8_t dataWord, uint16_t *pData, CAN_BUFFER_ELEMENT_s canMessage) {
    FAS_ASSERT(pData != NULL_PTR);
    /* See data sheet section 6.1 page 15 */
    if (dataWord == I165C_DW1) {
        *pData = canMessage.data[CAN_BYTE_0_POSITION];
        *pData |= (((uint16_t)canMessage.data[CAN_BYTE_1_POSITION]) << 8u) & 0xFF00u;
    }
    if (dataWord == I165C_DW2) {
        *pData = canMessage.data[CAN_BYTE_2_POSITION];
        *pData |= (((uint16_t)canMessage.data[CAN_BYTE_3_POSITION]) << 8u) & 0xFF00u;
    }
    if (dataWord == I165C_DW3) {
        *pData = canMessage.data[CAN_BYTE_4_POSITION];
        *pData |= (((uint16_t)canMessage.data[CAN_BYTE_5_POSITION]) << 8u) & 0xFF00u;
    }
}

static void I165C_ReadDataByte(uint8_t dataByte, uint8_t *pData, CAN_BUFFER_ELEMENT_s canMessage) {
    FAS_ASSERT(pData != NULL_PTR);
    /* See data sheet section 6.1 page 15 */
    switch (dataByte) {
        case I165C_DB1:
            *pData = canMessage.data[CAN_BYTE_1_POSITION];
            break;
        case I165C_DB2:
            *pData = canMessage.data[CAN_BYTE_2_POSITION];
            break;
        case I165C_DB3:
            *pData = canMessage.data[CAN_BYTE_3_POSITION];
            break;
        case I165C_DB4:
            *pData = canMessage.data[CAN_BYTE_4_POSITION];
            break;
        default:
            *pData = 0u;
            break;
    }
}

static void I165C_WriteCmd(uint8_t id, uint8_t command, CAN_BUFFER_ELEMENT_s *pCanMessage) {
    FAS_ASSERT(pCanMessage != NULL_PTR);
    /* CAN message is a request, set ID accordingly */
    pCanMessage->id     = id;
    pCanMessage->idType = I165C_TX_MESSAGE_IDENTIFIER_TYPE;
    /* First byte contains the CMD field */
    pCanMessage->data[CAN_BYTE_0_POSITION] = command;
}

static bool I165C_CheckResponse(uint8_t command, CAN_BUFFER_ELEMENT_s *pCanMessage) {
    FAS_ASSERT(pCanMessage != NULL_PTR);
    bool messageReceived   = false;
    uint8_t numberItems    = 0u;
    uint8_t queueReadTries = I165C_MAX_QUEUE_READS;

    /* Use loop on queue because IMD_info message could come meanwhile */
    do {
        numberItems = OS_GetNumberOfStoredMessagesInQueue(ftsk_imdCanDataQueue);
        if (numberItems > 0u) {
            if (OS_ReceiveFromQueue(ftsk_imdCanDataQueue, (void *)pCanMessage, 0u) == OS_SUCCESS) {
                /* data queue was no empty */
                if ((command == pCanMessage->data[CAN_BYTE_0_POSITION]) && (pCanMessage->id == CANRX_IMD_RESPONSE_ID) &&
                    (pCanMessage->idType == I165C_RX_MESSAGE_IDENTIFIER_TYPE)) {
                    messageReceived = true;
                    break;
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
                /* data queue was no empty */
                if (pCanMessage->id == I165C_MESSAGETYPE_IMD_INFO) {
                    imdInfoReceived = true;
                    break;
                }
            }
        }
        queueReadTries--;
    } while ((numberItems > 0u) && (queueReadTries > 0u));

    return imdInfoReceived;
}

static bool I165C_IsSelfTestFinished(CAN_BUFFER_ELEMENT_s canMessage) {
    bool initialized = true;
    uint16_t data    = 0u;

    /* Extract D_IMC_STATUS */
    I165C_ReadDataWordImdInfo(I165C_DW2, &data, canMessage);
    /* I165C_SELFTEST_RUNNING bit = 1 in IMD_Info DW2: self test running */
    uint16_t selfTestState = (data & (1u << I165C_SELFTEST_RUNNING_SHIFT));
    if (selfTestState != (1u << I165C_SELFTEST_RUNNING_SHIFT)) {
        /* self test not running */
        initialized = false;
    }

    /* Extract D_VIFC_STATUS */
    I165C_ReadDataWordImdInfo(I165C_DW3, &data, canMessage);
    /* I165C_INSULATION_MEASUREMENT bit = 0 in IMD_Info DW3: insulation measurement active */
    uint16_t insulationMeasurementState = (data & (1u << I165C_INSULATION_MEASUREMENT_STATUS_SHIFT));
    if (insulationMeasurementState != 0u) {
        /* insulation measurement active */
        initialized = false;
    }
#ifdef I165C_SELF_TEST_LONG
    /* I165C_IMC_SELFTEST_OVERALL_SCENARIO bit = 1 in IMD_Info DW3: selftest overall scenario not executed */
    uint16_t selfTestExecuted = (data & (1u << I165C_IMC_SELFTEST_OVERALL_SCENARIO_SHIFT));
    if (selfTestExecuted != 0u) {
        /* selftest overall scenario not executed */
        initialized = false;
    }
#else
    /* I165C_IMC_SELFTEST_PARAMETERCONFIG_SCENARIO bit = 1 in IMD_Info DW3: selftest parameter config not executed */
    uint16_t selfTestExecuted = (data & (1u << I165C_IMC_SELFTEST_PARAMETERCONFIG_SCENARIO_SHIFT));
    if (selfTestExecuted != 0u) {
        /* selftest parameter scenario not executed */
        initialized = false;
    }
#endif
    return initialized;
}

static bool I165C_HasSelfTestBeenExecuted(CAN_BUFFER_ELEMENT_s canMessage) {
    bool anySelfTestExecuted = false;
    uint16_t data            = 0u;

    /* Extract D_VIFC_STATUS */
    I165C_ReadDataWordImdInfo(I165C_DW3, &data, canMessage);

    /* I165C_IMC_SELFTEST_OVERALL_SCENARIO bit = 1 in IMD_Info DW3: selftest overall scenario not executed */
    uint16_t overallSelfTestExecuted = (data & (1u << I165C_IMC_SELFTEST_OVERALL_SCENARIO_SHIFT));
    if (overallSelfTestExecuted == 0u) {
        /* selftest overall scenario has been executed */
        anySelfTestExecuted = true;
    }
    /* I165C_IMC_SELFTEST_PARAMETERCONFIG_SCENARIO bit = 1 in IMD_Info DW3: selftest parameter config not executed */
    uint16_t parameterSelfTestExecuted = (data & (1u << I165C_IMC_SELFTEST_PARAMETERCONFIG_SCENARIO_SHIFT));
    if (parameterSelfTestExecuted == 0u) {
        /* selftest parameter scenario has been executed */
        anySelfTestExecuted = true;
    }
    return anySelfTestExecuted;
}

static void I165C_SetRelayState(uint8_t relay, uint8_t relayState) {
    FAS_ASSERT((relay == I165C_D_VIFC_HV_RELAIS_NEGATIVE) || (relay == I165C_D_VIFC_HV_RELAIS_POSITIVE));
    FAS_ASSERT((relayState == I165C_RELAY_STATE_OPEN) || (relayState == I165C_RELAY_STATE_CLOSED));
    /* Reset CAN message buffer */
    I165C_ResetCanData(&i165c_canTxMessage);
    /* Assemble CAN message */
    I165C_WriteCmd(I165C_MESSAGETYPE_IMD_REQUEST, I165C_CMD_S_VIFC_SET_HV_RELAIS, &i165c_canTxMessage);
    I165C_WriteDataWord(I165C_D_VIFC_HV_RELAIS_SET_REQUEST, relay, &i165c_canTxMessage);
    I165C_WriteDataWord(I165C_D_VIFC_HV_RELAIS_STATE_SET_REQUEST, relayState, &i165c_canTxMessage);
    /* Transmit CAN message */
    CAN_DataSend(I165C_CAN_NODE, i165c_canTxMessage.id, i165c_canTxMessage.idType, i165c_canTxMessage.data);
}

static void I165C_RequestRelayState(uint8_t relay) {
    FAS_ASSERT((relay == I165C_D_VIFC_HV_RELAIS_NEGATIVE) || (relay == I165C_D_VIFC_HV_RELAIS_POSITIVE));
    /* Reset CAN message buffer */
    I165C_ResetCanData(&i165c_canTxMessage);
    /* Assemble CAN message */
    I165C_WriteCmd(I165C_MESSAGETYPE_IMD_REQUEST, I165C_CMD_S_VIFC_GET_HV_RELAIS, &i165c_canTxMessage);
    I165C_WriteDataWord(I165C_D_VIFC_HV_RELAIS_GET_REQUEST, relay, &i165c_canTxMessage);
    /* Transmit CAN message */
    CAN_DataSend(I165C_CAN_NODE, i165c_canTxMessage.id, i165c_canTxMessage.idType, i165c_canTxMessage.data);
}

static bool I165C_CheckRelayState(CAN_BUFFER_ELEMENT_s canMessage, uint8_t relay, uint8_t relayState) {
    FAS_ASSERT((relay == I165C_D_VIFC_HV_RELAIS_NEGATIVE) || (relay == I165C_D_VIFC_HV_RELAIS_POSITIVE));
    FAS_ASSERT((relayState == I165C_RELAY_STATE_OPEN) || (relayState == I165C_RELAY_STATE_CLOSED));
    bool checkSuccess = true;
    uint16_t data     = 0u;

    I165C_ReadDataWord(I165C_D_VIFC_HV_RELAIS_GET_RESPONSE, &data, canMessage);
    /* IMD_Response DW1: relay */
    if (relay != data) {
        /* not request relay */
        checkSuccess = false;
    }

    I165C_ReadDataWord(I165C_D_VIFC_HV_RELAIS_STATE_GET_RESPONSE, &data, canMessage);
    /* IMD_Response DW2: relay state */
    if (relayState != data) {
        /* relay state does not match expected state */
        checkSuccess = false;
    }
    return checkSuccess;
}

static void I165C_SetMeasurementMode(uint8_t mode) {
    FAS_ASSERT((mode == I165C_ENABLE_MEASUREMENT) || (mode == I165C_DISABLE_MEASUREMENT));
    /* Reset CAN message buffer */
    I165C_ResetCanData(&i165c_canTxMessage);
    /* Assemble CAN message */
    I165C_WriteCmd(I165C_MESSAGETYPE_IMD_REQUEST, I165C_CMD_S_VIFC_CTL_MEASUREMENT, &i165c_canTxMessage);
    I165C_WriteDataWord(I165C_DW_VIFC_CTL_MEASUREMENT_REQUEST, mode, &i165c_canTxMessage);
    /* Transmit CAN message */
    CAN_DataSend(I165C_CAN_NODE, i165c_canTxMessage.id, i165c_canTxMessage.idType, i165c_canTxMessage.data);
}

static bool I165C_CheckMeasurementMode(CAN_BUFFER_ELEMENT_s canMessage, uint8_t mode) {
    FAS_ASSERT((mode == I165C_ENABLE_MEASUREMENT) || (mode == I165C_DISABLE_MEASUREMENT));
    bool measurementModeMatches = false;
    uint16_t dVIFCStatus        = 0u;

    /* Extract D_VIFC_STATUS word */
    I165C_ReadDataWordImdInfo(I165C_DW3, &dVIFCStatus, i165c_canRxMessage);
    /* Extract measurement mode from D_VIFC_STATUS word */
    uint8_t actualMeasurementMode = dVIFCStatus & (1u << I165C_INSULATION_MEASUREMENT_STATUS_SHIFT);

    /* Check if actual measurement mode matches passed measurement mode */
    if (actualMeasurementMode == mode) {
        /* Insulation measurement deactivated*/
        measurementModeMatches = true;
    }
    return measurementModeMatches;
}

static void I165C_SetAveragingFactor(uint8_t averagingFactor) {
    /* Averaging factor must be in the range 1...20 */
    FAS_ASSERT(averagingFactor != 0u);
    FAS_ASSERT(averagingFactor <= 20u);

    /* Reset CAN message buffer */
    I165C_ResetCanData(&i165c_canTxMessage);
    /* Assemble CAN message */
    I165C_WriteCmd(I165C_MESSAGETYPE_IMD_REQUEST, I165C_CMD_S_IMC_SET_MEAN_FACTOR, &i165c_canTxMessage);
    I165C_WriteDataWord(I165C_D_IMC_MEAN_FACTOR_SET_REQUEST, I165C_MEASUREMENT_AVERAGING_FACTOR, &i165c_canTxMessage);
    /* Transmit CAN message */
    CAN_DataSend(I165C_CAN_NODE, i165c_canTxMessage.id, i165c_canTxMessage.idType, i165c_canTxMessage.data);
}

static bool I165C_CheckAcknowledgeArrived(uint8_t command, uint8_t *pTries, CAN_BUFFER_ELEMENT_s *pCanMessage) {
    FAS_ASSERT(pTries != NULL_PTR);
    FAS_ASSERT(pCanMessage != NULL_PTR);
    /* AXIVION Routine Generic-MissingParameterAssert: command: parameter accepts whole range */

    bool acknowledgeReceived = false;
    if (I165C_CheckResponse(command, pCanMessage) == false) {
        (*pTries)++;
    } else {
        *pTries             = 0u;
        acknowledgeReceived = true;
    }
    return acknowledgeReceived;
}

static IMD_FSM_STATES_e I165C_Initialize(void) {
    IMD_FSM_STATES_e nextState = IMD_FSM_STATE_INITIALIZATION; /* stay in initialization state */
    bool earlyExit             = false;

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
                I165C_ResetCanData(&i165c_canTxMessage);
                I165C_WriteCmd(I165C_MESSAGETYPE_IMD_REQUEST, I165C_CMD_S_VIFC_CTL_LOCK, &i165c_canTxMessage);
                I165C_WriteDataWord(I165C_D_VIFC_LOCK_MODE_CTL_REQUEST, I165C_LOCKMODE_UNLOCKED, &i165c_canTxMessage);
                I165C_WriteDataWord(I165C_D_VIFC_LOCK_PWD_CTL_REQUEST, I165C_UNLOCK_PASSWORD, &i165c_canTxMessage);
                CAN_DataSend(I165C_CAN_NODE, i165c_canTxMessage.id, i165c_canTxMessage.idType, i165c_canTxMessage.data);
                I165C_SetInitializationState(
                    &i165c_initializationState, I165C_FSM_STATE_INITIALIZATION_UNLOCK_WAIT_ACK, I165C_FSM_SHORT_TIME);
                break;

            case I165C_FSM_STATE_INITIALIZATION_UNLOCK_WAIT_ACK:
                if (I165C_CheckAcknowledgeArrived(
                        I165C_CMD_S_VIFC_CTL_LOCK, &i165c_initializationState.receptionTries, &i165c_canRxMessage) ==
                    true) {
                    I165C_SetInitializationState(
                        &i165c_initializationState,
                        I165C_FSM_STATE_INITIALIZATION_CHECK_MEASUREMENT_STATE,
                        I165C_FSM_SHORT_TIME);
                } else {
                    /* Issue: 621 */
                }
                break;

            case I165C_FSM_STATE_INITIALIZATION_CHECK_MEASUREMENT_STATE:
                if (I165C_GetImdInfo(&i165c_canRxMessage) == true) {
                    if (I165C_CheckMeasurementMode(i165c_canRxMessage, I165C_ENABLE_MEASUREMENT) == false) {
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
                if (I165C_CheckAcknowledgeArrived(
                        I165C_CMD_S_VIFC_CTL_MEASUREMENT,
                        &i165c_initializationState.receptionTries,
                        &i165c_canRxMessage) == true) {
                    /* Measurement enabled -> continue with initialization procedure */
                    I165C_SetInitializationState(
                        &i165c_initializationState,
                        I165C_FSM_STATE_INITIALIZATION_REQUEST_HV_RELAY_OPENING,
                        I165C_FSM_SHORT_TIME);
                } else {
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
                if (I165C_CheckResponse(I165C_CMD_S_VIFC_GET_HV_RELAIS, &i165c_canRxMessage) == true) {
                    if (I165C_CheckRelayState(
                            i165c_canRxMessage, I165C_D_VIFC_HV_RELAIS_NEGATIVE, I165C_RELAY_STATE_OPEN) == true) {
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
                    i165c_initializationState.receptionTriesMessage++;
                    /* Issue: 621 */
                }
                break;

            case I165C_FSM_STATE_INITIALIZATION_CHECK_POSITIVE_HV_RELAY_STATE:
                /* Check if HV relays are open and measurement has been stopped */
                if (I165C_CheckResponse(I165C_CMD_S_VIFC_GET_HV_RELAIS, &i165c_canRxMessage) == true) {
                    if (I165C_CheckRelayState(
                            i165c_canRxMessage, I165C_D_VIFC_HV_RELAIS_POSITIVE, I165C_RELAY_STATE_OPEN) == true) {
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
                    i165c_initializationState.receptionTriesMessage++;
                    /* Issue: 621 */
                }
                break;

            case I165C_FSM_STATE_INITIALIZATION_REQUEST_SELF_TEST:
                /* A self test must be requested and can only be carried out
                 when the coupling relays are open. */

                if (I165C_GetImdInfo(&i165c_canRxMessage) == true) {
                    if (I165C_HasSelfTestBeenExecuted(i165c_canRxMessage) == false) {
                        I165C_ResetCanData(&i165c_canTxMessage);
                        I165C_WriteCmd(
                            I165C_MESSAGETYPE_IMD_REQUEST, I165C_CMD_S_IMC_CTL_SELFTEST, &i165c_canTxMessage);
#ifdef I165C_SELF_TEST_LONG
                        I165C_WriteDataWord(
                            I165C_D_IMC_SELFTEST_SCR_CTL_REQUEST, I165C_SELFTEST_SCENARIO_OVERALL, &i165c_canTxMessage);
#else /* I165C_SELF_TEST_SHORT */
                        I165C_WriteDataWord(
                            I165C_D_IMC_SELFTEST_SCR_CTL_REQUEST,
                            I165C_SELFTEST_SCENARIO_PARAMETERCONFIG,
                            &i165c_canTxMessage);
#endif
                        CAN_DataSend(
                            I165C_CAN_NODE, i165c_canTxMessage.id, i165c_canTxMessage.idType, i165c_canTxMessage.data);
                        i165c_initializationState.receptionTries = 0u;
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
                if (I165C_CheckAcknowledgeArrived(
                        I165C_CMD_S_IMC_CTL_SELFTEST, &i165c_initializationState.receptionTries, &i165c_canRxMessage) ==
                    true) {
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
                }
                break;
            case I165C_FSM_STATE_INITIALIZATION_WAIT_SELF_TEST:
                if (I165C_GetImdInfo(&i165c_canRxMessage) == true) {
                    if (I165C_IsSelfTestFinished(i165c_canRxMessage) == false) {
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
                if (I165C_CheckAcknowledgeArrived(
                        I165C_CMD_S_IMC_SET_MEAN_FACTOR,
                        &i165c_initializationState.receptionTries,
                        &i165c_canRxMessage) == true) {
                    I165C_SetInitializationState(
                        &i165c_initializationState,
                        I165C_FSM_STATE_INITIALIZATION_SET_ERROR_THRESHOLD,
                        I165C_FSM_SHORT_TIME);
                } else {
                    /* Issue: 621 */
                }
                break;

            case I165C_FSM_STATE_INITIALIZATION_SET_ERROR_THRESHOLD:
                I165C_ResetCanData(&i165c_canTxMessage);
                I165C_WriteCmd(I165C_MESSAGETYPE_IMD_REQUEST, I165C_CMD_S_IMC_SET_R_ISO_ERR_THR, &i165c_canTxMessage);
                I165C_WriteDataWord(
                    I165C_D_IMC_R_ISO_ERR_THR_SET_REQUEST, I165C_ERROR_THRESHOLD_kOhm, &i165c_canTxMessage);
                CAN_DataSend(I165C_CAN_NODE, i165c_canTxMessage.id, i165c_canTxMessage.idType, i165c_canTxMessage.data);
                I165C_SetInitializationState(
                    &i165c_initializationState,
                    I165C_FSM_STATE_INITIALIZATION_ERROR_THRESHOLD_WAIT_ACK,
                    I165C_FSM_SHORT_TIME);
                break;
            case I165C_FSM_STATE_INITIALIZATION_ERROR_THRESHOLD_WAIT_ACK:
                if (I165C_CheckAcknowledgeArrived(
                        I165C_CMD_S_IMC_SET_R_ISO_ERR_THR,
                        &i165c_initializationState.receptionTries,
                        &i165c_canRxMessage) == true) {
                    I165C_SetInitializationState(
                        &i165c_initializationState,
                        I165C_FSM_STATE_INITIALIZATION_SET_WARNING_THRESHOLD,
                        I165C_FSM_SHORT_TIME);
                } else {
                    /* Issue: 621 */
                }
                break;
            case I165C_FSM_STATE_INITIALIZATION_SET_WARNING_THRESHOLD:
                I165C_ResetCanData(&i165c_canTxMessage);
                I165C_WriteCmd(I165C_MESSAGETYPE_IMD_REQUEST, I165C_CMD_S_IMC_SET_R_ISO_ERR_WRN, &i165c_canTxMessage);
                I165C_WriteDataWord(
                    I165C_D_IMC_R_ISO_ERR_WRN_SET_REQUEST, I165C_WARNING_THRESHOLD_kOhm, &i165c_canTxMessage);
                CAN_DataSend(I165C_CAN_NODE, i165c_canTxMessage.id, i165c_canTxMessage.idType, i165c_canTxMessage.data);
                I165C_SetInitializationState(
                    &i165c_initializationState,
                    I165C_FSM_STATE_INITIALIZATION_WARNING_THRESHOLD_WAIT_ACK,
                    I165C_FSM_SHORT_TIME);
                break;
            case I165C_FSM_STATE_INITIALIZATION_WARNING_THRESHOLD_WAIT_ACK:
                if (I165C_CheckAcknowledgeArrived(
                        I165C_CMD_S_IMC_SET_R_ISO_ERR_WRN,
                        &i165c_initializationState.receptionTries,
                        &i165c_canRxMessage) == true) {
                    I165C_SetInitializationState(
                        &i165c_initializationState,
                        I165C_FSM_STATE_INITIALIZATION_DISABLE_MEASUREMENT,
                        I165C_FSM_SHORT_TIME);
                } else {
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
                if (I165C_CheckAcknowledgeArrived(
                        I165C_CMD_S_VIFC_CTL_MEASUREMENT,
                        &i165c_initializationState.receptionTries,
                        &i165c_canRxMessage) == true) {
                    /* Initialized -> switch to next state in IMD state machine */
                    nextState = IMD_FSM_STATE_IMD_ENABLE;
                    /* Reset state machine in case a re-initialization is necessary */
                    I165C_SetInitializationState(
                        &i165c_initializationState, I165C_FSM_STATE_INITIALIZATION_HAS_NEVER_RUN, I165C_FSM_SHORT_TIME);
                } else {
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
                if (I165C_CheckResponse(I165C_CMD_S_VIFC_GET_HV_RELAIS, &i165c_canRxMessage) == true) {
                    if (I165C_CheckRelayState(
                            i165c_canRxMessage, I165C_D_VIFC_HV_RELAIS_NEGATIVE, I165C_RELAY_STATE_CLOSED) == true) {
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
                if (I165C_CheckResponse(I165C_CMD_S_VIFC_GET_HV_RELAIS, &i165c_canRxMessage) == true) {
                    if (I165C_CheckRelayState(
                            i165c_canRxMessage, I165C_D_VIFC_HV_RELAIS_POSITIVE, I165C_RELAY_STATE_CLOSED) == true) {
                        i165c_enableState.receptionTries        = 0u;
                        i165c_enableState.receptionTriesMessage = 0u;

                        I165C_SetEnableState(
                            &i165c_enableState, I165C_FSM_STATE_ENABLE_START_MEASUREMENT, I165C_FSM_SHORT_TIME);
                    } else {
                        i165c_enableState.receptionTries++;
                        /* Issue: 621 */
                    }
                } else {
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
                if (I165C_CheckAcknowledgeArrived(
                        I165C_CMD_S_VIFC_CTL_MEASUREMENT, &i165c_enableState.receptionTries, &i165c_canRxMessage) ==
                    true) {
                    /* Enabled -> switch to next state in IMD state machine */
                    nextState = IMD_FSM_STATE_RUNNING;
                    /* Reset state machine in case a re-enabling is necessary */
                    I165C_SetEnableState(
                        &i165c_enableState, I165C_FSM_STATE_ENABLE_HAS_NEVER_RUN, I165C_FSM_SHORT_TIME);
                } else {
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
    uint16_t resistance_kOhm   = 0u;
    uint16_t statusFlags       = 0u;
    uint8_t data1              = 0u;
    uint8_t data2              = 0u;

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
                I165C_ResetCanData(&i165c_canTxMessage);
                I165C_WriteCmd(I165C_MESSAGETYPE_IMD_REQUEST, I165C_CMD_S_IMC_GET_R_ISO, &i165c_canTxMessage);
                CAN_DataSend(I165C_CAN_NODE, i165c_canTxMessage.id, i165c_canTxMessage.idType, i165c_canTxMessage.data);
                i165c_runningState.receptionTries = 0u;
                I165C_SetRunningState(
                    &i165c_runningState, I165C_FSM_STATE_RUNNING_READ_RESISTANCE_WAIT_ACK, I165C_FSM_SHORT_TIME);
                break;

            case I165C_FSM_STATE_RUNNING_READ_RESISTANCE_WAIT_ACK:
                if (I165C_CheckAcknowledgeArrived(
                        I165C_CMD_S_IMC_GET_R_ISO, &i165c_runningState.receptionTries, &i165c_canRxMessage) == true) {
                    /* Extract resistance value from response */
                    I165C_ReadDataWord(I165C_DW1, &resistance_kOhm, i165c_canRxMessage);
                    pTableInsulationMonitoring->insulationResistance_kOhm = resistance_kOhm;

                    /* Extract bias/tendency to the location of the insulation fault (error), if detected */
                    I165C_ReadDataByte(I165C_D_IMC_R_ISO_BIAS_GET_RESPONSE, &data1, i165c_canRxMessage);
                    if (data1 == I165C_BIAS_TO_HV_PLUS) {
                        pTableInsulationMonitoring->dfIsChassisShortToHvPlus  = true;
                        pTableInsulationMonitoring->dfIsChassisShortToHvMinus = false;
                    } else if (data1 == I165C_BIAS_TO_HV_MINUS) {
                        pTableInsulationMonitoring->dfIsChassisShortToHvMinus = true;
                        pTableInsulationMonitoring->dfIsChassisShortToHvPlus  = false;
                    } else {
                        pTableInsulationMonitoring->dfIsChassisShortToHvPlus  = false;
                        pTableInsulationMonitoring->dfIsChassisShortToHvMinus = false;
                    }

                    /* Extract counter value */
                    I165C_ReadDataByte(I165C_D_IMC_R_ISO_CNT_GET_RESPONSE, &data2, i165c_canRxMessage);
                    /* TODO: What to do with this info? */

                    I165C_SetRunningState(
                        &i165c_runningState, I165C_FSM_STATE_RUNNING_GET_MEASUREMENT, I165C_FSM_SHORT_TIME);
                } else {
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

                    /* Get measured resistance */
                    I165C_ReadDataWordImdInfo(I165C_DW1, &resistance_kOhm, i165c_canRxMessage);
                    pTableInsulationMonitoring->insulationResistance_kOhm = resistance_kOhm;

                    /* Get IMC status */
                    I165C_ReadDataWordImdInfo(I165C_DW2, &statusFlags, i165c_canRxMessage);
                    if (0u != (statusFlags & (1u << I165C_INSULATION_FAULT_SHIFT))) {
                        /* Insulation fault */
                        pTableInsulationMonitoring->dfIsCriticalResistanceDetected = true;
                    } else {
                        pTableInsulationMonitoring->dfIsCriticalResistanceDetected = false;
                    }
                    if (0u != (statusFlags & (1u << I165C_CHASSIS_FAULT_SHIFT))) {
                        /* Chassis fault */
                        pTableInsulationMonitoring->dfIsChassisFaultDetected = true;
                    } else {
                        pTableInsulationMonitoring->dfIsChassisFaultDetected = false;
                    }
                    if (0u != (statusFlags & (1u << I165C_SYSTEM_FAILURE_SHIFT))) {
                        /* System failure */
                        pTableInsulationMonitoring->dfIsDeviceErrorDetected = true;
                        pTableInsulationMonitoring->areDeviceFlagsValid     = false;
                    } else {
                        pTableInsulationMonitoring->dfIsDeviceErrorDetected = false;
                        pTableInsulationMonitoring->areDeviceFlagsValid     = true;
                    }
                    if (0u != (statusFlags & (1u << I165C_INSULATION_WARNING_SHIFT))) {
                        /* Insulation warning */
                        pTableInsulationMonitoring->dfIsWarnableResistanceDetected = true;
                    } else {
                        pTableInsulationMonitoring->dfIsWarnableResistanceDetected = false;
                    }

                    /* Get VIFC status */
                    I165C_ReadDataWordImdInfo(I165C_DW3, &statusFlags, i165c_canRxMessage);
                    if (0u != (statusFlags & (1u << I165C_INSULATION_MEASUREMENT_STATUS_SHIFT))) {
                        /* Insulation measurement deactivated*/
                        pTableInsulationMonitoring->isImdRunning = false;
                    } else {
                        pTableInsulationMonitoring->isImdRunning = true;
                    }
                    if (0u != (statusFlags & (1u << I165C_RESISTANCE_VALUE_OUTDATED_SHIFT))) {
                        /* Insulation resistance value outdated */
                        pTableInsulationMonitoring->dfIsMeasurementUpToDate = false;
                    } else {
                        pTableInsulationMonitoring->dfIsMeasurementUpToDate = true;
                    }
                    if ((pTableInsulationMonitoring->areDeviceFlagsValid == true) &&
                        (pTableInsulationMonitoring->isImdRunning == true) &&
                        (pTableInsulationMonitoring->dfIsMeasurementUpToDate == true)) {
                        pTableInsulationMonitoring->isInsulationMeasurementValid = true;
                    } else {
                        pTableInsulationMonitoring->isInsulationMeasurementValid = false;
                    }
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
                if (I165C_CheckAcknowledgeArrived(
                        I165C_CMD_S_VIFC_CTL_MEASUREMENT, &i165c_disableState.receptionTries, &i165c_canRxMessage) ==
                    true) {
                    I165C_SetDisableState(
                        &i165c_disableState, I165C_FSM_STATE_DISABLE_SET_HV_RELAY_STATE, I165C_FSM_SHORT_TIME);
                } else {
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
                if (I165C_CheckResponse(I165C_CMD_S_VIFC_GET_HV_RELAIS, &i165c_canRxMessage) == true) {
                    if (I165C_CheckRelayState(
                            i165c_canRxMessage, I165C_D_VIFC_HV_RELAIS_NEGATIVE, I165C_RELAY_STATE_OPEN) == true) {
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
                        /* Issue: 621 */
                    }
                } else {
                    i165c_disableState.receptionTriesMessage++;
                    /* Issue: 621 */
                }
                break;

            case I165C_FSM_STATE_DISABLE_CHECK_POSITIVE_HV_RELAY_STATE:
                if (I165C_CheckResponse(I165C_CMD_S_VIFC_GET_HV_RELAIS, &i165c_canRxMessage) == true) {
                    if (I165C_CheckRelayState(
                            i165c_canRxMessage, I165C_D_VIFC_HV_RELAIS_POSITIVE, I165C_RELAY_STATE_OPEN) == true) {
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
extern void TEST_I165C_ResetCanData(CAN_BUFFER_ELEMENT_s *canMessage) {
    I165C_ResetCanData(canMessage);
}
extern void TEST_I165C_WriteDataWord(uint8_t dataWord, uint16_t data, CAN_BUFFER_ELEMENT_s *canMessage) {
    I165C_WriteDataWord(dataWord, data, canMessage);
}
extern void TEST_I165C_ReadDataWord(uint8_t dataWord, uint16_t *data, CAN_BUFFER_ELEMENT_s canMessage) {
    I165C_ReadDataWord(dataWord, data, canMessage);
}
extern void TEST_I165C_ReadDataWordImdInfo(uint8_t dataWord, uint16_t *data, CAN_BUFFER_ELEMENT_s canMessage) {
    I165C_ReadDataWordImdInfo(dataWord, data, canMessage);
}
extern void TEST_I165C_ReadDataByte(uint8_t dataByte, uint8_t *data, CAN_BUFFER_ELEMENT_s canMessage) {
    I165C_ReadDataByte(dataByte, data, canMessage);
}
extern void TEST_I165C_WriteCmd(uint8_t id, uint8_t command, CAN_BUFFER_ELEMENT_s *canMessage) {
    I165C_WriteCmd(id, command, canMessage);
}
extern bool TEST_I165C_CheckResponse(uint8_t command, CAN_BUFFER_ELEMENT_s *canMessage) {
    return I165C_CheckResponse(command, canMessage);
}
extern bool TEST_I165C_GetImdInfo(CAN_BUFFER_ELEMENT_s *canMessage) {
    return I165C_GetImdInfo(canMessage);
}
extern bool TEST_I165C_IsSelfTestFinished(CAN_BUFFER_ELEMENT_s canMessage) {
    return I165C_IsSelfTestFinished(canMessage);
}
extern bool TEST_I165C_CheckAcknowledgeArrived(uint8_t command, uint8_t *tries, CAN_BUFFER_ELEMENT_s *canMessage) {
    return I165C_CheckAcknowledgeArrived(command, tries, canMessage);
}

#endif
