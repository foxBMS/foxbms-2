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
 * @file    can_cbs_rx_imd-response.c
 * @author  foxBMS Team
 * @date    2021-04-20 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  CANRX
 *
 * @brief   CAN driver Rx callback implementation
 * @details CAN Rx callback for IMD messages
 */

/*========== Includes =======================================================*/
#include "bender_iso165c_cfg.h"

/* AXIVION Next Codeline Generic-LocalInclude: 'can_cbs_rx.h' declares the
 * prototype for the callback 'CANRX_ImdResponse' */
#include "can_cbs_rx.h"
#include "can_cfg_rx-message-definitions.h"
#include "can_helper.h"
#include "foxmath.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
#define CANRX_IMD_RESPONSE_INSULATION_RESISTANCE_MEASUREMENT_START_BIT (8u)
#define CANRX_IMD_RESPONSE_INSULATION_RESISTANCE_MEASUREMENT_LENGTH    (16u)
#define CANRX_IMD_RESPONSE_INSULATION_FAULT_BIAS_START_BIT             (24u)
#define CANRX_IMD_RESPONSE_INSULATION_FAULT_BIAS_LENGTH                (8u)
#define CANRX_IMD_RESPONSE_HV_RELAIS_START_BIT                         (8u)
#define CANRX_IMD_RESPONSE_HV_RELAIS_LENGTH                            (16u)
#define CANRX_IMD_RESPONSE_HV_RELAIS_STATE_START_BIT                   (24u)
#define CANRX_IMD_RESPONSE_HV_RELAIS_STATE_LENGTH                      (16u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief Reads the information from the can data and transfers it to a can buffer element
 * @param messageDlc Dlc size of the message
 * @param kpkCanData Pointer to the data of the received message
 * @param canBuffer Pointer to the can buffer element where information is stored
 */
static void CANRX_TransferImdResponseMessageToCanBuffer(
    uint8_t messageDlc,
    const uint8_t *const kpkCanData,
    CAN_BUFFER_ELEMENT_s *canBuffer);

/*========== Static Function Implementations ================================*/
static void CANRX_TransferImdResponseMessageToCanBuffer(
    uint8_t messageDlc,
    const uint8_t *const kpkCanData,
    CAN_BUFFER_ELEMENT_s *canBuffer) {
    FAS_ASSERT(0u < messageDlc);
    FAS_ASSERT(messageDlc <= CAN_MAX_DLC);
    FAS_ASSERT(kpkCanData != NULL_PTR);
    FAS_ASSERT(canBuffer != NULL_PTR);

    /* determine dlc size of the message */
    const uint8_t boundedDlc = MATH_MinimumOfTwoUint8_t(messageDlc, CAN_MAX_DLC);

    /* transfer can data to buffer element*/
    for (uint8_t i = 0; i < boundedDlc; i++) {
        canBuffer->data[i] = kpkCanData[i];
    }
}

/*========== Extern Function Implementations ================================*/
extern uint32_t CANRX_ImdResponse(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim) {
    /* This handler is only implemented for little endian */
    FAS_ASSERT(message.endianness == CAN_LITTLE_ENDIAN);
    FAS_ASSERT(message.id == CANRX_IMD_RESPONSE_ID);
    FAS_ASSERT(message.idType == CANRX_IMD_RESPONSE_ID_TYPE);
    FAS_ASSERT(message.dlc <= CAN_MAX_DLC); /* Currently max 8 bytes in a CAN frame */
    FAS_ASSERT(kpkCanData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    /* set up can buffer element */
    CAN_BUFFER_ELEMENT_s canBuffer = {
        .canNode = I165C_CAN_NODE, .id = message.id, .idType = message.idType, .data = {0u}};
    uint32_t retVal = 1u;

    /* copy message data to buffer element */
    CANRX_TransferImdResponseMessageToCanBuffer(message.dlc, kpkCanData, &canBuffer);

    /*send buffer element to the back of queue*/
    if (OS_SendToBackOfQueue(*(kpkCanShim->pQueueImd), (void *)&canBuffer, 0u) == OS_SUCCESS) {
        retVal = 0u;
    }
    return retVal;
}

extern void CANRX_ImdResponseReadInsulationResistance(
    const uint8_t *const kpkCanData,
    DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring) {
    FAS_ASSERT(kpkCanData != NULL_PTR);
    FAS_ASSERT(pTableInsulationMonitoring != NULL_PTR);

    uint64_t messageData = 0u;
    uint64_t signalData  = 0u;

    /* Get measured insulation resistance and write it to the datatable */
    CAN_RxGetMessageDataFromCanData(&messageData, kpkCanData, CAN_LITTLE_ENDIAN);
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_IMD_RESPONSE_INSULATION_RESISTANCE_MEASUREMENT_START_BIT,
        CANRX_IMD_RESPONSE_INSULATION_RESISTANCE_MEASUREMENT_LENGTH,
        &signalData,
        CAN_LITTLE_ENDIAN);

    pTableInsulationMonitoring->insulationResistance_kOhm = (uint32_t)signalData;
}

extern void CANRX_ImdResponseCheckInsulationFaultTendency(
    const uint8_t *const kpkCanData,
    DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring) {
    FAS_ASSERT(kpkCanData != NULL_PTR);
    FAS_ASSERT(pTableInsulationMonitoring != NULL_PTR);

    uint64_t messageData = 0u;
    uint64_t signalData  = 0u;

    /* Get insulation fault bias */
    CAN_RxGetMessageDataFromCanData(&messageData, kpkCanData, CAN_LITTLE_ENDIAN);
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_IMD_RESPONSE_INSULATION_FAULT_BIAS_START_BIT,
        CANRX_IMD_RESPONSE_INSULATION_FAULT_BIAS_LENGTH,
        &signalData,
        CAN_LITTLE_ENDIAN);

    if (signalData == I165C_BIAS_TO_HV_PLUS) {
        pTableInsulationMonitoring->dfIsChassisShortToHvPlus  = true;
        pTableInsulationMonitoring->dfIsChassisShortToHvMinus = false;
    } else if (signalData == I165C_BIAS_TO_HV_MINUS) {
        pTableInsulationMonitoring->dfIsChassisShortToHvMinus = true;
        pTableInsulationMonitoring->dfIsChassisShortToHvPlus  = false;
    } else {
        pTableInsulationMonitoring->dfIsChassisShortToHvPlus  = false;
        pTableInsulationMonitoring->dfIsChassisShortToHvMinus = false;
    }
}

extern bool CANRX_ImdResponseCheckRelayState(const uint8_t *const kpkCanData, uint8_t relay, uint8_t relayState) {
    FAS_ASSERT(kpkCanData != NULL_PTR);
    FAS_ASSERT((relay == I165C_D_VIFC_HV_RELAIS_NEGATIVE) || (relay == I165C_D_VIFC_HV_RELAIS_POSITIVE));
    FAS_ASSERT((relayState == I165C_RELAY_STATE_OPEN) || (relayState == I165C_RELAY_STATE_CLOSED));

    uint64_t messageData = 0u;
    uint64_t signalData  = 0u;
    bool checkSuccess    = true;

    /* Get relay state from response message */
    CAN_RxGetMessageDataFromCanData(&messageData, kpkCanData, CAN_LITTLE_ENDIAN);

    /* Check if relay matches */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_IMD_RESPONSE_HV_RELAIS_START_BIT,
        CANRX_IMD_RESPONSE_HV_RELAIS_LENGTH,
        &signalData,
        CAN_LITTLE_ENDIAN);
    if (relay != signalData) {
        checkSuccess = false;
    }

    /* Check if relay state matches */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_IMD_RESPONSE_HV_RELAIS_STATE_START_BIT,
        CANRX_IMD_RESPONSE_HV_RELAIS_STATE_LENGTH,
        &signalData,
        CAN_LITTLE_ENDIAN);
    if (relayState != signalData) {
        checkSuccess = false;
    }

    return checkSuccess;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_CANRX_TransferImdResponseMessageToCanBuffer(
    uint8_t messageDlc,
    const uint8_t *const kpkCanData,
    CAN_BUFFER_ELEMENT_s *canBuffer) {
    CANRX_TransferImdResponseMessageToCanBuffer(messageDlc, kpkCanData, canBuffer);
}
#endif
