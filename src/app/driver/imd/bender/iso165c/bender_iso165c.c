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
 * @file    bender_iso165c.c
 * @author  foxBMS Team
 * @date    2019-04-07 (date of creation)
 * @updated 2021-12-01 (date of last update)
 * @ingroup DRIVERS
 * @prefix  I165C
 *
 * @brief   Driver for the insulation monitoring
 *
 * main file of bender iso165C and iso165C-1 driver
 *
 */

/*========== Includes =======================================================*/
#include "bender_iso165c.h"

#include "database_cfg.h"

#include "can.h"
#include "database.h"
#include "ftask.h"

/*========== Macros and Definitions =========================================*/

#define RESISTANCE_THRESHOLD (50000u)
#define I165C_CAN_NODE       (CAN1_NODE)

/*========== Static Constant and Variable Definitions =======================*/

static CAN_BUFFERELEMENT_s i165c_canTxMessage = {0u};
static CAN_BUFFERELEMENT_s i165c_canRxMessage = {0u};

static DATA_BLOCK_INSULATION_MONITORING_s i165c_insulationMeasurementData = {
    .header.uniqueId = DATA_BLOCK_ID_INSULATION_MONITORING};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/**
 * @brief   Reset CAN data.
 * @details Used before starting a new transmission.
 * @param   canMessage  CAN data to be reset
 */
static void I165C_ResetCanData(CAN_BUFFERELEMENT_s *canMessage);

/**
 * @brief   Write data in data word for CAN transmission.
 * @param   dataWord    data word ("position") in CAN structure to be written
 *                      to (see data sheet page 15)
 * @param   data        data to be written in data word
 * @param   canMessage  CAN structure to be used for transmission
 */
static void I165C_WriteDataWord(uint8_t dataWord, uint16_t data, CAN_BUFFERELEMENT_s *canMessage);

/**
 * @brief   Get data in data word from CAN transmission.
 * @param   dataWord    data word ("position") in CAN structure to be read from
 *                      (see data sheet page 15)
 * @param   data        data to be read from data word
 * @param   canMessage  CAN structure used for transmission
 */
static void I165C_ReadDataWord(uint8_t dataWord, uint16_t *data, CAN_BUFFERELEMENT_s canMessage);

/**
 * @brief   Get data in data word from CAN transmission, for the specific
 *          IMD_Info message.
 * @param   dataWord    data word ("position") in CAN structure to be read from
 *                      (see data sheet page 15)
 * @param   data        data to be read from data word
 * @param   canMessage  CAN structure used for transmission
 */
static void I165C_ReadDataWordImdInfo(uint8_t dataWord, uint16_t *data, CAN_BUFFERELEMENT_s canMessage);

/**
 * @brief   Get data in data byte from CAN transmission.
 * @param   dataWord    data byte ("position") in CAN structure to be read from
 *                      (see data sheet page 15)
 * @param   data        data to be read from data byte
 * @param   canMessage  CAN structure used for transmission
 */
static void I165C_ReadDataByte(uint8_t dataByte, uint8_t *data, CAN_BUFFERELEMENT_s canMessage);

/**
 * @brief   Compose CAN message for CAN transmission.
 * @details Write CMD byte.
 * @param   id          CAN ID to use
 * @param   cmd         command to be used (see data page 15 section 6.3 and
 *                      further)
 * @param   canMessage  CAN structure to be used for transmission
 */
static void I165C_WriteCmd(uint8_t id, uint8_t cmd, CAN_BUFFERELEMENT_s *canMessage);

/**
 * @brief   Check if iso165c acknowledged reception of sent message
 *          and get corresponding data.
 * @details Gets data from the CAN module through a queue.
 * @param   command     check if this command is sent by iso165c to acknowledge
 *                      reception
 * @param   canMessage  CAN data sent by the iso165c
 * @return  true if transmission acknowledged, false otherwise
 */
static bool I165C_CheckResponse(uint8_t command, CAN_BUFFERELEMENT_s *canMessage);

/**
 * @brief   Get IMD Info from iso165c
 * @details Gets data from the CAN module through a queue.
 * @param   canMessage  CAN data sent by the iso165c
 * @return  true if IMD_Info was received, false otherwise
 */
static bool I165C_GetImdInfo(CAN_BUFFERELEMENT_s *canMessage);

/**
 * @brief   Check if iso165c was initialized and is running
 * @details Check is made and the CAN IMD_Info data sent by the iso165c.
 * @param   canMessage  IMD_Info to be checked, sent by the iso165c
 * @return  true if IMD_Info was received, false otherwise
 */
static bool I165C_IsInitialized(CAN_BUFFERELEMENT_s canMessage);

/**
 * @brief   Check if iso165c acknowledged reception of command
 *
 * @details If acknowledged, currentState is set to nextState.
 *          If not acknowledged after allowed number of tries, restart
 *          initialization procedure: currentState is set to
 *          #I165C_STATE_SELFTEST.
 *
 * @param   command         command to be acknowledged
 * @param   currentState    current state of the state machine
 * @param   nextState       next state to go to if command acknowledged
 * @param   tries           variable counting the number of tries for reception
 * @param   canMessage      CAN data sent by the iso165c
 *
 */
static void I165C_CheckAcknowledgeArrived(
    uint8_t command,
    I165C_STATE_e *currentState,
    I165C_STATE_e nextState,
    uint8_t *tries,
    CAN_BUFFERELEMENT_s *canMessage);

/**
 * @brief   trigger function for the i165c driver state machine.
 * @details This function contains the sequence of events in the i165c state
 *          machine.
 *          It must be called time-triggered, every 100ms.
 */
static void I165C_Trigger(void);

/*========== Static Function Implementations ================================*/

static void I165C_ResetCanData(CAN_BUFFERELEMENT_s *canMessage) {
    FAS_ASSERT(canMessage != NULL_PTR);
    for (uint8_t i = 0; i < 8u; i++) {
        canMessage->data[i] = 0u;
    }
}

static void I165C_WriteDataWord(uint8_t dataWord, uint16_t data, CAN_BUFFERELEMENT_s *canMessage) {
    FAS_ASSERT(canMessage != NULL_PTR);
    /* See data sheet section 6.1 page 15 */
    if (dataWord == I165C_DW1) {
        canMessage->data[1u] = data & 0xFFu;
        canMessage->data[2u] = (data >> 8u) & 0xFFu;
    }
    if (dataWord == I165C_DW2) {
        canMessage->data[3u] = data & 0xFFu;
        canMessage->data[4u] = (data >> 8u) & 0xFFu;
    }
}

static void I165C_ReadDataWord(uint8_t dataWord, uint16_t *data, CAN_BUFFERELEMENT_s canMessage) {
    FAS_ASSERT(data != NULL_PTR);
    /* See data sheet section 6.1 page 15 */
    if (dataWord == I165C_DW1) {
        *data = canMessage.data[1u];
        *data |= (((uint16_t)canMessage.data[2u]) << 8u) & 0xFF00u;
    }
    if (dataWord == I165C_DW2) {
        *data = canMessage.data[3u];
        *data |= (((uint16_t)canMessage.data[4u]) << 8u) & 0xFF00u;
    }
    if (dataWord == I165C_DW3) {
        *data = canMessage.data[5u];
        *data |= (((uint16_t)canMessage.data[6u]) << 8u) & 0xFF00u;
    }
}

static void I165C_ReadDataWordImdInfo(uint8_t dataWord, uint16_t *data, CAN_BUFFERELEMENT_s canMessage) {
    FAS_ASSERT(data != NULL_PTR);
    /* See data sheet section 6.1 page 15 */
    if (dataWord == I165C_DW1) {
        *data = canMessage.data[0u];
        *data |= (((uint16_t)canMessage.data[1u]) << 8u) & 0xFF00u;
    }
    if (dataWord == I165C_DW2) {
        *data = canMessage.data[2u];
        *data |= (((uint16_t)canMessage.data[3u]) << 8u) & 0xFF00u;
    }
    if (dataWord == I165C_DW3) {
        *data = canMessage.data[4u];
        *data |= (((uint16_t)canMessage.data[5u]) << 8u) & 0xFF00u;
    }
}

static void I165C_ReadDataByte(uint8_t dataByte, uint8_t *data, CAN_BUFFERELEMENT_s canMessage) {
    FAS_ASSERT(data != NULL_PTR);
    /* See data sheet section 6.1 page 15 */
    switch (dataByte) {
        case I165C_DB1:
            *data = canMessage.data[1u];
            break;
        case I165C_DB2:
            *data = canMessage.data[2u];
            break;
        case I165C_DB3:
            *data = canMessage.data[3u];
            break;
        case I165C_DB4:
            *data = canMessage.data[4u];
            break;
        default:
            *data = 0u;
            break;
    }
}

static void I165C_WriteCmd(uint8_t id, uint8_t cmd, CAN_BUFFERELEMENT_s *canMessage) {
    FAS_ASSERT(canMessage != NULL_PTR);
    /* CAN message is a request, set ID accordingly */
    canMessage->id = id;
    /* First byte contains the CMD field */
    canMessage->data[0] = cmd;
}

static bool I165C_CheckResponse(uint8_t command, CAN_BUFFERELEMENT_s *canMessage) {
    FAS_ASSERT(canMessage != NULL_PTR);
    bool messageReceived   = false;
    uint8_t numberItems    = 0u;
    uint8_t queueReadTries = I165C_MAX_QUEUE_READS;

    /* Use loop on queue because IMD_info message could come meanwhile */
    do {
        numberItems = uxQueueMessagesWaiting(ftsk_imdCanDataQueue);
        if (numberItems > 0u) {
            if (OS_ReceiveFromQueue(ftsk_imdCanDataQueue, (void *)canMessage, 0u) == OS_SUCCESS) {
                /* data queue was no empty */
                if (canMessage->data[0] == command) {
                    messageReceived = true;
                    break;
                }
            }
        }
        queueReadTries--;
    } while ((numberItems > 0u) && (queueReadTries > 0u));

    return messageReceived;
}

static bool I165C_GetImdInfo(CAN_BUFFERELEMENT_s *canMessage) {
    FAS_ASSERT(canMessage != NULL_PTR);
    bool imdInfoReceived   = false;
    uint8_t numberItems    = 0u;
    uint8_t queueReadTries = I165C_MAX_QUEUE_READS;

    /* Use loop on queue because other messages could come meanwhile */
    do {
        numberItems = uxQueueMessagesWaiting(ftsk_imdCanDataQueue);
        if (numberItems > 0u) {
            if (OS_ReceiveFromQueue(ftsk_imdCanDataQueue, (void *)canMessage, 0u) == OS_SUCCESS) {
                /* data queue was no empty */
                if (canMessage->id == I165C_MESSAGETYPE_IMD_INFO) {
                    imdInfoReceived = true;
                    break;
                }
            }
        }
        queueReadTries--;
    } while ((numberItems > 0u) && (queueReadTries > 0u));

    return imdInfoReceived;
}

static bool I165C_IsInitialized(CAN_BUFFERELEMENT_s canMessage) {
    bool initialized = true;
    uint16_t data    = 0u;

    I165C_ReadDataWordImdInfo(I165C_DW2, &data, canMessage);
    /* I165C_SELFTEST_RUNNING bit = 1 in IMD_Info DW2: selft test running */
    if (0u != (data & (1u << I165C_SELFTEST_RUNNING))) {
        initialized = false;
    }

    I165C_ReadDataWordImdInfo(I165C_DW3, &data, canMessage);
    /**
     * I165C_INSULATION_MEASUREMENT bit = 1 in IMD_Info DW3:
     * insulation measurement deactivated
     */
    if (0u != (data & (1u << I165C_INSULATION_MEASUREMENT))) {
        initialized = false;
    }
    /**
     * I165C_IMC_SELFTEST_OVERALL_SCENARIO bit = 1 in IMD_Info DW3:
     * selftest overall scenario not executed
     */
    if (0u != (data & (1u << I165C_IMC_SELFTEST_OVERALL_SCENARIO))) {
        /* selftest overall scenario not executed */
        initialized = false;
    }

    return initialized;
}

static void I165C_CheckAcknowledgeArrived(
    uint8_t command,
    I165C_STATE_e *currentState,
    I165C_STATE_e nextState,
    uint8_t *tries,
    CAN_BUFFERELEMENT_s *canMessage) {
    FAS_ASSERT(currentState != NULL_PTR);
    FAS_ASSERT(tries != NULL_PTR);
    FAS_ASSERT(canMessage != NULL_PTR);

    if (false == I165C_CheckResponse(command, canMessage)) {
        (*tries)++;
    } else {
        *tries        = 0u;
        *currentState = nextState;
    }
    if ((*tries) >= I165C_TRANSMISSION_TRIES) {
        /* Transmission not acknowledged: restart initialization procedure */
        *currentState = I165C_STATE_SELFTEST;
    }
}

static void I165C_Trigger(void) {
    static I165C_STATE_e i165cState            = I165C_STATE_UNINITIALIZED;
    static uint8_t i165c_receptionTries        = 0u;
    static uint8_t i165c_receptionTriesImdInfo = 0u;
    static uint16_t dataImdInfo                = 0u;
    static uint16_t data0                      = 0u;
    static uint8_t data1                       = 0u;
    static uint8_t data2                       = 0u;

    switch (i165cState) {
        case I165C_STATE_UNINITIALIZED:
            i165cState = I165C_STATE_SELFTEST;
            break;
        case I165C_STATE_SELFTEST:
            I165C_ResetCanData(&i165c_canTxMessage);
            I165C_WriteCmd(I165C_MESSAGETYPE_IMD_REQUEST, I165C_CMD_S_IMC_CTL_SELFTEST, &i165c_canTxMessage);
            I165C_WriteDataWord(
                I165C_D_IMC_SELFTEST_SCR_CTL_REQUEST, I165C_SELFTEST_SCENARIO_OVERALL, &i165c_canTxMessage);
            CAN_DataSend(I165C_CAN_NODE, i165c_canTxMessage.id, i165c_canTxMessage.data);
            i165c_receptionTries = 0u;
            i165cState           = I165C_STATE_SELFTEST_WAIT_ACK;
            break;
        case I165C_STATE_SELFTEST_WAIT_ACK:
            I165C_CheckAcknowledgeArrived(
                I165C_CMD_S_IMC_CTL_SELFTEST,
                &i165cState,
                I165C_STATE_WAIT_SELFTEST,
                &i165c_receptionTries,
                &i165c_canRxMessage);
            break;
        case I165C_STATE_WAIT_SELFTEST:
            if (true == I165C_GetImdInfo(&i165c_canRxMessage)) {
                if (false == I165C_IsInitialized(i165c_canRxMessage)) {
                    i165c_receptionTries++;
                } else {
                    i165c_receptionTries        = 0u;
                    i165c_receptionTriesImdInfo = 0u;
                    i165cState                  = I165C_STATE_INITIALIZATION_FINISHED;
                    break;
                }
            } else {
                i165c_receptionTries++;
            }
            if (i165c_receptionTries >= I165C_INITIALIZATION_TRIES) {
                /* Initialization not working: restart initialization procedure */
                i165cState = I165C_STATE_SELFTEST;
                break;
            }
            break;
        case I165C_STATE_INITIALIZATION_FINISHED:
            i165cState = I165C_STATE_READ_RESISTANCE;
            break;
        case I165C_STATE_READ_RESISTANCE:
            I165C_ResetCanData(&i165c_canTxMessage);
            I165C_WriteCmd(I165C_MESSAGETYPE_IMD_REQUEST, I165C_CMD_S_IMC_GET_R_ISO, &i165c_canTxMessage);
            CAN_DataSend(I165C_CAN_NODE, i165c_canTxMessage.id, i165c_canTxMessage.data);
            i165c_receptionTries = 0u;
            i165cState           = I165C_STATE_READ_RESISTANCE_WAIT_ACK;
            break;
        case I165C_STATE_READ_RESISTANCE_WAIT_ACK:
            I165C_CheckAcknowledgeArrived(
                I165C_CMD_S_IMC_GET_R_ISO, &i165cState, I165C_STATE_UNLOCK, &i165c_receptionTries, &i165c_canRxMessage);
            if (i165cState == I165C_STATE_UNLOCK) {
                /* Response arrived */
                I165C_ReadDataWord(I165C_DW1, &data0, i165c_canRxMessage);
                I165C_ReadDataByte(I165C_DB1, &data1, i165c_canRxMessage);
                I165C_ReadDataByte(I165C_DB2, &data2, i165c_canRxMessage);
            }
            break;
        case I165C_STATE_UNLOCK:
            I165C_ResetCanData(&i165c_canTxMessage);
            I165C_WriteCmd(I165C_MESSAGETYPE_IMD_REQUEST, I165C_CMD_S_VIFC_CTL_LOCK, &i165c_canTxMessage);
            I165C_WriteDataWord(I165C_D_VIFC_LOCK_MODE_CTL_REQUEST, I165C_LOCKMODE_UNLOCKED, &i165c_canTxMessage);
            I165C_WriteDataWord(I165C_D_VIFC_LOCK_PWD_CTL_REQUEST, I165C_UNLOCK_PASSWD, &i165c_canTxMessage);
            CAN_DataSend(I165C_CAN_NODE, i165c_canTxMessage.id, i165c_canTxMessage.data);
            i165c_receptionTries = 0u;
            i165cState           = I165C_STATE_UNLOCK_WAIT_ACK;
            break;
        case I165C_STATE_UNLOCK_WAIT_ACK:
            I165C_CheckAcknowledgeArrived(
                I165C_CMD_S_VIFC_CTL_LOCK,
                &i165cState,
                I165C_STATE_SET_ERROR_THRESHOLD,
                &i165c_receptionTries,
                &i165c_canRxMessage);
            break;
        case I165C_STATE_SET_ERROR_THRESHOLD:
            I165C_ResetCanData(&i165c_canTxMessage);
            I165C_WriteCmd(I165C_MESSAGETYPE_IMD_REQUEST, I165C_CMD_S_IMC_SET_R_ISO_ERR_THR, &i165c_canTxMessage);
            I165C_WriteDataWord(I165C_D_IMC_R_ISO_ERR_THR_SET_REQUEST, I165C_ERROR_THRESHOLD_KOHM, &i165c_canTxMessage);
            CAN_DataSend(I165C_CAN_NODE, i165c_canTxMessage.id, i165c_canTxMessage.data);
            i165c_receptionTries = 0u;
            i165cState           = I165C_STATE_SET_ERROR_THRESHOLD_WAIT_ACK;
            break;
        case I165C_STATE_SET_ERROR_THRESHOLD_WAIT_ACK:
            I165C_CheckAcknowledgeArrived(
                I165C_CMD_S_IMC_SET_R_ISO_ERR_THR,
                &i165cState,
                I165C_STATE_SET_WARNING_THRESHOLD,
                &i165c_receptionTries,
                &i165c_canRxMessage);
            break;
        case I165C_STATE_SET_WARNING_THRESHOLD:
            I165C_ResetCanData(&i165c_canTxMessage);
            I165C_WriteCmd(I165C_MESSAGETYPE_IMD_REQUEST, I165C_CMD_S_IMC_SET_R_ISO_ERR_WRN, &i165c_canTxMessage);
            I165C_WriteDataWord(
                I165C_D_IMC_R_ISO_ERR_WRN_SET_REQUEST, I165C_WARNING_THRESHOLD_KOHM, &i165c_canTxMessage);
            CAN_DataSend(I165C_CAN_NODE, i165c_canTxMessage.id, i165c_canTxMessage.data);
            i165c_receptionTries = 0u;
            i165cState           = I165C_STATE_SET_WARNING_THRESHOLD_WAIT_ACK;
            break;
        case I165C_STATE_SET_WARNING_THRESHOLD_WAIT_ACK:
            I165C_CheckAcknowledgeArrived(
                I165C_CMD_S_IMC_SET_R_ISO_ERR_WRN,
                &i165cState,
                I165C_STATE_GET_MEASUREMENT,
                &i165c_receptionTries,
                &i165c_canRxMessage);
            break;
        case I165C_STATE_GET_MEASUREMENT:
            if (false == I165C_GetImdInfo(&i165c_canRxMessage)) {
                i165c_receptionTriesImdInfo++;
            } else {
                i165c_receptionTriesImdInfo = 0u;
                /* Get measured resistance */
                I165C_ReadDataWordImdInfo(I165C_DW1, &dataImdInfo, i165c_canRxMessage);
                i165c_insulationMeasurementData.insulationResistance_kOhm = dataImdInfo;
                /* Get IMD status */
                I165C_ReadDataWordImdInfo(I165C_DW2, &dataImdInfo, i165c_canRxMessage);
                if (0u != (dataImdInfo & (1u << I165C_INSULATION_FAULT))) {
                    /* Insulation fault */
                    i165c_insulationMeasurementData.insulationFault = 1u;
                } else {
                    i165c_insulationMeasurementData.insulationFault = 0u;
                }
                if (0u != (dataImdInfo & (1u << I165C_CHASSIS_FAULT))) {
                    /* Chassis fault */
                    i165c_insulationMeasurementData.chassisFault = 1u;
                } else {
                    i165c_insulationMeasurementData.chassisFault = 0u;
                }
                if (0u != (dataImdInfo & (1u << I165C_SYSTEM_FAILURE))) {
                    /* System failure */
                    i165c_insulationMeasurementData.systemFailure = 1u;
                } else {
                    i165c_insulationMeasurementData.systemFailure = 0u;
                }
                if (0u != (dataImdInfo & (1u << I165C_INSULATION_WARNING))) {
                    /* Insulation warning */
                    i165c_insulationMeasurementData.insulationWarning = 1u;
                } else {
                    i165c_insulationMeasurementData.insulationWarning = 0u;
                }
                /* Get VIFC status */
                I165C_ReadDataWordImdInfo(I165C_DW3, &dataImdInfo, i165c_canRxMessage);
                if (0u != (dataImdInfo & (1u << I165C_INSULATION_MEASUREMENT))) {
                    /* Insulation measurement deactivated*/
                }
                if (0u != (dataImdInfo & (1u << I165C_IMC_ALIVE_STATUS_DETECTION))) {
                    /* VIFC status failure */
                }
                if (0u != (dataImdInfo & (1u << I165C_INSULATION_RESISTANCE_VALUE))) {
                    /* Insulation resistance value outdated */
                }
                if (0u != (dataImdInfo & (1u << I165C_IMC_SELFTEST_OVERALL_SCENARIO))) {
                    /* selftest overall scenario not executed */
                }
                if (0u != (dataImdInfo & (1u << I165C_IMC_SELFTEST_PARAMETERCONFIG_SCENARIO))) {
                    /* selftest parameterconfig scenario not executed */
                }
                DATA_WRITE_DATA(&i165c_insulationMeasurementData);
            }
            if (i165c_receptionTriesImdInfo >= I165C_IMD_INFO_RECEIVE_TRIES) {
                /* IMD_Info not comming: restart initialization procedure */
                i165cState = I165C_STATE_SELFTEST;
                break;
            }
            break;
        default:
            /* invalid state */
            FAS_ASSERT(FAS_TRAP);
            break;
    }
}

/*========== Extern Function Implementations ================================*/

extern void IMD_Trigger(void) {
    I165C_Trigger();
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_I165C_ResetCanData(CAN_BUFFERELEMENT_s *canMessage) {
    I165C_ResetCanData(canMessage);
}
extern void TEST_I165C_WriteDataWord(uint8_t dataWord, uint16_t data, CAN_BUFFERELEMENT_s *canMessage) {
    I165C_WriteDataWord(dataWord, data, canMessage);
}
extern void TEST_I165C_ReadDataWord(uint8_t dataWord, uint16_t *data, CAN_BUFFERELEMENT_s canMessage) {
    I165C_ReadDataWord(dataWord, data, canMessage);
}
extern void TEST_I165C_ReadDataWordImdInfo(uint8_t dataWord, uint16_t *data, CAN_BUFFERELEMENT_s canMessage) {
    I165C_ReadDataWordImdInfo(dataWord, data, canMessage);
}
extern void TEST_I165C_ReadDataByte(uint8_t dataByte, uint8_t *data, CAN_BUFFERELEMENT_s canMessage) {
    I165C_ReadDataByte(dataByte, data, canMessage);
}
extern void TEST_I165C_WriteCmd(uint8_t id, uint8_t cmd, CAN_BUFFERELEMENT_s *canMessage) {
    I165C_WriteCmd(id, cmd, canMessage);
}
extern bool TEST_I165C_CheckResponse(uint8_t command, CAN_BUFFERELEMENT_s *canMessage) {
    return I165C_CheckResponse(command, canMessage);
}
extern bool TEST_I165C_GetImdInfo(CAN_BUFFERELEMENT_s *canMessage) {
    return I165C_GetImdInfo(canMessage);
}
extern bool TEST_I165C_IsInitialized(CAN_BUFFERELEMENT_s canMessage) {
    return I165C_IsInitialized(canMessage);
}
extern void TEST_I165C_CheckAcknowledgeArrived(
    uint8_t command,
    I165C_STATE_e *currentState,
    I165C_STATE_e nextState,
    uint8_t *tries,
    CAN_BUFFERELEMENT_s *canMessage) {
    I165C_CheckAcknowledgeArrived(command, currentState, nextState, tries, canMessage);
}

#endif
