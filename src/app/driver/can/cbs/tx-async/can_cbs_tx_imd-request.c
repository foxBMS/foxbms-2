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
 * @file    can_cbs_tx_imd-request.c
 * @author  foxBMS Team
 * @date    2023-06-14 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for pack value and string value messages
 */

/*========== Includes =======================================================*/
#include "can_cbs_tx_imd-request.h"

#include "bender_iso165c_cfg.h"

#include "can.h"
#include "can_cfg_tx-async-message-definitions.h"
#include "can_helper.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
#define CANTX_IMD_REQUEST_MUX_START_BIT         (0u)
#define CANTX_IMD_REQUEST_MUX_LENGTH            (8u)
#define CANTX_IMD_REQUEST_DATA_WORD_1_START_BIT (8u)
#define CANTX_IMD_REQUEST_DATA_WORD_1_LENGTH    (16u)
#define CANTX_IMD_REQUEST_DATA_WORD_2_START_BIT (24u)
#define CANTX_IMD_REQUEST_DATA_WORD_2_LENGTH    (16u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Request that relay opens
 * @param   relay which relay opens
 * @return  #STD_OK if transmission successful, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e CANTX_RequestRelayOpen(uint8_t relay);

/**
 * @brief   Request that relay close
 * @param   relay which relay closes
 * @return  #STD_OK if transmission successful, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e CANTX_RequestRelayClose(uint8_t relay);

/**
 * @brief   Request the current state of the relay
 * @param   relay which relay's state is requested
 * @return  #STD_OK if transmission successful, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e CANTX_RequestRelayState(uint8_t relay);

/**
 * @brief   Request to enable measurement
 * @return  #STD_OK if transmission successful, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e CANTX_RequestEnableMeasurement(void);

/**
 * @brief   Request to disable measurement
 * @return  #STD_OK if transmission successful, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e CANTX_RequestDisableMeasurement(void);

/**
 * @brief   Sets averaging factor
 * @return  #STD_OK if transmission successful, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e CANTX_SetAveragingFactor(void);

/**
 * @brief   Requests reading the resistance
 * @return  #STD_OK if transmission successful, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e CANTX_RequestReadResistance(void);

/**
 * @brief   Request unlock
 * @return  #STD_OK if transmission successful, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e CANTX_RequestUnlock(void);

/**
 * @brief   Request self test
 * @return  #STD_OK if transmission successful, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e CANTX_RequestSelfTest(void);

/**
 * @brief   Set Error Threshold
 * @return  #STD_OK if transmission successful, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e CANTX_SetErrorThreshold(void);

/**
 * @brief   Set warning threshold
 * @return  #STD_OK if transmission successful, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e CANTX_SetWarningThreshold(void);

/*========== Static Function Implementations ================================*/
static STD_RETURN_TYPE_e CANTX_RequestRelayOpen(uint8_t relay) {
    FAS_ASSERT((relay == I165C_D_VIFC_HV_RELAIS_POSITIVE) || (relay == I165C_D_VIFC_HV_RELAIS_NEGATIVE));
    uint8_t data[] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    uint64_t message = 0u;
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_IMD_REQUEST_MUX_START_BIT,
        CANTX_IMD_REQUEST_MUX_LENGTH,
        I165C_CMD_S_VIFC_SET_HV_RELAIS,
        CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_IMD_REQUEST_DATA_WORD_1_START_BIT,
        CANTX_IMD_REQUEST_DATA_WORD_1_LENGTH,
        relay,
        CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_IMD_REQUEST_DATA_WORD_2_START_BIT,
        CANTX_IMD_REQUEST_DATA_WORD_2_LENGTH,
        I165C_RELAY_STATE_OPEN,
        CAN_LITTLE_ENDIAN);

    CAN_TxSetCanDataWithMessageData(message, &data[0], CAN_LITTLE_ENDIAN);
    return CAN_DataSend(I165C_CAN_NODE, I165C_MESSAGE_TYPE_IMD_REQUEST, I165C_TX_MESSAGE_IDENTIFIER_TYPE, &data[0]);
}

static STD_RETURN_TYPE_e CANTX_RequestRelayClose(uint8_t relay) {
    FAS_ASSERT((relay == I165C_D_VIFC_HV_RELAIS_POSITIVE) || (relay == I165C_D_VIFC_HV_RELAIS_NEGATIVE));
    uint8_t data[] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    uint64_t message = 0u;
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_IMD_REQUEST_MUX_START_BIT,
        CANTX_IMD_REQUEST_MUX_LENGTH,
        I165C_CMD_S_VIFC_SET_HV_RELAIS,
        CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_IMD_REQUEST_DATA_WORD_1_START_BIT,
        CANTX_IMD_REQUEST_DATA_WORD_1_LENGTH,
        relay,
        CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_IMD_REQUEST_DATA_WORD_2_START_BIT,
        CANTX_IMD_REQUEST_DATA_WORD_2_LENGTH,
        I165C_RELAY_STATE_CLOSED,
        CAN_LITTLE_ENDIAN);

    CAN_TxSetCanDataWithMessageData(message, &data[0], CAN_LITTLE_ENDIAN);
    return CAN_DataSend(I165C_CAN_NODE, I165C_MESSAGE_TYPE_IMD_REQUEST, I165C_TX_MESSAGE_IDENTIFIER_TYPE, &data[0]);
}

static STD_RETURN_TYPE_e CANTX_RequestRelayState(uint8_t relay) {
    FAS_ASSERT((relay == I165C_D_VIFC_HV_RELAIS_POSITIVE) || (relay == I165C_D_VIFC_HV_RELAIS_NEGATIVE));
    uint8_t data[] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    uint64_t message = 0u;
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_IMD_REQUEST_MUX_START_BIT,
        CANTX_IMD_REQUEST_MUX_LENGTH,
        I165C_CMD_S_VIFC_GET_HV_RELAIS,
        CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_IMD_REQUEST_DATA_WORD_1_START_BIT,
        CANTX_IMD_REQUEST_DATA_WORD_1_LENGTH,
        relay,
        CAN_LITTLE_ENDIAN);

    CAN_TxSetCanDataWithMessageData(message, &data[0], CAN_LITTLE_ENDIAN);
    return CAN_DataSend(I165C_CAN_NODE, I165C_MESSAGE_TYPE_IMD_REQUEST, I165C_TX_MESSAGE_IDENTIFIER_TYPE, &data[0]);
}

static STD_RETURN_TYPE_e CANTX_RequestEnableMeasurement(void) {
    uint8_t data[] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    uint64_t message = 0u;
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_IMD_REQUEST_MUX_START_BIT,
        CANTX_IMD_REQUEST_MUX_LENGTH,
        I165C_CMD_S_VIFC_CTL_MEASUREMENT,
        CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_IMD_REQUEST_DATA_WORD_1_START_BIT,
        CANTX_IMD_REQUEST_DATA_WORD_1_LENGTH,
        I165C_ENABLE_MEASUREMENT,
        CAN_LITTLE_ENDIAN);

    CAN_TxSetCanDataWithMessageData(message, &data[0], CAN_LITTLE_ENDIAN);
    return CAN_DataSend(I165C_CAN_NODE, I165C_MESSAGE_TYPE_IMD_REQUEST, I165C_TX_MESSAGE_IDENTIFIER_TYPE, &data[0]);
}

static STD_RETURN_TYPE_e CANTX_RequestDisableMeasurement(void) {
    uint8_t data[] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    uint64_t message = 0u;
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_IMD_REQUEST_MUX_START_BIT,
        CANTX_IMD_REQUEST_MUX_LENGTH,
        I165C_CMD_S_VIFC_CTL_MEASUREMENT,
        CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_IMD_REQUEST_DATA_WORD_1_START_BIT,
        CANTX_IMD_REQUEST_DATA_WORD_1_LENGTH,
        I165C_DISABLE_MEASUREMENT,
        CAN_LITTLE_ENDIAN);

    CAN_TxSetCanDataWithMessageData(message, &data[0], CAN_LITTLE_ENDIAN);
    return CAN_DataSend(I165C_CAN_NODE, I165C_MESSAGE_TYPE_IMD_REQUEST, I165C_TX_MESSAGE_IDENTIFIER_TYPE, &data[0]);
}

static STD_RETURN_TYPE_e CANTX_SetAveragingFactor(void) {
    uint8_t data[] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    uint64_t message = 0u;
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_IMD_REQUEST_MUX_START_BIT,
        CANTX_IMD_REQUEST_MUX_LENGTH,
        I165C_CMD_S_IMC_SET_MEAN_FACTOR,
        CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_IMD_REQUEST_DATA_WORD_1_START_BIT,
        CANTX_IMD_REQUEST_DATA_WORD_1_LENGTH,
        I165C_MEASUREMENT_AVERAGING_FACTOR,
        CAN_LITTLE_ENDIAN);

    CAN_TxSetCanDataWithMessageData(message, &data[0], CAN_LITTLE_ENDIAN);
    return CAN_DataSend(I165C_CAN_NODE, I165C_MESSAGE_TYPE_IMD_REQUEST, I165C_TX_MESSAGE_IDENTIFIER_TYPE, &data[0]);
}

static STD_RETURN_TYPE_e CANTX_RequestReadResistance(void) {
    uint8_t data[] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    uint64_t message = 0u;
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_IMD_REQUEST_MUX_START_BIT,
        CANTX_IMD_REQUEST_MUX_LENGTH,
        I165C_CMD_S_IMC_GET_R_ISO,
        CAN_LITTLE_ENDIAN);

    CAN_TxSetCanDataWithMessageData(message, &data[0], CAN_LITTLE_ENDIAN);
    return CAN_DataSend(I165C_CAN_NODE, I165C_MESSAGE_TYPE_IMD_REQUEST, I165C_TX_MESSAGE_IDENTIFIER_TYPE, &data[0]);
}

static STD_RETURN_TYPE_e CANTX_RequestUnlock(void) {
    uint8_t data[] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    uint64_t message = 0u;
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_IMD_REQUEST_MUX_START_BIT,
        CANTX_IMD_REQUEST_MUX_LENGTH,
        I165C_CMD_S_VIFC_CTL_LOCK,
        CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_IMD_REQUEST_DATA_WORD_1_START_BIT,
        CANTX_IMD_REQUEST_DATA_WORD_1_LENGTH,
        I165C_LOCK_MODE_UNLOCKED,
        CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_IMD_REQUEST_DATA_WORD_2_START_BIT,
        CANTX_IMD_REQUEST_DATA_WORD_2_LENGTH,
        I165C_UNLOCK_PASSWORD,
        CAN_LITTLE_ENDIAN);

    CAN_TxSetCanDataWithMessageData(message, &data[0], CAN_LITTLE_ENDIAN);
    return CAN_DataSend(I165C_CAN_NODE, I165C_MESSAGE_TYPE_IMD_REQUEST, I165C_TX_MESSAGE_IDENTIFIER_TYPE, &data[0]);
}

static STD_RETURN_TYPE_e CANTX_RequestSelfTest(void) {
    uint8_t data[] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    uint64_t message = 0u;
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_IMD_REQUEST_MUX_START_BIT,
        CANTX_IMD_REQUEST_MUX_LENGTH,
        I165C_CMD_S_IMC_CTL_SELFTEST,
        CAN_LITTLE_ENDIAN);

#ifdef I165C_SELF_TEST_LONG
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_IMD_REQUEST_DATA_WORD_1_START_BIT,
        CANTX_IMD_REQUEST_DATA_WORD_1_LENGTH,
        I165C_SELFTEST_SCENARIO_OVERALL,
        CAN_LITTLE_ENDIAN);
#else /* I165C_SELF_TEST_SHORT */
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_IMD_REQUEST_DATA_WORD_1_START_BIT,
        CANTX_IMD_REQUEST_DATA_WORD_1_LENGTH,
        I165C_SELFTEST_SCENARIO_PARAMETERCONFIG,
        CAN_LITTLE_ENDIAN);
#endif

    CAN_TxSetCanDataWithMessageData(message, &data[0], CAN_LITTLE_ENDIAN);
    return CAN_DataSend(I165C_CAN_NODE, I165C_MESSAGE_TYPE_IMD_REQUEST, I165C_TX_MESSAGE_IDENTIFIER_TYPE, &data[0]);
}

static STD_RETURN_TYPE_e CANTX_SetErrorThreshold(void) {
    uint8_t data[] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    uint64_t message = 0u;
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_IMD_REQUEST_MUX_START_BIT,
        CANTX_IMD_REQUEST_MUX_LENGTH,
        I165C_CMD_S_IMC_SET_R_ISO_ERR_THR,
        CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_IMD_REQUEST_DATA_WORD_1_START_BIT,
        CANTX_IMD_REQUEST_DATA_WORD_1_LENGTH,
        I165C_ERROR_THRESHOLD_kOhm,
        CAN_LITTLE_ENDIAN);

    CAN_TxSetCanDataWithMessageData(message, &data[0], CAN_LITTLE_ENDIAN);
    return CAN_DataSend(I165C_CAN_NODE, I165C_MESSAGE_TYPE_IMD_REQUEST, I165C_TX_MESSAGE_IDENTIFIER_TYPE, &data[0]);
}

static STD_RETURN_TYPE_e CANTX_SetWarningThreshold(void) {
    uint8_t data[] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    uint64_t message = 0u;
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_IMD_REQUEST_MUX_START_BIT,
        CANTX_IMD_REQUEST_MUX_LENGTH,
        I165C_CMD_S_IMC_SET_R_ISO_WRN_THR,
        CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_IMD_REQUEST_DATA_WORD_1_START_BIT,
        CANTX_IMD_REQUEST_DATA_WORD_1_LENGTH,
        I165C_WARNING_THRESHOLD_kOhm,
        CAN_LITTLE_ENDIAN);

    CAN_TxSetCanDataWithMessageData(message, &data[0], CAN_LITTLE_ENDIAN);
    return CAN_DataSend(I165C_CAN_NODE, I165C_MESSAGE_TYPE_IMD_REQUEST, I165C_TX_MESSAGE_IDENTIFIER_TYPE, &data[0]);
}

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e CANTX_ImdRequest(CANTX_IMD_REQUEST_ACTIONS_e action) {
    FAS_ASSERT(action < CANTX_IMD_REQUEST_LAST_ACTION);
    STD_RETURN_TYPE_e successfullyQueued = STD_NOT_OK;
    switch (action) {
        case CANTX_IMD_REQUEST_OPEN_POSITIVE_RELAY:
            successfullyQueued = CANTX_RequestRelayOpen(I165C_D_VIFC_HV_RELAIS_POSITIVE);
            break;
        case CANTX_IMD_REQUEST_OPEN_NEGATIVE_RELAY:
            successfullyQueued = CANTX_RequestRelayOpen(I165C_D_VIFC_HV_RELAIS_NEGATIVE);
            break;
        case CANTX_IMD_REQUEST_CLOSE_POSITIVE_RELAY:
            successfullyQueued = CANTX_RequestRelayClose(I165C_D_VIFC_HV_RELAIS_POSITIVE);
            break;
        case CANTX_IMD_REQUEST_CLOSE_NEGATIVE_RELAY:
            successfullyQueued = CANTX_RequestRelayClose(I165C_D_VIFC_HV_RELAIS_NEGATIVE);
            break;
        case CANTX_IMD_REQUEST_POSITIVE_RELAY_STATE:
            successfullyQueued = CANTX_RequestRelayState(I165C_D_VIFC_HV_RELAIS_POSITIVE);
            break;
        case CANTX_IMD_REQUEST_NEGATIVE_RELAY_STATE:
            successfullyQueued = CANTX_RequestRelayState(I165C_D_VIFC_HV_RELAIS_NEGATIVE);
            break;
        case CANTX_IMD_REQUEST_ENABLE_MEASUREMENT:
            successfullyQueued = CANTX_RequestEnableMeasurement();
            break;
        case CANTX_IMD_REQUEST_DISABLE_MEASUREMENT:
            successfullyQueued = CANTX_RequestDisableMeasurement();
            break;
        case CANTX_IMD_REQUEST_SET_AVERAGING_FACTOR:
            successfullyQueued = CANTX_SetAveragingFactor();
            break;
        case CANTX_IMD_REQUEST_READ_RESISTANCE:
            successfullyQueued = CANTX_RequestReadResistance();
            break;
        case CANTX_IMD_REQUEST_INITIALIZATION_UNLOCK:
            successfullyQueued = CANTX_RequestUnlock();
            break;
        case CANTX_IMD_REQUEST_INITIALIZATION_SELF_TEST:
            successfullyQueued = CANTX_RequestSelfTest();
            break;
        case CANTX_IMD_REQUEST_INITIALIZATION_SET_ERROR_THRESHOLD:
            successfullyQueued = CANTX_SetErrorThreshold();
            break;
        case CANTX_IMD_REQUEST_INITIALIZATION_SET_WARNING_THRESHOLD:
            successfullyQueued = CANTX_SetWarningThreshold();
            break;
        default:
            FAS_ASSERT(FAS_TRAP);
            break; /* LCOV_EXCL_LINE */
    }
    return successfullyQueued;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST

/* export RTC helper functions */
extern STD_RETURN_TYPE_e TEST_CANTX_RequestRelayOpen(uint8_t relay) {
    return CANTX_RequestRelayOpen(relay);
}

extern STD_RETURN_TYPE_e TEST_CANTX_RequestRelayClose(uint8_t relay) {
    return CANTX_RequestRelayClose(relay);
}

extern STD_RETURN_TYPE_e TEST_CANTX_RequestRelayState(uint8_t relay) {
    return CANTX_RequestRelayState(relay);
}

#endif
