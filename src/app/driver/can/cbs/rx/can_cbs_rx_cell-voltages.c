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
 * @file    can_cbs_rx_cell-voltages.c
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
 * prototype for the callback 'CANRX_CellVoltages' */
#include "can_cbs_rx.h"
#include "can_cfg_rx-message-definitions.h"
#include "can_helper.h"
#include "diag.h"
#include "ftask.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** CAN message parameters for can cell voltage  */
#define CANRX_CAN_CELL_VOLTAGE_MUX_BIT_START           (7u)
#define CANRX_CAN_CELL_VOLTAGE_MUX_LENGTH              (8u)
#define CANRX_CAN_CELL_VOLTAGE0_INVALID_FLAG_BIT_START (12u)
#define CANRX_CAN_CELL_VOLTAGE1_INVALID_FLAG_BIT_START (13u)
#define CANRX_CAN_CELL_VOLTAGE2_INVALID_FLAG_BIT_START (14u)
#define CANRX_CAN_CELL_VOLTAGE3_INVALID_FLAG_BIT_START (15u)
#define CANRX_CAN_CELL_VOLTAGE_INVALID_FLAG_LENGTH     (1u)
#define CANRX_CAN_CELL_VOLTAGE0_BIT_START              (11u)
#define CANRX_CAN_CELL_VOLTAGE1_BIT_START              (30u)
#define CANRX_CAN_CELL_VOLTAGE2_BIT_START              (33u)
#define CANRX_CAN_CELL_VOLTAGE3_BIT_START              (52u)
#define CANRX_CAN_CELL_VOLTAGE_LENGTH                  (13u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief get the cell voltage signal from can message data
 * @param pCellVoltages TODO: describe what the pointer actually is
 * @param messageData received message data
 */
static void CANRX_GetCanAfeCellVoltagesFromMessage(
    CAN_CAN2AFE_CELL_VOLTAGES_QUEUE_s *pCellVoltages,
    uint64_t messageData);

/*========== Static Function Implementations ================================*/
static void CANRX_GetCanAfeCellVoltagesFromMessage(
    CAN_CAN2AFE_CELL_VOLTAGES_QUEUE_s *pCellVoltages,
    uint64_t messageData) {
    /* CAN signal parameters for can cell voltages */
    static const uint8_t canrx_kCanCellVoltageMuxBitStart = CANRX_CAN_CELL_VOLTAGE_MUX_BIT_START;
    static const uint8_t canrx_kCanCellVoltageMuxLength   = CANRX_CAN_CELL_VOLTAGE_MUX_LENGTH;
    static const uint8_t canrx_kCanCellVoltageInvalidFlagBitStart[CAN_NUM_OF_VOLTAGES_IN_CAN_CELL_VOLTAGES_MSG] = {
        CANRX_CAN_CELL_VOLTAGE0_INVALID_FLAG_BIT_START,
        CANRX_CAN_CELL_VOLTAGE1_INVALID_FLAG_BIT_START,
        CANRX_CAN_CELL_VOLTAGE2_INVALID_FLAG_BIT_START,
        CANRX_CAN_CELL_VOLTAGE3_INVALID_FLAG_BIT_START};
    static const uint8_t canrx_kCanCellVoltageInvalidFlagLength = CANRX_CAN_CELL_VOLTAGE_INVALID_FLAG_LENGTH;
    static const uint8_t canrx_kCanCellVoltageBitStart[CAN_NUM_OF_VOLTAGES_IN_CAN_CELL_VOLTAGES_MSG] = {
        CANRX_CAN_CELL_VOLTAGE0_BIT_START,
        CANRX_CAN_CELL_VOLTAGE1_BIT_START,
        CANRX_CAN_CELL_VOLTAGE2_BIT_START,
        CANRX_CAN_CELL_VOLTAGE3_BIT_START};
    static const uint8_t canrx_kCanCellVoltageLength = CANRX_CAN_CELL_VOLTAGE_LENGTH;

    /* Declare and initialize the extracted signal from message */
    uint64_t pCanSignalMuxValue    = 0u;
    uint64_t pCanSignalInvalidFlag = 0u;
    uint64_t pCanSignalVoltage     = 0u;

    /* Get the mux value */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        canrx_kCanCellVoltageMuxBitStart,
        canrx_kCanCellVoltageMuxLength,
        &pCanSignalMuxValue,
        CANRX_AFE_CELL_VOLTAGES_ENDIANNESS);
    pCellVoltages->muxValue = (uint8_t)pCanSignalMuxValue;

    /* Get the invalid flag */
    for (uint8_t i = 0; i < CAN_NUM_OF_VOLTAGES_IN_CAN_CELL_VOLTAGES_MSG; i++) {
        pCanSignalInvalidFlag = 0u;
        CAN_RxGetSignalDataFromMessageData(
            messageData,
            canrx_kCanCellVoltageInvalidFlagBitStart[i],
            canrx_kCanCellVoltageInvalidFlagLength,
            &pCanSignalInvalidFlag,
            CANRX_AFE_CELL_VOLTAGES_ENDIANNESS);
        pCellVoltages->invalidFlag[i] = ((uint8_t)pCanSignalInvalidFlag > 0u);
    }

    /* Get the voltages */
    for (uint8_t i = 0; i < CAN_NUM_OF_VOLTAGES_IN_CAN_CELL_VOLTAGES_MSG; i++) {
        pCanSignalVoltage = 0u;
        CAN_RxGetSignalDataFromMessageData(
            messageData,
            canrx_kCanCellVoltageBitStart[i],
            canrx_kCanCellVoltageLength,
            &pCanSignalVoltage,
            CANRX_AFE_CELL_VOLTAGES_ENDIANNESS);
        pCellVoltages->cellVoltage[i] = (uint16_t)pCanSignalVoltage;
    }
}

/*========== Extern Function Implementations ================================*/
extern uint32_t CANRX_CellVoltages(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim) {
    /* Check the information of this can message  */
    FAS_ASSERT(message.id == CANRX_AFE_CELL_VOLTAGES_ID);
    FAS_ASSERT(message.idType == CANRX_AFE_CELL_VOLTAGES_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(kpkCanData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* Get the message data from can data and pack it into CAN_CAN2AFE_CELL_VOLTAGES_QUEUE_s */
    uint64_t messageData                                    = 0u;
    CAN_CAN2AFE_CELL_VOLTAGES_QUEUE_s canrx_canCellVoltages = {0};
    CAN_RxGetMessageDataFromCanData(&messageData, kpkCanData, message.endianness);
    CANRX_GetCanAfeCellVoltagesFromMessage(&canrx_canCellVoltages, messageData);

    /* Write the whole temperature message to the queue*/
    if (OS_SendToBackOfQueue(ftsk_canToAfeCellVoltagesQueue, (void *)&canrx_canCellVoltages, 0u) == OS_SUCCESS) {
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
extern void TEST_CANRX_GetCanAfeCellVoltagesFromMessage(
    CAN_CAN2AFE_CELL_VOLTAGES_QUEUE_s *pCellVoltages,
    uint64_t messageData) {
    CANRX_GetCanAfeCellVoltagesFromMessage(pCellVoltages, messageData);
}
#endif
