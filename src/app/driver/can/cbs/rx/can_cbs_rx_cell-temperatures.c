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
 * @file    can_cbs_rx_cell-temperatures.c
 * @author  foxBMS Team
 * @date    2024-04-08 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup DRIVERS
 * @prefix  CANRX
 *
 * @brief   CAN driver Rx callback implementation
 * @details CAN Rx callback for the debug cell-temperatures
 */

/*========== Includes =======================================================*/
/* AXIVION Next Codeline Generic-LocalInclude: 'can_cbs_rx.h' declares the
 * prototype for the callback 'CANRX_CellTemperatures' */
#include "can_cbs_rx.h"
#include "can_cfg_rx-message-definitions.h"
#include "can_helper.h"
#include "diag.h"
#include "ftask.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** CAN message parameters for can cell temperature  */
#define CANRX_CAN_CELL_TEMPERATURE_MUX_BIT_START           (7u)
#define CANRX_CAN_CELL_TEMPERATURE_MUX_LENGTH              (8u)
#define CANRX_CAN_CELL_TEMPERATURE0_INVALID_FLAG_BIT_START (8u)
#define CANRX_CAN_CELL_TEMPERATURE1_INVALID_FLAG_BIT_START (9u)
#define CANRX_CAN_CELL_TEMPERATURE2_INVALID_FLAG_BIT_START (10u)
#define CANRX_CAN_CELL_TEMPERATURE3_INVALID_FLAG_BIT_START (11u)
#define CANRX_CAN_CELL_TEMPERATURE4_INVALID_FLAG_BIT_START (12u)
#define CANRX_CAN_CELL_TEMPERATURE5_INVALID_FLAG_BIT_START (13u)
#define CANRX_CAN_CELL_TEMPERATURE_INVALID_FLAG_LENGTH     (1u)
#define CANRX_CAN_CELL_TEMPERATURE0_BIT_START              (23u)
#define CANRX_CAN_CELL_TEMPERATURE1_BIT_START              (31u)
#define CANRX_CAN_CELL_TEMPERATURE2_BIT_START              (39u)
#define CANRX_CAN_CELL_TEMPERATURE3_BIT_START              (47u)
#define CANRX_CAN_CELL_TEMPERATURE4_BIT_START              (55u)
#define CANRX_CAN_CELL_TEMPERATURE5_BIT_START              (63u)
#define CANRX_CAN_CELL_TEMPERATURE_LENGTH                  (8u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief get the cell temperature signal from can message data
 * @param pCellTemperatures TODO: describe what the pointer actually is
 * @param messageData received message data
 */
static void CANRX_GetCanAfeCellTemperaturesFromMessage(
    CAN_CAN2AFE_CELL_TEMPERATURES_QUEUE_s *pCellTemperatures,
    uint64_t messageData);

/*========== Static Function Implementations ================================*/
static void CANRX_GetCanAfeCellTemperaturesFromMessage(
    CAN_CAN2AFE_CELL_TEMPERATURES_QUEUE_s *pCellTemperatures,
    uint64_t messageData) {
    /* CAN signal parameters for can cell temperatures */
    static const uint8_t canrx_kCanCellTemperatureMuxBitStart = CANRX_CAN_CELL_TEMPERATURE_MUX_BIT_START;
    static const uint8_t canrx_kCanCellTemperatureMuxLength   = CANRX_CAN_CELL_TEMPERATURE_MUX_LENGTH;
    static const uint8_t
        canrx_kCanCellTemperatureInvalidFlagBitStart[CAN_NUM_OF_TEMPERATURES_IN_CAN_CELL_TEMPERATURES_MSG] = {
            CANRX_CAN_CELL_TEMPERATURE0_INVALID_FLAG_BIT_START,
            CANRX_CAN_CELL_TEMPERATURE1_INVALID_FLAG_BIT_START,
            CANRX_CAN_CELL_TEMPERATURE2_INVALID_FLAG_BIT_START,
            CANRX_CAN_CELL_TEMPERATURE3_INVALID_FLAG_BIT_START,
            CANRX_CAN_CELL_TEMPERATURE4_INVALID_FLAG_BIT_START,
            CANRX_CAN_CELL_TEMPERATURE5_INVALID_FLAG_BIT_START};
    static const uint8_t canrx_kCanCellTemperatureInvalidFlagLength = CANRX_CAN_CELL_TEMPERATURE_INVALID_FLAG_LENGTH;
    static const uint8_t canrx_kCanCellTemperatureBitStart[CAN_NUM_OF_TEMPERATURES_IN_CAN_CELL_TEMPERATURES_MSG] = {
        CANRX_CAN_CELL_TEMPERATURE0_BIT_START,
        CANRX_CAN_CELL_TEMPERATURE1_BIT_START,
        CANRX_CAN_CELL_TEMPERATURE2_BIT_START,
        CANRX_CAN_CELL_TEMPERATURE3_BIT_START,
        CANRX_CAN_CELL_TEMPERATURE4_BIT_START,
        CANRX_CAN_CELL_TEMPERATURE5_BIT_START};
    static const uint8_t canrx_kCanCellTemperatureLength = CANRX_CAN_CELL_TEMPERATURE_LENGTH;

    /* Declare and initialize the extracted signal from message */
    uint64_t pCanSignalMuxValue    = 0u;
    uint64_t pCanSignalInvalidFlag = 0u;
    uint64_t pCanSignalTemperature = 0u;

    /* Get the pCellTemperatures.muxValue*/
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        canrx_kCanCellTemperatureMuxBitStart,
        canrx_kCanCellTemperatureMuxLength,
        &pCanSignalMuxValue,
        CANRX_AFE_CELL_TEMPERATURES_ENDIANNESS);
    pCellTemperatures->muxValue = (uint8_t)pCanSignalMuxValue;

    /* Get the invalid flag */
    for (uint8_t i = 0; i < CAN_NUM_OF_TEMPERATURES_IN_CAN_CELL_TEMPERATURES_MSG; i++) {
        pCanSignalInvalidFlag = 0u;
        CAN_RxGetSignalDataFromMessageData(
            messageData,
            canrx_kCanCellTemperatureInvalidFlagBitStart[i],
            canrx_kCanCellTemperatureInvalidFlagLength,
            &pCanSignalInvalidFlag,
            CANRX_AFE_CELL_TEMPERATURES_ENDIANNESS);
        pCellTemperatures->invalidFlag[i] = ((uint8_t)pCanSignalInvalidFlag > 0u);
    }

    /* Get the voltages */
    for (uint8_t i = 0; i < CAN_NUM_OF_TEMPERATURES_IN_CAN_CELL_TEMPERATURES_MSG; i++) {
        pCanSignalTemperature = 0u;
        CAN_RxGetSignalDataFromMessageData(
            messageData,
            canrx_kCanCellTemperatureBitStart[i],
            canrx_kCanCellTemperatureLength,
            &pCanSignalTemperature,
            CANRX_AFE_CELL_TEMPERATURES_ENDIANNESS);
        pCellTemperatures->cellTemperature[i] = (int16_t)pCanSignalTemperature;
    }
}

/*========== Extern Function Implementations ================================*/
extern uint32_t CANRX_CellTemperatures(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim) {
    /* Check the information of this can message  */
    FAS_ASSERT(message.id == CANRX_AFE_CELL_TEMPERATURES_ID);
    FAS_ASSERT(message.idType == CANRX_AFE_CELL_TEMPERATURES_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(kpkCanData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* Get the message data from can data and pack it into CAN_CAN2AFE_CELL_TEMPERATURES_QUEUE_s */
    uint64_t messageData                                            = 0u;
    CAN_CAN2AFE_CELL_TEMPERATURES_QUEUE_s canrx_canCellTemperatures = {0};
    CAN_RxGetMessageDataFromCanData(&messageData, kpkCanData, message.endianness);
    CANRX_GetCanAfeCellTemperaturesFromMessage(&canrx_canCellTemperatures, messageData);

    /* Write the whole temperature message to the queue */
    if (OS_SendToBackOfQueue(ftsk_canToAfeCellTemperaturesQueue, (void *)&canrx_canCellTemperatures, 0u) ==
        OS_SUCCESS) {
        /* queue is not full */
        (void)DIAG_Handler(DIAG_ID_CAN_RX_QUEUE_FULL, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
    } else {
        /* queue is full */
        (void)DIAG_Handler(DIAG_ID_CAN_RX_QUEUE_FULL, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
    }

    return 0u;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_CANRX_GetCanAfeCellTemperaturesFromMessage(
    CAN_CAN2AFE_CELL_TEMPERATURES_QUEUE_s *pCellTemperatures,
    uint64_t messageData) {
    CANRX_GetCanAfeCellTemperaturesFromMessage(pCellTemperatures, messageData);
}
#endif
