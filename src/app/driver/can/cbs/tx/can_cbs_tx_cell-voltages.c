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
 * @file    can_cbs_tx_cell-voltages.c
 * @author  foxBMS Team
 * @date    2021-04-20 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVER
 * @prefix  CANTX
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for cell voltages
 */

/*========== Includes =======================================================*/
#include "can_cbs_tx.h"
#include "can_cfg_tx-message-definitions.h"
#include "can_helper.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
/** the number of voltages per message-frame */
#define CANTX_NUMBER_OF_MUX_VOLTAGES_PER_MESSAGE (4u)

/**
 * CAN signals used in this message
 * Parameters:
 * bit start, bit length, factor, offset, minimum value, maximum value
 */
static const CAN_SIGNAL_TYPE_s cantx_cellVoltageMultiplexer  = {7u, 8u, 1.0f, 0.0f, 0.0f, 1.0f};
static const CAN_SIGNAL_TYPE_s cantx_cellVoltage0InvalidFlag = {12u, 1u, 1.0f, 0.0f, 0.0f, 1.0f};
static const CAN_SIGNAL_TYPE_s cantx_cellVoltage1InvalidFlag = {13u, 1u, 1.0f, 0.0f, 0.0f, 1.0f};
static const CAN_SIGNAL_TYPE_s cantx_cellVoltage2InvalidFlag = {14u, 1u, 1.0f, 0.0f, 0.0f, 1.0f};
static const CAN_SIGNAL_TYPE_s cantx_cellVoltage3InvalidFlag = {15u, 1u, 1.0f, 0.0f, 0.0f, 1.0f};
static const CAN_SIGNAL_TYPE_s cantx_cellVoltage0_mV         = {11u, 13u, 1.0f, 0.0f, 0.0f, 8192.0f};
static const CAN_SIGNAL_TYPE_s cantx_cellVoltage1_mV         = {30u, 13u, 1.0f, 0.0f, 0.0f, 8192.0f};
static const CAN_SIGNAL_TYPE_s cantx_cellVoltage2_mV         = {33u, 13u, 1.0f, 0.0f, 0.0f, 8192.0f};
static const CAN_SIGNAL_TYPE_s cantx_cellVoltage3_mV         = {52u, 13u, 1.0f, 0.0f, 0.0f, 8192.0f};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief Helper function for CANTX_CellVoltages()
 *
 * Used in the CANTX_CellVoltages() callback to set
 * invalid flag data and voltage data
 * in the CAN frame.
 *
 * @param[in] muxId                          multiplexer value
 * @param[in] pMessage                       pointer to CAN frame data
 * @param[in] cellVoltageSignal              signal characteristics for voltage data
 * @param[in] cellVoltageInvalidFlagSignal   signal characteristics for invalid flag data
 * @param[in] endianness                     big or little endianness of data
 * @param[in] kpkCanShim                     shim to the database entries
 */
static void CANTX_VoltageSetData(
    uint8_t muxId,
    uint64_t *pMessage,
    CAN_SIGNAL_TYPE_s cellVoltageSignal,
    CAN_SIGNAL_TYPE_s cellVoltageInvalidFlagSignal,
    CAN_ENDIANNESS_e endianness,
    const CAN_SHIM_s *const kpkCanShim);

/*========== Static Function Implementations ================================*/

static void CANTX_VoltageSetData(
    uint8_t muxId,
    uint64_t *pMessage,
    CAN_SIGNAL_TYPE_s cellVoltageSignal,
    CAN_SIGNAL_TYPE_s cellVoltageInvalidFlagSignal,
    CAN_ENDIANNESS_e endianness,
    const CAN_SHIM_s *const kpkCanShim) {
    /* cell index must not be greater than the number of cells */
    if (muxId < (BS_NR_OF_CELL_BLOCKS_PER_STRING * BS_NR_OF_STRINGS)) {
        /* Get string, module and cell number */
        const uint8_t stringNumber = DATA_GetStringNumberFromVoltageIndex(muxId);
        const uint8_t moduleNumber = DATA_GetModuleNumberFromVoltageIndex(muxId);
        const uint8_t cellNumber   = DATA_GetCellNumberFromVoltageIndex(muxId);

        uint32_t signalData_valid = 0u;
        /* Valid bits data */
        if ((kpkCanShim->pTableCellVoltage->invalidCellVoltage[stringNumber][moduleNumber] & (0x01u << cellNumber)) ==
            0u) {
            signalData_valid = 0u;
        } else {
            signalData_valid = 1u;
        }
        /* Set valid bit data in CAN frame */
        CAN_TxSetMessageDataWithSignalData(
            pMessage,
            cellVoltageInvalidFlagSignal.bitStart,
            cellVoltageInvalidFlagSignal.bitLength,
            signalData_valid,
            endianness);

        /*Voltage data */
        float_t signalData_mV =
            (float_t)(kpkCanShim->pTableCellVoltage
                          ->cellVoltage_mV[stringNumber]
                                          [(moduleNumber * BS_NR_OF_CELL_BLOCKS_PER_MODULE) + cellNumber]);
        /* Apply offset and factor */
        CAN_TxPrepareSignalData(&signalData_mV, cellVoltageSignal);
        /* Set voltage data in CAN frame */
        CAN_TxSetMessageDataWithSignalData(
            pMessage, cellVoltageSignal.bitStart, cellVoltageSignal.bitLength, (uint32_t)signalData_mV, endianness);
    }
}

/*========== Extern Function Implementations ================================*/
extern uint32_t CANTX_CellVoltages(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(message.id == CANTX_CELL_VOLTAGES_ID);
    FAS_ASSERT(message.idType == CANTX_CELL_VOLTAGES_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(pMuxId != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t messageData = 0u;

    /* Reset mux if maximum was reached */
    if (*pMuxId >= (BS_NR_OF_STRINGS * BS_NR_OF_CELL_BLOCKS_PER_STRING)) {
        *pMuxId = 0u;
    }
    /* First signal to transmit cell voltages: get database values */
    if (*pMuxId == 0u) {
        DATA_READ_DATA(kpkCanShim->pTableCellVoltage);
    }

    /* Set mux signal in CAN frame */
    uint32_t signalData = *pMuxId / CANTX_NUMBER_OF_MUX_VOLTAGES_PER_MESSAGE;
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        cantx_cellVoltageMultiplexer.bitStart,
        cantx_cellVoltageMultiplexer.bitLength,
        (uint32_t)signalData,
        message.endianness);

    /* Set other signals in CAN frame */
    CANTX_VoltageSetData(
        *pMuxId, &messageData, cantx_cellVoltage0_mV, cantx_cellVoltage0InvalidFlag, message.endianness, kpkCanShim);
    /* Increment multiplexer for next cell */
    (*pMuxId)++;
    CANTX_VoltageSetData(
        *pMuxId, &messageData, cantx_cellVoltage1_mV, cantx_cellVoltage1InvalidFlag, message.endianness, kpkCanShim);
    /* Increment multiplexer for next cell */
    (*pMuxId)++;
    CANTX_VoltageSetData(
        *pMuxId, &messageData, cantx_cellVoltage2_mV, cantx_cellVoltage2InvalidFlag, message.endianness, kpkCanShim);
    /* Increment multiplexer for next cell */
    (*pMuxId)++;
    CANTX_VoltageSetData(
        *pMuxId, &messageData, cantx_cellVoltage3_mV, cantx_cellVoltage3InvalidFlag, message.endianness, kpkCanShim);
    /* Increment multiplexer for next cell */
    (*pMuxId)++;

    /* All signal data copied in CAN frame, now copy data in the buffer that will be use to send the frame */
    CAN_TxSetCanDataWithMessageData(messageData, pCanData, message.endianness);

    return 0;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
